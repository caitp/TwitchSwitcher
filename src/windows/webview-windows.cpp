// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#ifdef TSW_WEBVIEW_WIN32

#include <twitchsw/webview.h>

#include "windows/webview-windows.h"

#include <algorithm>
#include <locale>

// ATL / MFC helpers
//   CComPtr<T>
#include <atlcomcli.h>

// DWebBrowserEvents2 members
#include <ExDispid.h>

#include <MsHtmdid.h>
#include <jsrt.h>
#include <jscript9diag.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifndef arraysize
#define arraysize(x) (sizeof(x) / sizeof(*x))
#endif

enum ThreadMessages {
    // Use WM_USER + 1000, as WM_USER + 1 seems to be sent to this module by another project
    // (obs-studio, perhaps?)
    kCloseWebViewUI = WM_USER + 1000,
    kCreateWebViewUI,
    kSetWebViewTitle,
};

namespace twitchsw {

struct MessageData {
    WebViewImpl* self;
    void** result;
};

const char* WebView::kOBS_UI_TYPE = "win32";

// Helper to convert a string to UTF16 from what is assumed to be UTF8. If the resulting
// size is greater than the output buffer size, the return value should be freed with
// ::free().
static char16_t* utf8ToUtf16(const std::string& string, char16_t* out, size_t inlineSize) {
    char16_t* buffer = out;
    size_t bufferSize = inlineSize;
    if (buffer == nullptr || inlineSize == 0) {
        bufferSize = inlineSize = 64;
        buffer = new char16_t[inlineSize];
    }
    do {
        int result = MultiByteToWideChar(
            CP_UTF8, 0, string.c_str(), static_cast<int>(string.length()),
            reinterpret_cast<LPWSTR>(buffer), static_cast<int>(bufferSize));

        if (result != 0) {
            if (static_cast<size_t>(result) >= bufferSize) {
                char16_t* newBuffer = new char16_t[bufferSize + 1];
                ::memmove(newBuffer, buffer, bufferSize * sizeof(char16_t));
                if (buffer != out)
                    delete[] buffer;
                buffer = newBuffer;
            }
            buffer[result] = 0;
            return buffer;
        }
        bufferSize = std::max(inlineSize, static_cast<size_t>(256)) * 2;
        char16_t* newBuffer = new char16_t[bufferSize];
        if (buffer != out)
            delete[] buffer;
        buffer = newBuffer;
    } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if (buffer != nullptr && buffer != out)
        delete[] buffer;
    return nullptr;
}

static std::string utf16ToUtf8(BSTR bstr) {
    if (bstr == nullptr) return std::string();
    int len = (int)SysStringLen(bstr);
    char inlineBuffer[256] = "";
    char* buffer = inlineBuffer;
    int bufferSize = arraysize(inlineBuffer);

    do {
        int result = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, bstr, len, buffer, bufferSize - 1, nullptr, nullptr);

        if (result != 0) {
            std::string result(buffer);
            if (buffer != inlineBuffer)
                delete[] buffer;
            return result;
        }

        if (buffer != inlineBuffer)
            delete[] buffer;
        bufferSize = bufferSize * 2;
        buffer = new char[bufferSize];
        if (!buffer)
            break;
    } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if (buffer != nullptr && buffer != inlineBuffer)
        delete[] buffer;
    return std::string();
}

static std::string formatError(HRESULT error) {
    char inlineBuffer[64] = "";
    char* buffer = inlineBuffer;
    int bufferSize = arraysize(inlineBuffer);
    DWORD result;
    do {
        result = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, bufferSize, nullptr);
        if (result != 0) {
            std::string result(buffer);
            if (buffer != inlineBuffer)
                delete[] buffer;
            return result;
        }

        bufferSize = bufferSize * 2;
        if (buffer != inlineBuffer)
            delete[] buffer;
        buffer = new char[bufferSize];
    } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

    if (buffer != inlineBuffer)
        delete[] buffer;
    return std::string();
}

//
// WebView frame
//

DWORD WebViewImpl::g_threadId = 0;
HANDLE WebViewImpl::g_thread = nullptr;
void WebViewImpl::initialize() {
    if (g_threadId != 0)
        return;
    g_thread = CreateThread(NULL, 0, &WebViewImpl::messagePumpThreadProc, nullptr, 0, &g_threadId);
}

