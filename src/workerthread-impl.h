// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <twitchsw/workerthread.h>
#include <twitchsw/webview.h>

#include <list>
#include <mutex>
#include <thread>
#include <future>

namespace twitchsw {

struct MessageData {
    MessageData()
        : message(WorkerThread::kNoMessage)
        , param(nullptr)
    {
    }

    MessageData(WorkerThread::Message messageID, PassRefPtr<EventData> data = nullptr)
        : message(messageID)
        , param(data)
    {
    }
    WorkerThread::Message message;
    RefPtr<EventData> param = nullptr;
};

struct AuthStatus {
    HttpResponse response;
    std::string accessToken;
};

class WorkerThreadImpl {
public:
    ~WorkerThreadImpl();

    void start();

    void postMessage(WorkerThread::Message message, PassRefPtr<EventData> data = nullptr) {
        if (m_thread == nullptr) return;

        bool wasEmpty;
        {
            std::lock_guard<std::mutex> lock(m_messageListMutex);
            wasEmpty = m_messageList.empty();
            MessageData event(message, data);
            if (message <= WorkerThread::kLastPriviledgedMessage)
                m_messageList.push_front(event);
            else
                m_messageList.push_back(event);
        }
        if (wasEmpty)
            m_didReceiveMessage.notify_one();
    }

private:
    friend class WorkerThread;
    std::thread* m_thread = nullptr;
    std::condition_variable m_didReceiveMessage;
    std::mutex m_messageListMutex;
    std::list<MessageData> m_messageList;
    std::string m_accessToken;
    WeakPtr<WebView> m_currentWebView;

    void run();

    static void runImpl(WorkerThreadImpl* worker);

    bool waitForMessage(MessageData& data) {
        std::unique_lock<std::mutex> lock(m_messageListMutex);
        while (m_messageList.empty())
            m_didReceiveMessage.wait(lock);
        MessageData copy = m_messageList.front();
        data = { copy.message, copy.param.leakRef() };
        m_messageList.pop_front();
        return true;
    }

    // Blocks for the allotted time, returns true if a message has become available.
    template <typename Rep, typename Period>
    bool waitForMessageAvailable(const std::chrono::duration<Rep, Period>& timeout_duration) {
        std::unique_lock<std::mutex> lock(m_messageListMutex);
        if (!m_messageList.empty() &&
            m_didReceiveMessage.wait_for(lock, timeout_duration) == std::cv_status::no_timeout)
            return true;
        return false;
    }

    // Blocks for the allotted time, and reutrns message if found
    template <typename Rep, typename Period>
    bool waitForMessage(MessageData& data, const std::chrono::duration<Rep, Period>& timeout_duration) {
        std::unique_lock<std::mutex> lock(m_messageListMutex);
        if (!m_messageList.empty() ||
            m_didReceiveMessage.wait_for(lock, timeout_duration) == std::cv_status::no_timeout) {
            // FIXME(caitp): For some reason MSVC reaches this point with an empty messageList. Am I abusing the message list CV?
            if (!m_messageList.empty()) {
                MessageData copy = m_messageList.front();
                data = { copy.message, copy.param.leakRef() };
                m_messageList.pop_front();
                return true;
            }
        }
        return false;
    }

    // If returned false, WorkerThreadImpl is dead and you should exit.
    bool handleMessage(MessageData& data);

    std::future<AuthStatus> authenticateIfNeeded();
    bool update(Ref<UpdateEvent> data);
    bool updateInternal(const std::string& accessToken, String game, String title);
    void cleanup();
};

}  // namespace twitchsw
