// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#ifdef TSW_WEBVIEW_WKWEBVIEW

#include <twitchsw/twitchsw.h>
#include <twitchsw/webview.h>

#include <string>

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <WebKit/WebKit.h>

namespace twitchsw {

class WebViewImpl {
public:
    WebViewImpl();
    ~WebViewImpl();

    static void initialize();
    static void shutdown();

    void open(const std::string& url, const HttpRequestOptions& options);
    void close();
    void setTitle(const std::string& title);
    void show();

    void setOnRedirect(const OnRedirectCallback& callback);
    void setOnComplete(const _OnCompleteCallback& callback);
    void setOnWebViewDestroyed(const _OnWebViewDestroyed& callback);

    void* nativeHandle();
private:
    WKWebView* m_browser;
};

}  // namespace twitchsw

#endif  // TSW_WEBVIEW_WKWEBVIEW