void WebViewImpl::shutdown() {
    if (g_threadId == 0)
        return;
    PostThreadMessage(g_threadId, WM_QUIT, 0, 0);
    if (WaitForSingleObject(g_thread, 1000) == WAIT_TIMEOUT)
        TerminateThread(g_thread, 1);
    g_threadId = 0;
    g_thread = nullptr;
}

WebViewImpl::WebViewImpl()
    : m_hwnd(nullptr)
    , m_hinstance(nullptr)
    , m_content(nullptr)
    , m_showScrollBars(false)
{
    InitializeCriticalSection(&m_criticalSection);
    InitializeConditionVariable(&m_conditionVariable);
}

WebViewImpl::~WebViewImpl()
{
    close();
}

void WebViewImpl::closeInternal() {
    // ASSERT: GetCurrentThreadId() == g_threadId
    if (m_content != nullptr) {
        m_content->close();
        m_content->Release();
        m_content = nullptr;
    }
    if (m_hwnd != nullptr) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    m_hinstance = nullptr;
}

void WebViewImpl::close() {
    if (GetCurrentThreadId() == g_threadId)
        return closeInternal();
    EnterCriticalSection(&m_criticalSection);
    PostThreadMessage(g_threadId, kCloseWebViewUI, 0, (LPARAM)this);
    SleepConditionVariableCS(&m_conditionVariable, &m_criticalSection, INFINITE);
    LeaveCriticalSection(&m_criticalSection);
}

void WebViewImpl::open(const std::string& url, const HttpRequestOptions& options) {
    if (!ensureUI() || !m_content)
        return;
    m_url = url;
    if (options.onRedirect())
        m_onRedirect = options.onRedirect();
    return m_content->open(url, options);
}

void WebViewImpl::show() {
    BOOL ret = ShowWindow(m_hwnd, SW_SHOW);
    if (!ret)
        LOG(LOG_WARNING, "ShowWindow() failed.");
    ret = SetForegroundWindow(m_hwnd);
    if (!ret)
        LOG(LOG_WARNING, "SetForegroundWindow() failed.");

}

void WebViewImpl::setTitle(const std::string& title) {
    m_title = title;
    if (m_hwnd) {
        auto titleUtf16 = utf8ToUtf16(title, nullptr, 0);
        SendNotifyMessageA(m_hwnd, kSetWebViewTitle, 0, (LPARAM)titleUtf16);
    }
}

//
// WebView internal API
//
WebContent::WebContent(WebViewImpl* impl) {
    m_refs = 0;
    m_impl = impl;
    m_hwnd = nullptr;
    m_browser = nullptr;
    m_cookie = 0;
    m_didFinishRequest = false;
    m_isNavigating = false;
}

WebContent::~WebContent() {
}

HRESULT WebContent::setupOLE() {
    HRESULT hresult;
    if (m_browser != nullptr || m_cookie != 0)
        return E_FAIL;

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    IOleObject* oleObj = nullptr;

    hresult = CoCreateInstance(CLSID_WebBrowser, NULL, CLSCTX_INPROC_SERVER, IID_IOleObject, (void**)&oleObj);
    if (oleObj == nullptr)
        return hresult;

    do {
        if (FAILED(hresult = oleObj->SetClientSite((IOleClientSite*)this)))
            break;

        if (FAILED(hresult = oleObj->SetHostNames(L"MyHost", L"MyDoc")))
            break;

        if (FAILED(hresult = OleSetContainedObject(oleObj, TRUE)))
            break;

        if (FAILED(hresult = oleObj->DoVerb(OLEIVERB_SHOW, NULL, this, 0, m_hwnd, &rc)))
            break;

        bool connected = false;
        IConnectionPointContainer* container = nullptr;
        oleObj->QueryInterface(IID_IConnectionPointContainer, (void**)&container);
        if (container != nullptr) {
            IConnectionPoint* connectionPoint = nullptr;
            container->FindConnectionPoint(DIID_DWebBrowserEvents2, &connectionPoint);

            if (connectionPoint != nullptr) {
                connectionPoint->Advise(reinterpret_cast<IUnknown*>(this), &m_cookie);
                connectionPoint->Release();
                connected = true;
            }
            container->Release();
        }

        if (!connected)
            break;

        if (FAILED(hresult = oleObj->QueryInterface(IID_IWebBrowser2, (void**)&m_browser)))
            break;

        m_browser->put_Silent(VARIANT_TRUE);
        oleObj->Release();
        return S_OK;
    } while (0);

    if (m_browser != nullptr)
        m_browser->Release();

    if (oleObj != nullptr)
        oleObj->Release();

    m_browser = nullptr;
    return hresult;
}

