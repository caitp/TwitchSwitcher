// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#ifdef TSW_WEBVIEW_WIN32

//#include "windows/eventsink.h"

#include <windows.h>
#include <Exdisp.h>

#include "windows/COleClientSite.h"
#include "windows/CDispatch.h"
#include "windows/CDocHostShowUI.h"
#include "windows/CDocHostUIHandler.h"
#include "windows/COleInPlaceSite.h"
#include "windows/COleInPlaceFrame.h"

#define TSW_WEBVIEW_WNDCLASS "TSW.WebView"

namespace twitchsw {

#pragma warning(suppress : 4584)
class WebContent : public IUnknown, COleClientSite, CDispatch, CDocHostShowUI, CDocHostUIHandler, COleInPlaceSite, COleInPlaceFrame {
private:
    //typedef COMEventSink<WebViewImpl, IWebBrowser2, DWebBrowserEvents2> WebBrowserEvents2Handler;

public:
    WebContent(WebViewImpl*);
    ~WebContent();

    void open(const std::string& url, const HttpRequestOptions& options);
    void close();
    HWND nativeHandle() { return m_hwnd; }

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** result) override;
    TSW_IMPLEMENT_COMMON_METHODS();

    // IDispatch
    HRESULT STDMETHODCALLTYPE Invoke(DISPID member, REFIID riid, LCID lcid, WORD flags, DISPPARAMS* params, VARIANT* result, EXCEPINFO* excepInfo, UINT* argErr) override;

    // IDocHostUIHandler
    HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO* info) override;
    HRESULT STDMETHODCALLTYPE GetExternal(IDispatch** result) override;

    // IOleWindow (COleInPlaceSite)
    HRESULT STDMETHODCALLTYPE GetWindow(HWND* result) override;

    // IOleInPlaceSite
    HRESULT STDMETHODCALLTYPE GetWindowContext(IOleInPlaceFrame** frame, IOleInPlaceUIWindow** doc, LPRECT posRect, LPRECT clipRect, LPOLEINPLACEFRAMEINFO info) override;
    HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT posRect) override;

    // IOleInPlaceFrame
    HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG msg, WORD id) override;

    // IDocHostShowUI
    HRESULT STDMETHODCALLTYPE ShowMessage(HWND hwnd, LPOLESTR text, LPOLESTR caption, DWORD type, LPOLESTR helpFile, DWORD helpContext, LRESULT* result) override;

    // IOleClientSite
    HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL show) override;

protected:
    HRESULT setupOLE();
    HWND create(HWND parent, HINSTANCE hInstance, UINT id, bool showScrollBars);

    static LRESULT CALLBACK WebContentWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    virtual LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    friend class WebViewImpl;
    TSW_DECLARE_IUNKNOWN_MEMBERS();

    WebViewImpl* m_impl;
    IWebBrowser2* m_browser;
    HWND m_hwnd; // webview HWND
    DWORD m_cookie;
    bool m_showScrollBars;

    bool m_initializedOle;
    bool m_didFinishRequest;
    bool m_isNavigating;

    //WebBrowserEvents2Handler* m_browserEvents2;
    OnRedirectCallback m_onRedirect;
    _OnCompleteCallback m_onComplete;
};

class WebViewImpl {
public:
    WebViewImpl();
    ~WebViewImpl();

    void open(const std::string& url, const HttpRequestOptions& options);
    void close();

    static void initialize();
    static void shutdown();

    HWND nativeHandle() {
        if (m_content)
            return m_hwnd;
        return 0;
    }

    void setOnWebViewDestroyed(const _OnWebViewDestroyed& callback) {
        m_onWebViewDestroyed = callback;
    }

    void setOnRedirect(const OnRedirectCallback& callback) {
        if (m_content)
            m_content->m_onRedirect = callback;
    }

    void setOnComplete(const _OnCompleteCallback& callback) {
        if (m_content)
            m_content->m_onComplete = callback;
        m_onComplete = callback;
    }

    void show();

protected:
    virtual bool ensureUI();
    HWND create(HINSTANCE hInstance, UINT x, UINT y, UINT width, UINT height, bool showScrollBars);

    void closeInternal();
    HWND createInternal();

private:
    friend class WebContent;
    friend class WebView;

    static LRESULT CALLBACK WebWindowWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // A new thread is created with its own message pump for each WebViewImpl and its children. So don't create too
    // many of these!
    // TODO(caitp): Refactor to only use a single global thread for MSHTML message loops.
    static DWORD WINAPI messagePumpThreadProc(LPVOID param);

    static DWORD g_threadId;
    static HANDLE g_thread;
    CONDITION_VARIABLE m_conditionVariable;
    CRITICAL_SECTION m_criticalSection;

    HWND m_hwnd;
    HINSTANCE m_hinstance;
    WebContent* m_content;
    bool m_showScrollBars;
    _OnWebViewDestroyed m_onWebViewDestroyed;
    _OnCompleteCallback m_onComplete;
};

}

#endif
