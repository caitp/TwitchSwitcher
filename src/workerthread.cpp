// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <twitchsw/scenewatcher.h>
#include <twitchsw/http.h>
#include <twitchsw/webview.h>

#include "workerthread-impl.h"

#include <obs.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

namespace twitchsw {

WorkerThreadImpl* WorkerThread::m_impl = nullptr;
WorkerThread::WorkerThread() {}
WorkerThread::~WorkerThread() { terminate(); }

void WorkerThread::start() {
    if (m_impl) return;
    m_impl = new WorkerThreadImpl;
    m_impl->start();
}

void WorkerThread::terminate() {
    if (!m_impl || !m_impl->m_thread || !m_impl->m_thread->joinable()) return;
    m_impl->postMessage(WorkerThread::kTerminate);
    m_impl->m_thread->join();
    delete m_impl;
    m_impl = nullptr;
}

void WorkerThread::update(Ref<UpdateEvent> event) {
    if (!m_impl || !m_impl->m_thread) return;
    m_impl->postMessage(WorkerThread::kUpdate, event.ptr());
}

//
//
//

WorkerThreadImpl::~WorkerThreadImpl() {
    if (m_thread != nullptr) {
        delete m_thread;
        m_thread = nullptr;
    }
}

void WorkerThreadImpl::start() {
    if (m_thread != nullptr) return;
    m_thread = new std::thread(runImpl, this);
}

void WorkerThreadImpl::runImpl(WorkerThreadImpl* impl) {
#if !defined(TSW_WIN32) || !TSW_WIN32
    // Name thread for debugging purposes. Non-win32 threads are assumed to use pthreads.
    // File a bug if this breaks things!
    pthread_setname_np("TSW.WorkerThread");
#endif
    impl->run();
}

void WorkerThreadImpl::run() {
    while (true) {
        MessageData event;
        if (waitForMessage(event)) {
            if (!handleMessage(event))
                break;
        }
    }
}

bool WorkerThreadImpl::handleMessage(MessageData& event) {
    switch (event.message) {
    case WorkerThread::kTerminate:
        cleanup();
        return false;

    case WorkerThread::kUpdate:
        if (!update(adoptRef(*event.param).cast<UpdateEvent>()))
            return false;
        break;
    }
    return true;
}

class SimpleException : public std::exception {
public:
    SimpleException(const std::string& reason) : std::exception(), m_reason(reason) {}
    virtual ~SimpleException() {}

