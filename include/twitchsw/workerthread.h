// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <string>

#include <twitchsw/twitchsw.h>
#include <twitchsw/string.h>
#include <twitchsw/refs.h>

struct obs_output;

namespace twitchsw {

class EventData : public ThreadSafeRefCounted<EventData> {
public:
    EventData() : ThreadSafeRefCounted() {}
    virtual ~EventData() {}
};

class UpdateEvent : public EventData {
public:
    UpdateEvent(String&& scene, String&& game, String&& title)
        : m_scene(std::move(scene))
        , m_game(std::move(game))
        , m_title(std::move(title))
    {
    }

    UpdateEvent(const String& scene, const String& game, const String& title)
        : m_scene(scene)
        , m_game(game)
        , m_title(title)
    {
    }

    ~UpdateEvent() override
    {
    }

    String stream() const { return m_scene; }
    String game() const { return m_game; }
    String title() const { return m_title; }

private:
    String m_scene;
    String m_game;
    String m_title;
};

class WorkerThreadImpl;
class WorkerThread {
public:
    enum Message {
        kNoMessage = 0,

        // Priviledged messages are pushed to the front of the message queue for eager consumption
        kTerminate,
        kLastPriviledgedMessage = kTerminate,

        // Non-priviledged messages pushed to the back and processed in order.
        kUpdate
    };

    WorkerThread();
    ~WorkerThread();

    void start();
    void terminate();

    // Post an "update" message to the worker thread, if the thread is started.
    static void update(Ref<UpdateEvent> event);

private:
    static WorkerThreadImpl* m_impl;
};

}  // namespace twitchsw
