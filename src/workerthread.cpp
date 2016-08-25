// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include "workerthread-impl.h"
#include <obs.h>

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

void WorkerThreadImpl::update(UpdateEvent&& data) {
    Ref<UpdateEvent> event = data;
    LOG(LOG_INFO, "update!");
}

void WorkerThreadImpl::cleanup() {
    LOG(LOG_INFO, "terminated.");
}

//
//
//
UpdateEventImpl::UpdateEventImpl(obs_output_t* output, String&& game, String&& title)
    : EventDataImpl(), m_output(output), m_game(std::move(game)), m_title(std::move(title)) {
    obs_output_addref(output);
}

UpdateEventImpl::~UpdateEventImpl() {
    obs_output_release(m_output);
}


}  // namespace twitchsw
