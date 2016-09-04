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

#import "mac/WebViewController.h"

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
    void setOnAbort(const _OnAbortCallback& callback);

    void* nativeHandle();

    // Helpers for WebViewController
    const OnRedirectCallback& onRedirect() const { return m_onRedirect; }
    const _OnCompleteCallback& onComplete() const { return m_onComplete; }
    const _OnWebViewDestroyed& onDestroyed() const { return m_onDestroyed; }
    const _OnAbortCallback& onAbort() const { return m_onAbort; }

protected:
    bool ensureUI();

private:
    WebViewController* m_controller;
    NSWindow* m_window;
    OnRedirectCallback m_onRedirect;
    _OnCompleteCallback m_onComplete;
    _OnWebViewDestroyed m_onDestroyed;
    _OnAbortCallback m_onAbort;

    std::string m_title;
};

}  // namespace twitchsw

#endif  // TSW_WEBVIEW_WKWEBVIEW
