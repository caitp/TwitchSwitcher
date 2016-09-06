// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <twitchsw/webview.h>

#ifdef TSW_WEBVIEW_WIN32
#include "windows/webview-windows.h"
#elif defined(TSW_WEBVIEW_WKWEBVIEW)
#include "mac/webview-wkwebview.h"
#else
#error "Not supported on this platform yet."
#endif

#include <obs.h>

namespace twitchsw {


//
// WebView public API
//
WebView::WebView()
    : m_impl(nullptr) {
}

WebView::~WebView() {
    LOG(LOG_INFO, "[WebView] destroyed");
    if (m_weakPtrs)
        delete m_weakPtrs;
}

void WebView::initialize() {
    WebViewImpl::initialize();
}

void WebView::shutdown() {
    WebViewImpl::shutdown();
}

WebViewImpl* WebView::getOrCreateImpl() {
    if (LIKELY(m_impl))
        return m_impl;
    m_impl = new WebViewImpl();
    m_impl->m_webView = this;
    m_impl->setTitle(m_title);
    return m_impl;
}

WebView& WebView::open(const std::string& url, const HttpRequestOptions& options) {
    WebViewImpl* p = getOrCreateImpl();

    // Will leak unless WebViewImpl calls the OnWebViewDestroyed callback.
    WeakPtr<WebView> weakThis = createWeakPtr();
    p->setOnWebViewDestroyed([weakThis]() {
        Ref<WebView> webView = *weakThis.get();
        if (webView->m_onAbort)
            webView->m_onAbort(webView.get());
    });
    p->open(url, options);

    return *this;
}

WebView& WebView::close() {
    auto p = impl();
    if (p)
        p->close();
    return *this;
}

WebView& WebView::setTitle(const std::string& title) {
    WebViewImpl* impl = m_impl;
    m_title = title;
    if (impl)
        impl->setTitle(title);
    return *this;
}

WebView& WebView::setOnRedirect(const OnRedirectCallback& callback) {
    getOrCreateImpl()->setOnRedirect(callback);
    return *this;
}

WebView& WebView::setOnComplete(const OnCompleteCallback& callback) {
    WebViewImpl* impl = getOrCreateImpl();
    m_onComplete = callback;
    WeakPtr<WebView> refThis = createWeakPtr();
    impl->setOnComplete([refThis]() {
        // Caller of this callback should have a ref to WebView keeping the
        // weak ref alive.
        Ref<WebView> webView = *refThis.get();
        if (webView->m_onComplete)
            webView->m_onComplete(webView.get());
    });
    return *this;
}

WebView& WebView::setOnAbort(const OnAbortCallback& callback) {
    WebViewImpl* impl = getOrCreateImpl();
    m_onAbort = callback;
    WeakPtr<WebView> refThis = createWeakPtr();
    impl->setOnAbort([&refThis]() {
        Ref<WebView> webView = *refThis.get();
        if (webView->m_onAbort)
            webView->m_onAbort(webView.get());
    });
    return *this;
}

WebView& WebView::show() {
    if (m_impl) {
        auto window = m_impl->nativeHandle();
        if (window != nullptr) {
            auto ui = obs_create_ui("webview", "webview", kOBS_UI_TYPE, nullptr, window);
            m_impl->show();
        }
    }

    return *this;
}

WeakPtr<WebView> WebView::createWeakPtr() {
    if (m_weakPtrs == nullptr)
        m_weakPtrs = new WeakPtrFactory<WebView>(this);
    return m_weakPtrs->createWeakPtr();
}

}
