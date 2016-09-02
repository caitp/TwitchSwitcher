// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <algorithm>

#include <curl/curl.h>

#include <twitchsw/twitchsw.h>
#include <twitchsw/http.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace twitchsw {

static bool g_didInitCURL = false;

class CURLRequest {
public:
    CURLRequest(const std::string& requestUrl) : url(requestUrl) {}
    ~CURLRequest() {
        if (curl) curl_easy_cleanup(curl);
    }
    int send() {
        long status = -1;
        if (curl != nullptr) return -1;

        curl = curl_easy_init();
        if (curl == nullptr) return -1;

        std::string reqUrl = url;
        int params = 0;
        for (auto it : parameters) {
            auto value = it.value;
            std::string result = it.name;

            if (!value.empty()) {
                char* escaped = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.length()));
                if (!escaped)
                    continue;
                result += "=" + std::string(escaped);
                curl_free(escaped);
            }

            if (params++)
                reqUrl += "&" + result;
            else
                reqUrl += "?" + result;
        }

        CURLcode res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_URL, reqUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        curl_easy_setopt(curl, CURLOPT_DNS_USE_GLOBAL_CACHE, false);
        curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, 2);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, static_cast<void*>(this));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLRequest::receiveData);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        if (method != "GET") {
            if (body.length()) {
                curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
            }
            if (method == "PUT")
                curl_easy_setopt(curl, CURLOPT_PUT, 1L);
            else if (method == "POST")
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
            else
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
            curl_easy_setopt(curl, CURLOPT_READDATA, this);
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, CURLRequest::sendData);
        }

    retry:
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);

        if (status >= 300 && status <= 303 && onRedirect) {
            char* redirectUrl = nullptr;
            // Redirect occurred --- Call the on redirect callback.
            if (curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redirectUrl) == CURLE_OK) {
                OnRedirect command = onRedirect(std::string(redirectUrl), buffer);
                if (command == OnRedirect::Follow) {
                    curl_easy_setopt(curl, CURLOPT_URL, redirectUrl);
                    buffer.clear();
                    goto retry;
                }
                if (command == OnRedirect::Finish) {
                    buffer.clear();
                }
            }
        }

        if (res != CURLE_OK) {
            LOG_HTTP(LOG_DEBUG, "`%s %s` (%d) failed: %s", method.c_str(), reqUrl.c_str(), status, curl_easy_strerror(res));
        } else {
            LOG_HTTP(LOG_DEBUG, "`%s %s` (%d)", method.c_str(), reqUrl.c_str(), status);
        }

        return static_cast<int>(status);
    }

    void setHeaders(const std::map<std::string, std::string>& requestHeaders) {
        if (curl != nullptr) return;
        for (auto pair : requestHeaders) {
            std::string combined = pair.first + ": " + pair.second;
            headers = curl_slist_append(headers, combined.c_str());
        }
    }

    void setParameters(const std::list<URLQueryParameter>& requestParameters) {
        if (curl != nullptr) return;
        parameters = requestParameters;
    }

    void setMethod(const std::string& requestMethod) {
        method = requestMethod;
    }

    void setBody(const std::string& requestBody) {
        body = requestBody;
    }

    void setOnRedirect(const OnRedirectCallback& callback) {
        onRedirect = callback;
    }
    const std::string& content() const { return buffer; }

private:
    static size_t receiveData(void* ptr, size_t size, size_t nmemb, void* userdata) {
        CURLRequest* req = static_cast<CURLRequest*>(userdata);
        req->buffer.append(static_cast<const char*>(ptr), size * nmemb);
        return size * nmemb;
    }

    static size_t sendData(char *buffer, size_t size, size_t nitems, void* userdata) {
        CURLRequest* req = static_cast<CURLRequest*>(userdata);
        size_t remaining = req->body.length() - req->cursor;
        size_t toRead = std::min(size * nitems, remaining);
        if (toRead == 0) return 0;
        ::memcpy(buffer, req->body.c_str() + req->cursor, toRead);
        req->cursor += toRead;
        return toRead;
    }

    CURL* curl = nullptr;
    std::string method = "GET";
    std::string url;
    std::string body;
    std::string buffer;
    struct curl_slist* headers = nullptr;
    size_t cursor = 0;
    std::list<URLQueryParameter> parameters;
    OnRedirectCallback onRedirect;
};

bool Http::initializeCURLIfNeeded() {
    if (g_didInitCURL) return g_didInitCURL;

    curl_global_init(CURL_GLOBAL_ALL);

    g_didInitCURL = true;
    return true;
}

void Http::Shutdown() {
    if (g_didInitCURL) {
        curl_global_cleanup();
    }
}

HttpResponse Http::GET(const std::string& url, const HttpRequestOptions& options) {
    if (!initializeCURLIfNeeded()) return HttpResponse(-1);

    CURLRequest request(url);
    request.setHeaders(options.m_headers);
    request.setParameters(options.m_parameters);
    request.setOnRedirect(options.m_onRedirect);
    int status = request.send();
    return HttpResponse(status, request.content());
}

HttpResponse Http::PUT(const std::string& url, const std::string& body, const HttpRequestOptions& options) {
    if (!initializeCURLIfNeeded()) return HttpResponse(-1);

    CURLRequest request(url);
    request.setHeaders(options.m_headers);
    request.setParameters(options.m_parameters);
    request.setOnRedirect(options.m_onRedirect);
    request.setMethod("PUT");
    request.setBody(body);
    int status = request.send();
    return HttpResponse(status, request.content());
}

}  // namespace twitchsw
