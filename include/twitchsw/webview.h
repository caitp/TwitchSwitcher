// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <mutex>
#include <string>

#include <twitchsw/twitchsw.h>
#include <twitchsw/http.h>
#include <twitchsw/refs.h>

namespace twitchsw {

#define TSW_WEBVIEW_DEFAULT_TITLE "WebView"

class WebViewImpl;
class WebView;
typedef std::function<void()> _OnWebViewDestroyed;
typedef std::function<void()> _OnCompleteCallback;
typedef std::function<void()> _OnAbortCallback;
typedef std::function<void(WebView&)> OnCompleteCallback;
typedef std::function<void(WebView&)> OnAbortCallback;

class WebView : public ThreadSafeRefCounted<WebView> {
public:
    explicit WebView();
    ~WebView();

    static const char* kOBS_UI_TYPE;


    static void initialize();
    static void shutdown();

    WebView& open(const std::string& url, const HttpRequestOptions& options);
    WebView& open(const std::string& url) {
        Http http;
        HttpRequestOptions options = http.request();
        return open(url, options);
    }
    WebView& open(const std::string& url, const std::map<std::string, std::string>& headers) {
        Http http;
        HttpRequestOptions options = http.request();
        options.m_headers = headers;
        return open(url, options);
    }

    WebView& setOnRedirect(const OnRedirectCallback& callback);
    WebView& setOnComplete(const OnCompleteCallback& callback);
    WebView& setOnAbort(const OnAbortCallback& callback);
    WebView& show();
    WebView& close();
    WebView& setTitle(const std::string& title);
    const std::string& title() const {
        return m_title;
    }

    WeakPtr<WebView> createWeakPtr();

private:
    WebViewImpl* getOrCreateImpl();
    WebViewImpl* m_impl;
    OnCompleteCallback m_onComplete;
    OnAbortCallback m_onAbort;
    std::string m_title = TSW_WEBVIEW_DEFAULT_TITLE;
    WeakPtrFactory<WebView>* m_weakPtrs = nullptr;

    WebViewImpl* impl() { return m_impl; }
    const WebViewImpl* impl() const { return m_impl; }
};

}  // namespace twitchsw
