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
    WorkerThread::Message message;
    Ref<EventData> param;
};

struct AuthStatus {
    HttpResponse response;
    std::string accessToken;
};

class WorkerThreadImpl {
public:
    ~WorkerThreadImpl();

    void start();

    void postMessage(WorkerThread::Message message, EventData&& data = EventData(RefEmptyConstructor::Tag)) {
        if (m_thread == nullptr) return;

        bool wasEmpty;
        {
            std::lock_guard<std::mutex> lock(m_messageListMutex);
            wasEmpty = m_messageList.empty();
            data.refIfNeeded();
            if (message <= WorkerThread::kLastPriviledgedMessage)
                m_messageList.push_front({ message, data });
            else
                m_messageList.push_back({ message, data });
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
    WeakWebView m_currentWebView;

    void run();

    static void runImpl(WorkerThreadImpl* worker);

    bool waitForMessage(MessageData& data) {
        std::unique_lock<std::mutex> lock(m_messageListMutex);
        while (m_messageList.empty())
            m_didReceiveMessage.wait(lock);
        data = m_messageList.front();
        data.param.refIfNeeded();
        m_messageList.pop_front();
        return true;
    }

    // Blocks for the allotted time, returns true if a message has become available.
    template <typename Rep, typename Period>
    bool waitForMessageAvailable(const std::chrono::duration<Rep,Period>& timeout_duration) {
        std::unique_lock<std::mutex> lock(m_messageListMutex);
        if (!m_messageList.empty() &&
            m_didReceiveMessage.wait_for(lock, timeout_duration) == std::cv_status::no_timeout)
            return true;
        return false;
    }

    // Blocks for the allotted time, and reutrns message if found
    template <typename Rep, typename Period>
    bool waitForMessage(MessageData& data, const std::chrono::duration<Rep,Period>& timeout_duration) {
        std::unique_lock<std::mutex> lock(m_messageListMutex);
        if (!m_messageList.empty() ||
            m_didReceiveMessage.wait_for(lock, timeout_duration) == std::cv_status::no_timeout) {
            data = m_messageList.front();
            data.param.refIfNeeded();
            m_messageList.pop_front();
            return true;
        }
        return false;
    }

    // If returned false, WorkerThreadImpl is dead and you should exit.
    bool handleMessage(MessageData& data);

    std::future<AuthStatus> authenticateIfNeeded();
    bool update(UpdateEvent&& data);
    bool updateInternal(const std::string& accessToken, const Ref<String> game, const Ref<String> title);
    void cleanup();
};

}  // namespace twitchsw