LRESULT CALLBACK WebViewImpl::WebWindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Parent window of the WebView's WndProc
    if (msg == WM_NCCREATE) {
        WebViewImpl* self = static_cast<WebViewImpl*>(LPCREATESTRUCT(lParam)->lpCreateParams);
        self->m_hwnd = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self);
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    WebViewImpl* self = reinterpret_cast<WebViewImpl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (self == nullptr)
        return DefWindowProc(hwnd, msg, wParam, lParam);
    return self->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT WebViewImpl::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        m_content->m_impl = this;
        m_content->m_hwnd = m_content->create(m_hwnd, m_hinstance, 0, m_showScrollBars);
        break;

    case WM_SIZE: {
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);
        if (m_content->m_hwnd)
            MoveWindow(m_content->m_hwnd, 0, 0, width, height, TRUE);
        break;
    }

    case WM_DESTROY:
        // `this` may/should be invalid at this point.
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)nullptr);

        auto onAbort = m_onAbort;

        // Prevent retaining WebView
        Ref<WebView> webView = *m_webView.get();
        m_webView = nullptr;
        if (onAbort)
            onAbort(webView.get(), m_url);
        WakeConditionVariable(&m_conditionVariable);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WebContent::WebContentWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        WebContent* self = reinterpret_cast<WebContent*>(LPCREATESTRUCT(lParam)->lpCreateParams);
        self->m_hwnd = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)self);


        HRESULT hresult = self->setupOLE();
        if (FAILED(hresult)) {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Failed to instantiate WebView: '%s' (%8X) --- Please file a bug at https://github.com/caitp/TwitchSwitcher",
                formatError(hresult).c_str(), hresult);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)nullptr);
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    WebContent* self = reinterpret_cast<WebContent*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (self == nullptr)
        return DefWindowProc(hwnd, msg, wParam, lParam);
    return self->WndProc(hwnd, msg, wParam, lParam);
}

LRESULT WebContent::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        // Supress scrollbars
        if (createStruct->style & (WS_HSCROLL | WS_VSCROLL))
            SetWindowLongPtr(m_hwnd, GWL_STYLE, createStruct->style & ~(WS_HSCROLL | WS_VSCROLL));
        break;
    }

    case WM_DESTROY: {
        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)nullptr);
        close();
        return 0;
    }

    case WM_SIZE: {
        if (m_browser != nullptr) {
            m_browser->put_Width(LOWORD(lParam));
            m_browser->put_Height(HIWORD(lParam));
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT paintStruct;
        BeginPaint(m_hwnd, &paintStruct);
        {
            HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(paintStruct.hdc, &paintStruct.rcPaint, brush);
            DeleteObject(brush);
        }
        EndPaint(m_hwnd, &paintStruct);
        return 0;
    }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND WebViewImpl::create(HINSTANCE hInstance, UINT x, UINT y, UINT width, UINT height, bool showScrollbars) {
    WNDCLASSEX wcex;
    static const char* className = "TSW.WebViewWindow";

    m_hinstance = hInstance;
    m_showScrollBars = showScrollbars;

    if (!GetClassInfoEx(hInstance, className, &wcex)) {
        ZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(wcex);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = (WNDPROC)WebViewImpl::WebWindowWndProc;
        wcex.cbWndExtra = sizeof(this);
        wcex.hInstance = hInstance;
        wcex.hIcon = NULL;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName = NULL;
        wcex.lpszClassName = className;
        wcex.hIconSm = NULL;
        if (!RegisterClassEx(&wcex)) {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Could not register WebView UI for WebViewWindow: '%s'. Please file a bug at https://github.com/caitp/TwitchSwitcher",
                formatError(GetLastError()).c_str());
            return nullptr;
        }
    }

    if (m_content != nullptr) {
        m_content->close();
        m_content = nullptr;
    }
    m_content = new WebContent(this);
    m_content->AddRef();

    return CreateWindowEx(0, className, "", WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION, x, y, width, height, NULL, NULL, hInstance, (LPVOID)this);
}

