// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <thread>
#include <mutex>

#include <obs.hpp>
#include <obs-module.h>

#include "scenewatcher.h"
#include "workerthread.h"
#include "http.h"

using namespace twitchsw;

static bool hasStreamingOutput() {
    bool found = false;
    const char* outputType;
    for (int i = 0; obs_enum_output_types(i, &outputType); ++i) {
        std::string type(outputType);
        if (type == "rtmp_output") {
            found = true;
            break;
        }
    }
    return found;
}

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("twitchsw", "en-US")
OBS_MODULE_AUTHOR("Caitlin Potter")

MODULE_EXPORT const char* obs_module_name(void) {
    return "TwitchSwitcher";
}

WorkerThread g_worker;
SceneWatcher g_watcher;

MODULE_EXPORT bool obs_module_load(void) {
    if (!hasStreamingOutput()) {
        LOG(LOG_ERROR, "Streaming output type not found.");
        return false;
    }

    LOG(LOG_INFO, "Started up");
    TwitchSwitcher::initializeSceneItem();
    g_worker.start();
    g_watcher.start();
    return true;
}

MODULE_EXPORT void obs_module_unload(void) {
    g_watcher.terminate();
    g_worker.terminate();
}