    const std::string& reason() const { return m_reason; }

private:
    std::string m_reason;
};

bool WorkerThreadImpl::updateInternal(const std::string& accessToken, String game, String title) {
    Http http;
    http.
        setHeader("Authorization", "OAuth " + accessToken).
        setHeader("Client-Id", TSW_CLIENT_ID).
        setHeader("content-type", "application/json").
        setHeader("Accept", "application/vnd.twitchtv.v3+json").
        setHeader("charsets", "utf-8");

    // Step 1: load the channel information that is already present, so that we know which channel to update
    auto response = http.
        request().
        //setParameter("client_id", TSW_CLIENT_ID).
        //setParameter("oauth_token", accessToken).
        get("https://api.twitch.tv/kraken/channel");

    if (response.status() != 200)
        return true;

    std::string channel;

    {
        using namespace rapidjson;
        Document doc;
        doc.Parse(response.content());
        auto displayName = doc.FindMember("display_name");
        if (displayName == doc.MemberEnd()) {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Unexpected JSON response from /channel endpoint. Please file a bug at https://github.com/caitp/TwitchSwitcher");
            return true;
        }

        channel = displayName->value.GetString();
    }

    // FIXME: Use obs localization API
    LOG(LOG_INFO, "Updating stream to game '%s' with title '%s'", game.characters(), title.characters());
    std::string body;
    {
        // Create JSON object to send to Twitch.
        // https://github.com/justintv/Twitch-API/blob/master/v3_resources/channels.md#put-channelschannel
        using namespace rapidjson;
        CrtAllocator allocator;
        StringBuffer buffer(&allocator, game.length() + title.length() + 256);
        Writer<StringBuffer> writer(buffer, &allocator);
        writer.StartObject();
        writer.Key("channel", 7);
        writer.StartObject();
        if (game.length()) {
            writer.Key("game", 4);
            writer.String(game.characters(), game.length());
        }

        if (title.length()) {
            writer.Key("status", 6);
            writer.String(title.characters(), title.length());
        }
        writer.EndObject();
        writer.EndObject();
        body = buffer.GetString();
    }
    response = http.
        request().
        //setParameter("oauth_token", accessToken).
        //setParameter("client_id", TSW_CLIENT_ID).
        put("https://api.twitch.tv/kraken/channels/" + channel, body);

    if (response.status() != 200) {
        // May be JSON info describing the failure.
        using namespace rapidjson;
        Document doc;
        ParseResult parseResult = doc.Parse(response.content());
        std::string result;
        if (parseResult) {
            auto error = doc.FindMember("error");
            auto message = doc.FindMember("message");
            if (error != doc.MemberEnd() && error->value.IsString())
                result += error->value.GetString();
            if (message != doc.MemberEnd() && message->value.IsString()) {
                if (result.length())
                    result += ": ";
                result += message->value.GetString();
            }
        }
        if (result.empty())
            result = response.content();
        // FIXME: Use obs localization API
        LOG(LOG_WARNING, "[Twitch API] '%s'. Please file a bug at https://github.com/caitp/TwitchSwitcher", result.c_str());
    }
    return true;
}

std::future<AuthStatus> WorkerThreadImpl::authenticateIfNeeded() {
    auto result = std::make_shared <std::promise<AuthStatus>>();
    std::future<AuthStatus> future = result->get_future();
    if (m_accessToken.length()) {
        result->set_value({ HttpResponse(200, std::string()), m_accessToken });
        return future;
    }

    String key;
    if (!SceneWatcher::getTwitchCredentials(key)) {
        result->set_exception(std::make_exception_ptr(SimpleException("Could not retrieve Stream Key.")));
        return future;
    }

    Http http;
    http.
        setHeader("Authorization", "OAuth " + key.toStdString()).
        setHeader("Client-Id", TSW_CLIENT_ID).
        setHeader("Content-Type", "application/json").
        setHeader("Accept", "application/vnd.twitchtv.v3+json").
        setHeader("charsets", "utf-8");

    // Get the channel for the authenticated user. I don't do anything with this other than get your channel name.
    std::string channel;
    HttpResponse response;

    std::string code;
    std::string authUrl;
    HttpRequestOptions authRequest = http.
        request().
        setParameter("client_id", TSW_CLIENT_ID).
        setParameter("response_type", "token").
        setParameter("redirect_uri", "http://localhost").
        setParameter("scope", TSW_PERMISSIONS_SCOPE).
        setOnRedirect([&](const std::string& url, const std::string& document) {
        authUrl = url;
        return OnRedirect::Fail;
    });
    response = authRequest.
        get("https://api.twitch.tv/kraken/oauth2/authorize");

    if (!authUrl.c_str()) {
        // FIXME: Use obs localization API
        result->set_exception(std::make_exception_ptr(SimpleException("Did not get redirect URI from oauth2/authorize endpoint: '%s'.")));
        return future;
    }

    if (!m_currentWebView.isNull()) {
        m_currentWebView->close();
        m_currentWebView = nullptr;
    }

    Ref<WebView> webView = *adoptRef(new WebView());
    authRequest.setOnRedirect([this](const std::string& url, const std::string& body) {
        // Should gain access to authorization code here, if the URL looks a certain way...
        static const std::string redirectUri = "http://localhost";
        if (std::equal(redirectUri.begin(), redirectUri.end(), url.begin())) {
            auto begin = url.find("access_token=", redirectUri.length() + 1);
            if (begin != std::string::npos) {
                auto end = url.find('&', begin);
                if (end == std::string::npos)
                    this->m_accessToken = url.substr(begin + 13);
                else
                    this->m_accessToken = url.substr(begin + 13, end - (begin + 13));
            }
            return OnRedirect::Finish;
        }
        return OnRedirect::Follow;
    });

    struct RequestState : public RefCounted<RequestState> {
        bool gotAuthToken = false;
    };

    RefPtr<RequestState> requestState = new RequestState;
    webView->setOnComplete([this, result, requestState](WebView& webView, String url) {
        if (this->m_accessToken.length()) {
            LOG(LOG_INFO, "gotAuthToken: %s\n", this->m_accessToken.c_str());
            requestState->gotAuthToken = true;
            webView.close();
        }
        if (this->m_accessToken.length())
            result->set_value({ HttpResponse(200, std::string()), m_accessToken });
        else
            result->set_exception(std::make_exception_ptr(SimpleException("Did not get authorization token")));
    }).
        setOnAbort([result, requestState](WebView& webView, String url) {
        // Prevent hangs when a response is not going to happen.
        if (!requestState->gotAuthToken)
            result->set_exception(std::make_exception_ptr(SimpleException("Request aborted")));
    }).
        setTitle("Please sign in"). // FIXME: Use obs localization API
        open(authUrl, authRequest).show();
    m_currentWebView = webView;
    return future;
}

bool WorkerThreadImpl::update(Ref<UpdateEvent> data) {
    String stream = data->stream();
    String game = data->game();
    String title = data->title();
    LOG(LOG_DEBUG, "Updating stream '%s'\n      Game = '%s'\n    Status = '%s'", stream.characters(), game.characters(), title.characters());
    auto accessTokenFuture = authenticateIfNeeded();
    while (true) {
        // Nested message loop, special casing the Update message.
        MessageData event;
        if (waitForMessage(event, std::chrono::milliseconds(300))) {
            if (event.message == WorkerThread::kUpdate) {
                data = adoptRef(*event.param).cast<UpdateEvent>();
                // When sign-in is complete, will use the event data from the
                // most recent event.
                game = data->game();
                title = data->title();
            } else {
                if (!handleMessage(event))
                    return false;
            }
        }
        if (accessTokenFuture.wait_for(std::chrono::milliseconds(300)) == std::future_status::ready)
            break;
    }

    std::string accessToken;

    try {
        AuthStatus status = accessTokenFuture.get();
        accessToken = status.accessToken;
    } catch (const SimpleException& e) {
        // FIXME: Use obs localization API
        LOG(LOG_WARNING, "Authorization failed: %s. Please file a bug at https://github.com/caitp/TwitchSwitcher", e.reason().c_str());
        return true;
    } catch (const std::future_error& e) {
        if (e.code() == std::future_errc::broken_promise) {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Aborted getting access token.");
        }
        return true;
    }

    return updateInternal(accessToken, game, title);
}

void WorkerThreadImpl::cleanup() {
    if (!m_currentWebView.isNull())
        m_currentWebView->close();
}

}  // namespace twitchsw
