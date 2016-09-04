// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include "mac/webview-wkwebview.h"

#ifdef TSW_WEBVIEW_WKWEBVIEW

namespace twitchsw {

// ??? obs_create_ui doesn't seem to matter anyways, investigate this...
const char* WebView::kOBS_UI_TYPE = "mac";

void WebViewImpl::initialize() {}
void WebViewImpl::shutdown() {}

WebViewImpl::WebViewImpl()
    : m_browser(nullptr)
{
}

WebViewImpl::~WebViewImpl()
{
}

void* WebViewImpl::nativeHandle()
{
}

void WebViewImpl::open(const std::string& url, const HttpRequestOptions& options)
{
}

void WebViewImpl::close()
{
}

void WebViewImpl::setTitle(const std::string& title)
{
}

void WebViewImpl::show()
{
}

void WebViewImpl::setOnRedirect(const OnRedirectCallback& callback)
{
}

void WebViewImpl::setOnComplete(const _OnCompleteCallback& callback)
{
}

void WebViewImpl::setOnWebViewDestroyed(const _OnWebViewDestroyed& callback)
{
}

}  // namespace twitchsw

#endif  // TSW_WEBVIEW_WKWEBVIEW
