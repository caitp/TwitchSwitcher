// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include "workerthread.h"
#include "scenewatcher-impl.h"
#include "obs-source.h"

extern twitchsw::SceneWatcher g_watcher;

namespace twitchsw {

SceneWatcher::SceneWatcher() {

}

SceneWatcher::~SceneWatcher() {
    terminate();
}

void SceneWatcher::start() {
    if (m_impl != nullptr) return;
    m_impl = new SceneWatcherImpl();
}

void SceneWatcher::terminate() {
    if (m_impl == nullptr) return;
    delete m_impl;
    m_impl = nullptr;
}

bool SceneWatcher::getTwitchCredentials(Ref<String>& key) {
    if (g_watcher.m_impl == nullptr) return false;
    return g_watcher.m_impl->getTwitchCredentials(key);
}

//
//
//

SceneWatcherImpl::SceneWatcherImpl() {
    connectSignalHandlers();
}

SceneWatcherImpl::~SceneWatcherImpl() {
    // TODO(caitp): land a patch in obs-studio to make this call safe during obs_shutdown().
    // Currently, the main obs signal handlers are destroyed before freeing modules, and
    // the signal handler list is not set to null.
    // https://obsproject.com/mantis/view.php?id=595
    //
    // disconnectSignalHandlers();

    if (m_streamingOutput != nullptr) {
        obs_weak_output_release(m_streamingOutput);
        m_streamingOutput = nullptr;
    }
}

// static
bool SceneWatcherImpl::isTwitchStream(obs_service_t* service) {
    const std::string twitchUrlPrefix("rtmp://live-");
    const std::string twitchUrlSuffix(".twitch.tv/app");

    std::string url = obs_service_get_url(service);
    // Only use service if it is a  Twitch stream
    return std::equal(twitchUrlPrefix.begin(), twitchUrlPrefix.end(), url.begin()) &&
        std::equal(twitchUrlSuffix.rbegin(), twitchUrlSuffix.rend(), url.rbegin());
}

// static
bool SceneWatcherImpl::isTwitchStream(obs_output_t* output) {

    obs_service_t* service = obs_output_get_service(output);
    if (service == nullptr)
        return false;

    return isTwitchStream(service);
}

// static
bool SceneWatcherImpl::scanForStreamingOutputsProc(void* param, obs_output_t* output) {
    const char* name = obs_output_get_name(output);
    SceneWatcherImpl* impl = static_cast<SceneWatcherImpl*>(param);
    if (impl->m_streamingOutput != nullptr)
        return false;

    if (!isTwitchStream(output) && std::string(name) != "simple_stream")
        return true;

    impl->m_streamingOutput = obs_output_get_weak_output(output);
    return false;
}

void SceneWatcherImpl::scanForStreamingOutputs() {
    obs_enum_outputs(scanForStreamingOutputsProc, this);
}

bool SceneWatcherImpl::scanForStreamingServicesProc(void* param, obs_service_t* service) {
    SceneWatcherImpl* impl = static_cast<SceneWatcherImpl*>(param);

    if (impl->m_streamingService != nullptr)
        return false;

    if (!isTwitchStream(service))
        return true;

    impl->m_streamingService = obs_service_get_weak_service(service);
    return false;
}

void SceneWatcherImpl::scanForStreamingServices() {
    obs_enum_services(scanForStreamingServicesProc, this);
}


bool SceneWatcherImpl::getTwitchCredentials(Ref<String>& key) {
    obs_service_t* service = obs_weak_service_get_service(m_streamingService);
    if (service == nullptr) {
        obs_weak_service_release(m_streamingService);
        m_streamingService = nullptr;
        scanForStreamingServices();
        if (m_streamingService == nullptr)
            return false;
        service = obs_weak_service_get_service(m_streamingService);
    }
    if (service == nullptr)
        return false;

    key = String(obs_service_get_key(service));
    return key->length() > 0;
}

void SceneWatcherImpl::connectSignalHandlers() {
    signal_handler_t* signals = obs_get_signal_handler();
    if (!signals) return;
    signal_handler_connect(signals, "source_create", addSourceIfNeeded, this);
    signal_handler_connect(signals, "source_load", addSourceIfNeeded, this);
    signal_handler_connect(signals, "source_remove", removeSourceIfNeeded, this);
    signal_handler_connect(signals, "source_destroy", removeSourceIfNeeded, this);
}

void SceneWatcherImpl::disconnectSignalHandlers() {
    signal_handler_t* signals = obs_get_signal_handler();
    if (!signals) return;
    signal_handler_disconnect(signals, "source_create", addSourceIfNeeded, this);
    signal_handler_disconnect(signals, "source_load", addSourceIfNeeded, this);
    signal_handler_disconnect(signals, "source_remove", removeSourceIfNeeded, this);
    signal_handler_disconnect(signals, "source_destroy", removeSourceIfNeeded, this);
}

// Called in response to source_create and source_load. If source is a Scene, adds to scene management
// collection.
void SceneWatcherImpl::addSourceIfNeeded(void* userdata, calldata_t* calldata) {
    obs_source_t* source;
    SceneWatcherImpl* self = static_cast<SceneWatcherImpl*>(userdata);
    if (!calldata_get_ptr(calldata, "source", &source)) return;

    if (obs_source_get_type(source) != OBS_SOURCE_TYPE_SCENE)
        return;

    self->addScene(source);
}

// Called in response to source_remove and source_destroy. If source is a Scene, removes scene from scene
// mangement collection.
void SceneWatcherImpl::removeSourceIfNeeded(void* userdata, calldata_t* calldata) {
    obs_source_t* source;
    SceneWatcherImpl* self = static_cast<SceneWatcherImpl*>(userdata);
    if (!calldata_get_ptr(calldata, "source", &source)) return;

    if (obs_source_get_type(source) != OBS_SOURCE_TYPE_SCENE)
        return;

    self->removeScene(source);
}


void SceneWatcherImpl::addScene(obs_source_t* scene) {
    if (findScene(scene).isNull()) {
        Scene newScene(this, scene);
        newScene.ref();
        m_scenes.push_back(newScene);
    }
}

void SceneWatcherImpl::removeScene(obs_source_t* scene) {
    for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        Ref<Scene> foundScene = *it;
        if (foundScene->source() == scene) {
            m_scenes.erase(it);
            return;
        }
    }
}

