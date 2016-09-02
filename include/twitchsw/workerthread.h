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

class EventDataImpl : public RefCounted<EventDataImpl> {
public:
    EventDataImpl() : RefCounted() {}
    virtual ~EventDataImpl() {}
};

TSW_DECLARE_REF_CLASS(EventData, EventDataImpl);

class UpdateEventImpl : public EventDataImpl {
public:
    UpdateEventImpl(String&& game, String&& title);
    ~UpdateEventImpl() override;

    TSW_BASIC_ALLOCATOR(UpdateEventImpl);

    Ref<String> game() const { return m_game; }
    Ref<String> title() const { return m_title; }

private:
    Ref<String> m_game;
    Ref<String> m_title;
};
TSW_DECLARE_REF_CLASS(UpdateEvent, UpdateEventImpl);

class WorkerThreadImpl;
class WorkerThread {
public:
    enum Message {
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
    static void update(UpdateEvent& event);

private:
    static WorkerThreadImpl* m_impl;
};

}  // namespace twitchsw
