// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <string>
#include <map>

namespace twitchsw {

class HttpResponse {
public:
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

class Http {
public:
    static void Shutdown();
    static HttpResponse GET(const std::string& url, const std::map<std::string, std::string>& headers);
    static HttpResponse GET(const std::string& url) {
        std::map<std::string, std::string> headers;
        return Http::GET(url, headers);
    }

    static HttpResponse PUT(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& body);
    static HttpResponse PUT(const std::string& url, const std::map<std::string, std::string>& headers, const void* data, size_t length) {
        std::string body(static_cast<const char*>(data), length);
        return Http::PUT(url, headers, body);
    }
    static HttpResponse PUT(const std::string& url, const std::string& body) {
        std::map<std::string, std::string> headers;
        return Http::PUT(url, headers, body);
    }
    static HttpResponse PUT(const std::string& url, const void* data, size_t length) {
        const std::string body(static_cast<const char*>(data), length);
        return Http::PUT(url, body);
    }

    HttpResponse get(const std::string& url, const std::map<std::string, std::string>& extraHeaders) {
        std::map<std::string, std::string> headers;
        headers.insert(m_defaultHeaders.begin(), m_defaultHeaders.end());
        for (auto pair : extraHeaders) {
            headers.insert_or_assign(pair.first, pair.second);
        }
        return Http::GET(url, headers);
    }

    HttpResponse get(const std::string& url) {
        return Http::GET(url, m_defaultHeaders);
    }

    Http& setHeader(const std::string& name, const std::string& value) {
        m_defaultHeaders[name] = value;
        return *this;
    }

private:
    static bool initializeCURLIfNeeded();
    std::map<std::string, std::string> m_defaultHeaders;
};

}  // namespace twitchsw
