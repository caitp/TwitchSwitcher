// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <functional>
#include <list>
#include <map>
#include <string>

namespace twitchsw {

enum class OnRedirect {
    Fail,
    Follow,
    Finish
};

class HttpResponse {
public:
    HttpResponse() : m_status(-1), m_content() {}
    explicit HttpResponse(int status, const std::string& content)
        : m_status(status)
        , m_content(content)
    {}
    explicit HttpResponse(int status)
        : m_status(status)
        , m_content("")
    {}
    HttpResponse(const HttpResponse& other)
        : m_status(other.m_status)
        , m_content(other.m_content)
    {}
    HttpResponse& operator=(HttpResponse&& other) {
        m_status = other.m_status;
        m_content = std::move(other.m_content);
        return *this;
    }
    HttpResponse& operator=(const HttpResponse& other) = default;
    ~HttpResponse() = default;

    int status() const { return m_status; }
    const std::string& content() const { return m_content; }

protected:
    int m_status;
    std::string m_content;
};

struct URLQueryParameter {
    URLQueryParameter(const std::string& paramName) : name(paramName) {}
    URLQueryParameter(const std::string& paramName, const std::string& paramValue) : name(paramName), value(paramValue) {}
    std::string name;
    std::string value = "";
};

typedef std::function<OnRedirect(const std::string& url, const std::string& content)> OnRedirectCallback;

class Http;
class HttpRequestOptions {
public:
    explicit HttpRequestOptions(Http* http) : m_http(http) {}

    HttpResponse get(const std::string& url);
    HttpResponse put(const std::string& url, const std::string& body);
    HttpResponse put(const std::string& url, const void* data, size_t length);

    HttpRequestOptions& setHeader(const std::string& key, const std::string& value) {
        m_headers.insert_or_assign(key, value);
        return *this;
    }
    const std::map<std::string, std::string>& headers() const { return m_headers; }

    HttpRequestOptions& setParameter(const std::string& key) {
        m_parameters.push_back(URLQueryParameter(key));
        return *this;
    }

    HttpRequestOptions& setParameter(const std::string& key, const std::string& value) {
        m_parameters.push_back(URLQueryParameter(key, value));
        return *this;
    }

    HttpRequestOptions& setOnRedirect(const OnRedirectCallback& callback) {
        m_onRedirect = callback;
        return *this;
    }
    const OnRedirectCallback& onRedirect() const { return m_onRedirect; }

private:
    friend class WebView;
    friend class WebViewImpl;
    HttpRequestOptions& mergeHeaders(const std::map<std::string, std::string> headers) {
        if (headers.empty()) return *this;
        if (m_headers.empty()) {
            m_headers.insert(headers.begin(), headers.end());
            return *this;
        }

        std::map<std::string, std::string> newHeaders;
        newHeaders.insert(headers.begin(), headers.end());
        for (auto pair : m_headers) {
            newHeaders.insert_or_assign(pair.first, pair.second);
        }
        m_headers = newHeaders;
        return *this;
    }

    friend class Http;
    Http* m_http;
    std::map<std::string, std::string> m_headers;
    std::list<URLQueryParameter> m_parameters;
    OnRedirectCallback m_onRedirect;
};

class Http {
public:
    static void Shutdown();
    static HttpResponse GET(const std::string& url, const HttpRequestOptions& options);
    static HttpResponse PUT(const std::string& url, const std::string& body, const HttpRequestOptions& options);

    HttpResponse get(const std::string& url, HttpRequestOptions& options) {
        return Http::GET(url, options.mergeHeaders(m_defaultHeaders));
    }

    HttpResponse put(const std::string& url, const std::string& body, HttpRequestOptions& options) {
        return Http::PUT(url, body, options.mergeHeaders(m_defaultHeaders));
    }

    Http& setHeader(const std::string& name, const std::string& value) {
        m_defaultHeaders[name] = value;
        return *this;
    }

    HttpRequestOptions request() { return HttpRequestOptions(this); }

private:
    static bool initializeCURLIfNeeded();
    std::map<std::string, std::string> m_defaultHeaders;
};

inline HttpResponse HttpRequestOptions::get(const std::string& url) {
    return m_http->get(url, *this);
}
inline HttpResponse HttpRequestOptions::put(const std::string& url, const std::string& body) {
    return m_http->put(url, body, *this);
}
inline HttpResponse HttpRequestOptions::put(const std::string& url, const void* data, size_t length) {
    return put(url, std::string(static_cast<const char*>(data), length));
}

}  // namespace twitchsw
