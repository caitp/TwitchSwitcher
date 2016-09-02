// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <string>

#include <twitchsw/twitchsw.h>
#include <twitchsw/http.h>

namespace twitchsw {

class WebViewImpl;
class WebView;
typedef std::function<void()> _OnWebViewDestroyed;
typedef std::function<void()> _OnCompleteCallback;
typedef std::function<void(WebView&)> OnCompleteCallback;

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
    WebView& show();
    WebView& close();

private:
    struct Data : public RefCounted<Data> {
        Data(WebViewImpl* impl) : m_impl(impl) {}
        ~Data();
        WebViewImpl* m_impl;
        OnCompleteCallback m_onComplete;
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