HWND WebViewImpl::createInternal() {
    UINT screenWidth = GetSystemMetrics(SM_CXMAXIMIZED);
    UINT screenHeight = GetSystemMetrics(SM_CYMAXIMIZED);
    UINT width = (screenWidth / 3) * 2;
    UINT height = (screenHeight / 3) * 2;
    UINT x = (screenWidth - width) / 2;
    UINT y = (screenHeight - height) / 2;
    HWND hwnd = create(GetModuleHandle(NULL), x, y, width, height, true);
    if (!m_title.empty()) {
        char16_t* title = utf8ToUtf16(m_title, nullptr, 0);
        DefWindowProcW(hwnd, WM_SETTEXT, 0, (LPARAM)title);
        delete[] title;
    }
    return hwnd;
}

bool WebViewImpl::ensureUI() {
    if (m_hwnd == nullptr) {
        HWND hwnd = nullptr;
        auto currentThreadId = GetCurrentThreadId();
        if (currentThreadId == g_threadId)
            hwnd = createInternal();
        else {
            EnterCriticalSection(&m_criticalSection);
            MessageData data = { this, (void**)&hwnd };
            PostThreadMessage(g_threadId, kCreateWebViewUI, 0, (LPARAM)&data);
            SleepConditionVariableCS(&m_conditionVariable, &m_criticalSection, INFINITE);
            LeaveCriticalSection(&m_criticalSection);
        }
        if (!hwnd || !m_content)
            return false;
    }
    return true;
}

HWND WebContent::create(HWND parent, HINSTANCE hInstance, UINT id, bool showScrollBars) {
    WNDCLASSEX wcex = { 0 };
    if (!GetClassInfoEx(hInstance, TSW_WEBVIEW_WNDCLASS, &wcex)) {
        ZeroMemory(&wcex, sizeof(wcex));
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = (WNDPROC)WebContent::WebContentWndProc;
        wcex.lpszClassName = TSW_WEBVIEW_WNDCLASS;
        wcex.cbWndExtra = sizeof(this);

        if (!RegisterClassEx(&wcex)) {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Could not register WebView UI for WebView: '%s'. Please file a bug at https://github.com/caitp/TwitchSwitcher",
                formatError(GetLastError()).c_str());
            return nullptr;
        }
    }

    //m_hasScrollbars = showScrollBars;
    HWND hwnd = CreateWindowExA(0, TSW_WEBVIEW_WNDCLASS, "", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        0, 0, 100, 100, parent, (HMENU)(LONG_PTR)id, hInstance, (LPVOID)this);
    return hwnd;
}

void WebContent::open(const std::string& url, const HttpRequestOptions& options) {
    m_didFinishRequest = false;
    if (m_browser == nullptr)
        return;

    bool fail = false;
    char16_t* urlUtf16 = nullptr, urlBuffer[256];
    char16_t* headersUtf16 = nullptr, headersBuffer[512];

    VARIANT empty;
    VariantInit(&empty);


    urlUtf16 = utf8ToUtf16(url, urlBuffer, arraysize(urlBuffer));
    if (urlUtf16 == nullptr) goto fail;

    VARIANT headers;
    VariantInit(&empty);

    const std::map<std::string, std::string>& requestHeaders = options.headers();
    if (!requestHeaders.empty()) {
        std::string joined;
        for (auto pair : requestHeaders)
            joined += pair.first + ": " + pair.second + ";\r\n";
        headersUtf16 = utf8ToUtf16(joined, headersBuffer, arraysize(headersBuffer));
        if (headersUtf16 == nullptr) goto fail;

        BSTR bstrHeaders = SysAllocString(reinterpret_cast<const OLECHAR*>(headersUtf16));
        headers.vt = VT_BSTR;
        headers.bstrVal = bstrHeaders;
    }

    BSTR bstrURL = SysAllocString(reinterpret_cast<const OLECHAR*>(urlUtf16));
    HRESULT result = m_browser->Navigate(bstrURL, &empty, &empty, &empty, &headers);

    if (!SUCCEEDED(result)) goto fail;

    m_browser->put_Visible(VARIANT_TRUE);

    goto success;

fail:
    m_browser->Quit();
    // Fall through to deallocate

success:
    if (urlUtf16 != nullptr && urlUtf16 != urlBuffer)
        delete[] urlUtf16;
    if (headersUtf16 != nullptr && headersUtf16 != headersBuffer)
        delete[] headersUtf16;
}

