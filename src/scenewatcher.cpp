// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <twitchsw/workerthread.h>
#include <twitchsw/sceneitem.h>

#include "scenewatcher-impl.h"

#include <obs-source.h>

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

bool SceneWatcher::getTwitchCredentials(String& key) {
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

    OBSService service = obs_output_get_service(output);
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

    signal_handler_t* signals = obs_output_get_signal_handler(output);
    signal_handler_connect(signals, "start", onStartStreaming, impl);

    impl->m_streamingOutput = OBSGetWeakRef(output);
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

    impl->m_streamingService = OBSGetWeakRef(service);
    return false;
}

void SceneWatcherImpl::scanForStreamingServices() {
    obs_enum_services(scanForStreamingServicesProc, this);
}


bool SceneWatcherImpl::getTwitchCredentials(String& key) {
    if (m_streamingService == nullptr) {
        scanForStreamingServices();
        if (m_streamingService == nullptr)
            return false;
    }

    OBSService service = OBSGetStrongRef(m_streamingService);
    if (service == nullptr) {
        m_streamingService = nullptr;
        scanForStreamingServices();
        if (m_streamingService == nullptr)
            return false;
        service = OBSGetStrongRef(m_streamingService);
        if (service == nullptr)
            return false;
    }

    key = String(obs_service_get_key(service));
    return key.length() > 0;
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


// Called in response to `start` of obs_output_t. We use this to update the twitch status and game on stream start,
// if needed
void SceneWatcherImpl::onStartStreaming(void* userdata, calldata_t* calldata) {
    SceneWatcherImpl* impl = static_cast<SceneWatcherImpl*>(userdata);
    obs_output_t* output;
    if (!calldata_get_ptr(calldata, "output", &output))
        return;

    do {
        if (impl->m_streamingOutput == nullptr)
            break;

        OBSOutput currentOutput = OBSGetStrongRef(impl->m_streamingOutput);
        if (currentOutput != output) {
            impl->m_streamingOutput = nullptr;
            break;
        }

        if (impl->m_currentScene)
            impl->m_currentScene->updateIfNeeded(true);
        return;
    } while (0);

    signal_handler_t* signals = obs_output_get_signal_handler(output);
    signal_handler_disconnect(signals, "start", onStartStreaming, impl);
}


void SceneWatcherImpl::addScene(obs_source_t* scene) {
    if (findScene(scene).isNull()) {
        LOG(LOG_INFO, "addScene(%p)", scene);
        m_scenes.push_back(adoptRef(new Scene(this, scene)));
    }
}

void SceneWatcherImpl::removeScene(obs_source_t* scene) {
    for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        RefPtr<Scene> foundScene = *it;
        if (foundScene->source() == scene) {
            m_scenes.erase(it);
            if (foundScene == m_currentScene)
                m_currentScene = nullptr;
            return;
        }
    }
}

bool SceneWatcherImpl::isStreaming() {
    if (m_streamingOutput == nullptr)
        scanForStreamingOutputs();

    if (m_streamingOutput == nullptr)
        return false;

    OBSOutput output = OBSGetStrongRef(m_streamingOutput);
    if (output == nullptr) {
        m_streamingOutput = nullptr;
        return false;
    }

    if (!obs_output_active(output))
        return false;

    if (!isTwitchStream(output)) {
        m_streamingOutput = nullptr;
        return false;
    }

    return true;
}

RefPtr<Scene> SceneWatcherImpl::findScene(obs_source_t* source) {
    for (auto it = m_scenes.begin(); it != m_scenes.end(); ++it) {
        RefPtr<Scene> scene = *it;
        if (scene->source() == source)
            return scene;
    }
    return RefPtr<Scene>();
}

//
//
//

Scene::Scene(SceneWatcherImpl* impl, obs_source_t* scene)
    : RefCounted(), m_impl(impl), m_source(scene), m_item(nullptr) {
    connectSignalHandlers();
    if (!m_item)
        m_item = takeFirstTwitchSceneItem(m_source);
}

