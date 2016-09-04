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

void WorkerThread::update(UpdateEvent& ref) {
    if (!m_impl || !m_impl->m_thread) return;
    m_impl->postMessage(WorkerThread::kUpdate, ref.cast<EventData>());
}

void WorkerThread::update(UpdateEvent&& ref) {
    if (!m_impl || !m_impl->m_thread) return;
    m_impl->postMessage(WorkerThread::kUpdate, ref.cast<EventData>());
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

void WorkerThreadImpl::run() {
    while (true) {
        MessageData event;
        if (waitForMessage(event)) {
            switch (event.message) {
            case WorkerThread::kTerminate:
                cleanup();
                return;

            case WorkerThread::kUpdate:
                update(event.param.cast<UpdateEvent>());
                break;
            }
        }
    }
}

class SimpleException : public std::exception {
public:
    SimpleException(const std::string& reason) : std::exception(), m_reason(reason) {}
    virtual ~SimpleException() {}

    const std::string& reason() const { return m_reason; }

private:
    std::string m_reason;
};

void WorkerThreadImpl::updateInternal(const std::string& accessToken, const Ref<String> game, const Ref<String> title) {
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
        return;

    std::string channel;

    {
        using namespace rapidjson;
        Document doc;
        doc.Parse(response.content());
        auto displayName = doc.FindMember("display_name");
        if (displayName == doc.MemberEnd()) {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Unexpected JSON response from /channel endpoint. Please file a bug at https://github.com/caitp/TwitchSwitcher");
            return;
        }

        channel = displayName->value.GetString();
    }

    // FIXME: Use obs localization API
    LOG(LOG_INFO, "Updating stream to game '%s' with title '%s'", game->c_str(), title->c_str());
    std::string body;
    {
        // Create JSON object to send to Twitch.
        // https://github.com/justintv/Twitch-API/blob/master/v3_resources/channels.md#put-channelschannel
        using namespace rapidjson;
        CrtAllocator allocator;
        StringBuffer buffer(&allocator, game->length() + title->length() + 256);
        Writer<StringBuffer> writer(buffer, &allocator);
        writer.StartObject();
        writer.Key("channel", 7);
        writer.StartObject();
        if (game->length()) {
            writer.Key("game", 4);
            writer.String(game->c_str(), game->length());
        }

        if (title->length()) {
            writer.Key("status", 6);
            writer.String(title->c_str(), title->length());
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
    return;
}

std::future<AuthStatus> WorkerThreadImpl::authenticateIfNeeded() {
    auto result = std::make_shared <std::promise<AuthStatus>>();
    std::future<AuthStatus> future = result->get_future();
    if (m_accessToken.length()) {
        result->set_value({ HttpResponse(200, std::string()), m_accessToken });
        return future;
    }

    Ref<String> key;
    if (!SceneWatcher::getTwitchCredentials(key)) {
        result->set_exception(std::make_exception_ptr(SimpleException("Could not retrieve Stream Key.")));
        return future;
    }

    Http http;
    http.
        setHeader("Authorization", "OAuth " + std::string(key->c_str(), key->length())).
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



    WebView webView;
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
    webView.setOnComplete([this, result](WebView& webView) {
        if (this->m_accessToken.length()) {
            webView.close();
            if (this->m_accessToken.length())
                result->set_value({ HttpResponse(200, std::string()), m_accessToken });
            else
                result->set_exception(std::make_exception_ptr(SimpleException("Did not get authorization token.")));
        }
    }).
        setTitle("Please sign in"). // FIXME: Use obs localization API
        open(authUrl, authRequest).show();

    return future;
}

void WorkerThreadImpl::update(UpdateEvent&& data) {
    Ref<UpdateEvent> event = data;

    Ref<String> game = data->game();
    Ref<String> title = data->title();

    auto accessTokenFuture = authenticateIfNeeded();
    accessTokenFuture.wait();
    std::string accessToken;

    try {
        AuthStatus status = accessTokenFuture.get();
        accessToken = status.accessToken;
    } catch (const SimpleException& e) {
        // FIXME: Use obs localization API
        LOG(LOG_WARNING, "Authorization failed: %s. Please file a bug at https://github.com/caitp/TwitchSwitcher", e.reason().c_str());
        return;
    } catch (const std::future_error& e) {
        if (e.code() == std::future_errc::broken_promise) {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Aborted getting access token.");
        }
        return;
    }

    return updateInternal(accessToken, game, title);
}

void WorkerThreadImpl::cleanup() {
}

//
//
//
UpdateEventImpl::UpdateEventImpl(String&& game, String&& title)
    : EventDataImpl(), m_game(std::move(game)), m_title(std::move(title)) {
}

UpdateEventImpl::~UpdateEventImpl() {
}


}  // namespace twitchsw