void WebContent::close() {
    if (this) {
        if (m_browser) {
            m_browser->Stop();
            m_browser->Quit();
            m_browser->Release();
            m_browser = nullptr;
        }

        m_hwnd = nullptr;
        Release();
    }
}

HRESULT STDMETHODCALLTYPE WebContent::QueryInterface(REFIID riid, void** result) {
    *result = NULL;

    if (riid == IID_IUnknown || riid == IID_IOleClientSite) {
        *result = static_cast<IOleClientSite*>(this);
    } else if (riid == IID_IOleWindow || riid == IID_IOleInPlaceSite) {
        *result = static_cast<IOleInPlaceSite*>(this);
    } else if (riid == IID_IOleInPlaceUIWindow) {
        *result = static_cast<IOleInPlaceUIWindow*>(this);
    } else if (riid == IID_IOleInPlaceFrame) {
        *result = static_cast<IOleInPlaceFrame*>(this);
    } else if (riid == IID_IDispatch) {
        *result = static_cast<IDispatch*>(this);
    } else if (riid == IID_IDocHostUIHandler) {
        *result = static_cast<IDocHostUIHandler*>(this);
    } else if (riid == IID_IDocHostShowUI) {
        *result = static_cast<IDocHostShowUI*>(this);
    }

    if (*result != NULL) {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

// IDispatch
HRESULT STDMETHODCALLTYPE WebContent::Invoke(DISPID member, REFIID riid, LCID lcid, WORD flags, DISPPARAMS* params, VARIANT* result, EXCEPINFO* excepInfo, UINT* argErr) {
    switch (member) {
    case DISPID_BEFORENAVIGATE2: {
        /* BeforeNavigate2(IDispatch* dispatch, BSTR& url, INT flags, BSTR& targetFrameName, BSTR postData, BSTR headers, BOOL& cancel) */
        String url = utf16ToUtf8(COM::getParameter<VT_BSTR | TSW_VARIANTREF>(params, 1));
        String headers = utf16ToUtf8(COM::getParameter<VT_BSTR | TSW_VARIANTREF>(params, 5));
        m_isNavigating = true;

        // Avoid redirect callback for weird ieframe nonsense
        if (url.startsWith("res://ieframe.dll"))
            break;

        if (m_impl && m_impl->m_onRedirect) {
            auto onRedirect = m_impl->m_onRedirect;
            std::string body;
            m_impl->m_url = url;
            // TODO(caitp): ASSERT(m_impl->m_webView)
            // Retain the webview through the two potential callbacks.
            Ref<WebView> webView = *m_impl->m_webView.get();
            OnRedirect result = onRedirect(url.toStdString(), std::string());
            if (result != OnRedirect::Follow) {
                bool* cancel = COM::getParameter<VT_BOOL | VT_BYREF>(params, 6);
                *cancel = true;
                m_isNavigating = false;
                m_didFinishRequest = true;
                if (m_impl && m_impl->m_onComplete) {
                    auto onComplete = m_impl->m_onComplete;
                    onComplete(webView.get(), url);
                }
                // Navigation was cancelled, so notify the caller.
            }
        }
        break;
    }
    case DISPID_DOCUMENTCOMPLETE: {
        m_isNavigating = false;
        if (!m_didFinishRequest) {
            if (m_impl && m_impl->m_onComplete && m_impl->m_webView) {
                Ref<WebView> webView = *m_impl->m_webView.get();
                auto onComplete = m_impl->m_onComplete;
                onComplete(webView.get(), m_impl->m_url);
            }
            m_didFinishRequest = true;
        }
        break;
    }

    case DISPID_NAVIGATECOMPLETE2:
        break;

    case DISPID_AMBIENT_DLCONTROL:
        result->vt = VT_I4;
        result->lVal = DLCTL_DLIMAGES | DLCTL_VIDEOS | DLCTL_BGSOUNDS | DLCTL_SILENT;
        break;

    case DISPID_FILEDOWNLOAD: {
        /* FileDownload(BOOL isActiveDocument, BOOL& cancel) */
        bool isActiveDocument = COM::getParameter<VT_BOOL>(params, 0);
        bool* cancel = COM::getParameter<VT_BOOL | VT_BYREF>(params, 1);

        // Avoid downloading files unrelated to the current document...
        if (!isActiveDocument)
            *cancel = true;
        break;
    }

    case DISPID_NAVIGATEERROR: {
        /* NavigateError(IDispatch* dispatch, BSTR& url, BSTR& targetFrameName, INT statusCode, BOOL& cancel)
           Fired to indicate the a binding error has occured */
        String url = utf16ToUtf8(COM::getParameter<VT_BSTR | TSW_VARIANTREF>(params, 1));
        String targetFrameName = utf16ToUtf8(COM::getParameter<VT_BSTR | TSW_VARIANTREF>(params, 2));
        int status = COM::getParameter<VT_I4 | TSW_VARIANTREF>(params, 3);
        bool* cancel = COM::getParameter<VT_BOOL | VT_BYREF>(params, 4);
        LOG(LOG_WARNING, "Navigation failed with status %d. Please file a bug at https://github.com/caitp/TwitchSwitch.", status);
        break;
    }
    case DISPID_PRIVACYIMPACTEDSTATECHANGE: {
        // Fired when the user's browsing experience is impacted
        break;
    }

    case DISPID_NEWPROCESS: {
        // Fired when a navigation must be redirected due to Protected Mode
        break;
    }
    case DISPID_THIRDPARTYURLBLOCKED: {
        String url = utf16ToUtf8(COM::getParameter<VT_BSTR | TSW_VARIANTREF>(params, 0));
        LOG(LOG_WARNING, "Blocked access to third party URL: %s", url.characters());
        break;
    }

    default:
        return DISP_E_MEMBERNOTFOUND;
    }
    return S_OK;
}

// IDocHostUIHandler
HRESULT STDMETHODCALLTYPE WebContent::GetHostInfo(DOCHOSTUIINFO* info) {
    info->dwFlags = DOCHOSTUIFLAG_NO3DOUTERBORDER | DOCHOSTUIFLAG_ENABLE_REDIRECT_NOTIFICATION;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE WebContent::GetExternal(IDispatch** result) {
    *result = reinterpret_cast<IDispatch*>(this);
    return S_OK;
}

// IOleWindow (COleInPlaceSite)
HRESULT STDMETHODCALLTYPE WebContent::GetWindow(HWND* result) {
    *result = m_hwnd;
    return S_OK;
}

// IOleInPlaceSite
HRESULT STDMETHODCALLTYPE WebContent::GetWindowContext(IOleInPlaceFrame** frame, IOleInPlaceUIWindow** doc, LPRECT posRect, LPRECT clipRect, LPOLEINPLACEFRAMEINFO info) {
    *frame = reinterpret_cast<IOleInPlaceFrame*>(this);
    AddRef();
    *doc = nullptr;
    info->fMDIApp = FALSE;
    info->hwndFrame = m_hwnd;
    info->haccel = 0;
    info->cAccelEntries = 0;
    GetClientRect(m_hwnd, posRect);
    GetClientRect(m_hwnd, clipRect);
    return S_OK;
}
HRESULT STDMETHODCALLTYPE WebContent::OnPosRectChange(LPCRECT posRect) {
    IOleInPlaceObject* oleObj = nullptr;
    if (m_browser == nullptr) {
        return E_FAIL;
    }

    m_browser->QueryInterface(IID_IOleInPlaceObject, (void**)&oleObj);

    if (oleObj != nullptr) {
        oleObj->SetObjectRects(posRect, posRect);
        oleObj->Release();
    }

    return S_OK;
}

// IOleInPlaceFrame
HRESULT STDMETHODCALLTYPE WebContent::TranslateAccelerator(LPMSG msg, WORD id) {
    HRESULT result = S_FALSE;
    IOleInPlaceActiveObject* frame = nullptr;
    if (m_browser != nullptr && SUCCEEDED(m_browser->QueryInterface(IID_IOleInPlaceActiveObject, (void**)&frame))) {
        result = frame->TranslateAcceleratorA(msg);
        frame->Release();
    }
    return result;
}


// IDocHostShowUI
HRESULT STDMETHODCALLTYPE WebContent::ShowMessage(HWND hwnd, LPOLESTR text, LPOLESTR caption, DWORD type, LPOLESTR helpFile, DWORD helpContext, LRESULT* result) {
    return S_FALSE;
}

// IOleClientSite
HRESULT STDMETHODCALLTYPE WebContent::OnShowWindow(BOOL show) {
    if (show) {
        BOOL ret = SetForegroundWindow(m_hwnd);
        SetActiveWindow(m_hwnd);
    }
    return S_OK;
}


//
//
//
WebContent* findWebContentForHWND(HWND hwnd) {
    char className[(arraysize(TSW_WEBVIEW_WNDCLASS) / sizeof(int) + 1) * sizeof(int)] = "";
    const size_t kStringSize = arraysize(TSW_WEBVIEW_WNDCLASS) - 1;
    const size_t kBufferSize = arraysize(className);
    do {
        UINT result = RealGetWindowClassA(hwnd, className, arraysize(className));
        if (result == kStringSize && ::strncmp(className, TSW_WEBVIEW_WNDCLASS, kStringSize + 1) == 0)
            return reinterpret_cast<WebContent*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    } while (hwnd = GetParent(hwnd));
    return nullptr;
}

DWORD WINAPI WebViewImpl::messagePumpThreadProc(LPVOID param) {
    auto currentThreadId = GetCurrentThreadId();
    MSG msg;

    HRESULT hresult;
    if (FAILED(hresult = OleInitialize(NULL))) {
        // FIXME: Use obs localization API
        LOG(LOG_WARNING, "Failed to initialize OLE layer: '%s' (%8x). Please file a bug at https://github.com/caitp/TwitchSwitcher",
            formatError(hresult).c_str(), hresult);
        ExitThread(1);
    }

    ZeroMemory(&msg, sizeof msg);
    PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE);
    while (true) {
        auto status = GetMessage(&msg, nullptr, 0, 0);
        if (status == 0)
            break;
        else if (status > 0) {
            switch (msg.message) {
            case kCloseWebViewUI: {
                WebViewImpl* self = reinterpret_cast<WebViewImpl*>(msg.lParam);
                self->closeInternal();
                WakeConditionVariable(&self->m_conditionVariable);
                break;
            }
            case kCreateWebViewUI: {
                MessageData* data = reinterpret_cast<MessageData*>(msg.lParam);
                HWND* result = reinterpret_cast<HWND*>(data->result);
                WebViewImpl* self = data->self;
                *result = self->createInternal();
                WakeConditionVariable(&self->m_conditionVariable);
                break;
            }
            case kSetWebViewTitle: {
                wchar_t* title = reinterpret_cast<wchar_t*>(msg.lParam);
                DefWindowProcW(msg.hwnd, WM_SETTEXT, 0, (LPARAM)title);
                delete[] title;
                break;
            }
            default:
                TranslateMessage(&msg);
                if (msg.message == WM_KEYDOWN) {
                    WebContent* content = findWebContentForHWND(msg.hwnd);
                    if (content != nullptr)
                        content->TranslateAcceleratorA(&msg, 0);
                }
                DispatchMessage(&msg);
                break;
            }
        } else {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Error occurred in WebView messagePumpThreadProc: %s (%d)", formatError(static_cast<HRESULT>(status)).c_str(), status);
        }

    }

    OleUninitialize();
    ExitThread(0);
    return 0;
}

}  // namespace twitchsw

#endif
