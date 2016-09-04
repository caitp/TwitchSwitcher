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
WebView::WebView() {
    m_data = nullptr;
}

WebView::WebView(WebView::Data* data)
    : m_data(data)
{
    data->ref();
}

WebView::~WebView() {
    if (m_data && !m_data->deref())
        delete m_data;
}

void WebView::initialize() {
    WebViewImpl::initialize();
}

void WebView::shutdown() {
    WebViewImpl::shutdown();
}

WebView::Data::~Data() {
    for (auto p : m_weakPtrs)
        *p = nullptr;
    if (m_impl) {
        m_impl->close();
        m_impl = nullptr;
    }
}

WebViewImpl* WebView::getOrCreateImpl() {
    Data* data = getOrCreateData();
    if (!data->m_impl) {
        data->m_impl = new WebViewImpl();
        data->m_impl->setTitle(data->m_title);
    }
    return data->m_impl;
}

WebView::Data* WebView::getOrCreateData() {
    if (m_data)
        return m_data;
    m_data = new Data { nullptr };
    return m_data;
}

WebView& WebView::open(const std::string& url, const HttpRequestOptions& options) {
    WebViewImpl* p = getOrCreateImpl();
    Data* data = m_data;

    // Will leak unless WebViewImpl calls the OnWebViewDestroyed callback.
    data->ref();
    p->setOnWebViewDestroyed([data]() {
        if (!data->deref())
            delete data;
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
    Data* data = getOrCreateData();
    WebViewImpl* impl = data->m_impl;
    if (impl)
        impl->setTitle(title);
    data->m_title = title;
    return *this;
}

WebView& WebView::setOnRedirect(const OnRedirectCallback& callback) {
    WebViewImpl* p = getOrCreateImpl();
    p->setOnRedirect(callback);

    return *this;
}

WebView& WebView::setOnComplete(const OnCompleteCallback& callback) {
    WebViewImpl* impl = getOrCreateImpl();
    Data* data = m_data;
    data->m_onComplete = callback;
    impl->setOnComplete([data]() {
        if (data->m_onComplete) {
            WebView webview(data);
            data->m_onComplete(webview);
        }
    });
    return *this;
}

WebView& WebView::setOnAbort(const OnAbortCallback& callback) {
    WebViewImpl* impl = getOrCreateImpl();
    Data* data = m_data;
    data->m_onAbort = callback;
    impl->setOnAbort([data]() {
        if (data->m_onAbort) {
            WebView webview(data);
            data->m_onAbort(webview);
        }
    });
    return *this;
}

WebView& WebView::show() {
    if (impl()) {
        auto window = impl()->nativeHandle();
        if (window != nullptr) {
            auto ui = obs_create_ui("webview", "webview", kOBS_UI_TYPE, nullptr, window);
            impl()->show();
        }

    }

    return *this;
}

}
