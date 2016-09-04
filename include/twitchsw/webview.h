// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <string>

#include <twitchsw/twitchsw.h>
#include <twitchsw/http.h>

namespace twitchsw {

#define TSW_WEBVIEW_DEFAULT_TITLE "WebView"

class WebViewImpl;
class WebView;
typedef std::function<void()> _OnWebViewDestroyed;
typedef std::function<void()> _OnCompleteCallback;
typedef std::function<void()> _OnAbortCallback;
typedef std::function<void(WebView&)> OnCompleteCallback;
typedef std::function<void(WebView&)> OnAbortCallback;

class WebView {
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
        static const std::string defaultTitle = TSW_WEBVIEW_DEFAULT_TITLE;
        return m_data ? m_data->m_title : defaultTitle;
    }

private:
    struct Data : public RefCounted<Data> {
        Data(WebViewImpl* impl) : RefCounted<Data>(), m_impl(impl) {}
        ~Data();
        WebViewImpl* m_impl;
        OnCompleteCallback m_onComplete;
        OnAbortCallback m_onAbort;
        std::string m_title = TSW_WEBVIEW_DEFAULT_TITLE;
    };
    Data* m_data;

    WebView(Data* data);

    WebViewImpl* getOrCreateImpl();
    Data* getOrCreateData();

    Data* data() { return m_data; }
    const Data* data() const { return m_data; }
    WebViewImpl* impl() { return m_data ? m_data->m_impl : nullptr; }
    const WebViewImpl* impl() const { return m_data ? m_data->m_impl : nullptr; }
};

}