bool SceneWatcherImpl::isStreaming() {
    if (m_streamingOutput == nullptr)
        scanForStreamingOutputs();

    if (m_streamingOutput == nullptr)
        return false;

    obs_output_t* output = obs_weak_output_get_output(m_streamingOutput);
    if (output == nullptr) {
        obs_weak_output_release(m_streamingOutput);
        m_streamingOutput = nullptr;
        return false;
    }

    if (!obs_output_active(output))
        return false;

    if (!isTwitchStream(output)) {
        obs_weak_output_release(m_streamingOutput);
        m_streamingOutput = nullptr;
        return false;
    }
    return true;
}

Ref<Scene> SceneWatcherImpl::findScene(obs_source_t* source) {
    for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        Ref<Scene> scene = *it;
        if (scene->source() == source)
            return scene;
    }
    return Ref<Scene>();
}

//
//
//

SceneImpl::SceneImpl(SceneWatcherImpl* impl, obs_source_t* scene)
    : RefCounted(), m_impl(impl), m_source(scene) {
    connectSignalHandlers();
    if (!m_item)
        m_item = takeFirstTwitchSceneItem(m_source);
}

SceneImpl::~SceneImpl() {
    disconnectSignalHandlers();

    m_source = nullptr;
    m_item = nullptr;
}

void SceneImpl::connectSignalHandlers() {
    signal_handler_t* signals = obs_source_get_signal_handler(source());
    if (!signals) return;

    signal_handler_connect(signals, "item_add", onAddSceneItem, this);
    signal_handler_connect(signals, "item_remove", onRemoveSceneItem, this);
    signal_handler_connect(signals, "show", onShow, this);
    signal_handler_connect(signals, "activate", onActivate, this);
}

void SceneImpl::disconnectSignalHandlers() {
    signal_handler_t* signals = obs_source_get_signal_handler(m_source);
    if (!signals) return;

    signal_handler_disconnect(signals, "item_add", onAddSceneItem, this);
    signal_handler_disconnect(signals, "item_remove", onRemoveSceneItem, this);
    signal_handler_disconnect(signals, "show", onShow, this);
    signal_handler_disconnect(signals, "activate", onActivate, this);
}

void SceneImpl::onAddSceneItem(void* userdata, calldata_t* calldata) {
    SceneImpl* scene = static_cast<SceneImpl*>(userdata);
    if (scene->m_item != nullptr) return;

    obs_sceneitem_t* item;
    if (calldata_get_ptr(calldata, "item", &item) && isTwitchSceneItem(item))
        scene->m_item = item;
}

void SceneImpl::onRemoveSceneItem(void* userdata, calldata_t* calldata) {
    SceneImpl* scene = static_cast<SceneImpl*>(userdata);
    if (scene->m_item == nullptr) return;

    obs_sceneitem_t* item;
    if (calldata_get_ptr(calldata, "item", &item)) {
        if (item == scene->m_item) {
            obs_sceneitem_release(item);
            scene->m_item = scene->takeFirstTwitchSceneItem(scene->m_source);
        }
    }
}

void SceneImpl::onShow(void* userdata, calldata_t* calldata) {
    static_cast<SceneImpl*>(userdata)->updateIfNeeded();
}

void SceneImpl::onActivate(void* userdata, calldata_t* calldata) {
}

void SceneImpl::updateIfNeeded() {
    LOG(LOG_DEBUG, "Updating stream '%s'", obs_source_get_name(m_source));
    WorkerThread::update(UpdateEvent(nullptr, "meow", "hello"));
}

//
//
//
bool SceneImpl::isTwitchSceneItem(obs_sceneitem_t* item) {
    return false;
}


obs_sceneitem_t* SceneImpl::takeFirstTwitchSceneItem(obs_source_t* scene) {
    obs_sceneitem_t* item = nullptr;

    return item;
}

}