Scene::~Scene() {
    disconnectSignalHandlers();

    m_source = nullptr;
    m_item = nullptr;
}

void Scene::connectSignalHandlers() {
    signal_handler_t* signals = obs_source_get_signal_handler(source());
    if (!signals) return;

    signal_handler_connect(signals, "item_add", onAddSceneItem, this);
    signal_handler_connect(signals, "item_remove", onRemoveSceneItem, this);
    signal_handler_connect(signals, "show", onShow, this);
    signal_handler_connect(signals, "activate", onActivate, this);
}

void Scene::disconnectSignalHandlers() {
    signal_handler_t* signals = obs_source_get_signal_handler(m_source);
    if (!signals) return;

    signal_handler_disconnect(signals, "item_add", onAddSceneItem, this);
    signal_handler_disconnect(signals, "item_remove", onRemoveSceneItem, this);
    signal_handler_disconnect(signals, "show", onShow, this);
    signal_handler_disconnect(signals, "activate", onActivate, this);
}

void Scene::onAddSceneItem(void* userdata, calldata_t* calldata) {
    RefPtr<Scene> scene = static_cast<Scene*>(userdata);
    if (scene->m_item != nullptr) return;

    obs_sceneitem_t* item;
    if (calldata_get_ptr(calldata, "item", &item) && isTwitchSceneItem(item)) {
        scene->m_item = item;
    }
}

void Scene::onRemoveSceneItem(void* userdata, calldata_t* calldata) {
    RefPtr<Scene> scene = static_cast<Scene*>(userdata);
    if (scene->m_item == nullptr) return;

    obs_sceneitem_t* item;
    if (calldata_get_ptr(calldata, "item", &item)) {
        if (item == scene->m_item) {
            scene->m_item = scene->takeFirstTwitchSceneItem(scene->m_source, item);
        }
    }
}

void Scene::onShow(void* userdata, calldata_t* calldata) {
    RefPtr<Scene> scene = static_cast<Scene*>(userdata);
    SceneWatcherImpl* impl = scene->m_impl;
    impl->setCurrentScene(scene);
    scene->updateIfNeeded();
}

void Scene::onActivate(void* userdata, calldata_t* calldata) {
}

void Scene::updateIfNeeded(bool force) {
    TSWSceneItem* item = TSWSceneItem::fromSceneItem(m_item);
    if (item == nullptr) return;

    if (!force && TwitchSwitcher::isDisabled(TwitchSwitcher::kUpdateWithoutStreaming) && !this->m_impl->isStreaming())
        return;

    String scene = obs_source_get_name(m_source);
    String game = item->game();
    String title = item->title();
    // FIXME: Use obs localization API
    WorkerThread::update(*new UpdateEvent(scene, game, title));
}

//
//
//
bool Scene::isTwitchSceneItem(obs_sceneitem_t* item) {
    if (!item)
        return false;

    obs_source_t* source = obs_sceneitem_get_source(item);
    return TSWSceneItem::fromSource(source) != nullptr;
}

struct TakeFirstTwitchSceneItemData {
    obs_sceneitem_t* ignore;
    obs_sceneitem_t* result;
};

bool Scene::takeFirstTwitchSceneItemProc(obs_scene_t* scene, obs_sceneitem_t* item, void* param) {
    TakeFirstTwitchSceneItemData* data = reinterpret_cast<TakeFirstTwitchSceneItemData*>(param);
    if (item == data->ignore || !isTwitchSceneItem(item))
        return true;

    data->result = item;
    return false;
}

obs_sceneitem_t* Scene::takeFirstTwitchSceneItem(obs_source_t* scene, obs_sceneitem_t* ignore) {
    TakeFirstTwitchSceneItemData data { ignore, nullptr };
    obs_scene_enum_items(obs_scene_from_source(scene), takeFirstTwitchSceneItemProc, &data);
    return data.result;
}

}  // namespace twitchsw
