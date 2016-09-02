// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <thread>
#include <mutex>

#include <obs.hpp>
#include <obs-module.h>

#include <twitchsw/scenewatcher.h>
#include <twitchsw/workerthread.h>
#include <twitchsw/http.h>
#include <twitchsw/webview.h>

#ifdef _DEBUG
#include <rapidjson/reader.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#endif

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
    WebView::initialize();
    g_worker.start();
    g_watcher.start();
    return true;
}

MODULE_EXPORT void obs_module_unload(void) {
    WebView::shutdown();
    g_watcher.terminate();
    g_worker.terminate();
}

//
//
//
void TwitchSwitcher::prettyPrintJSON(const std::string& string) {
    return prettyPrintJSON(string.c_str(), string.length());
}
void TwitchSwitcher::prettyPrintJSON(Ref<String> string) {
    return prettyPrintJSON(string->c_str(), string->length());
}
void TwitchSwitcher::prettyPrintJSON(const char* str, size_t length) {
#ifdef _DEBUG
    using namespace rapidjson;
    Reader reader;
    CrtAllocator allocator;
    StringStream is(str);
    StringBuffer buffer(&allocator, length + 256);
    PrettyWriter<StringBuffer> prettyWriter(buffer);
    prettyWriter.SetIndent(' ', 4);
    reader.Parse(is, prettyWriter);
    buffer.GetString();

    LOG(LOG_DEBUG, "%s", buffer.GetString());
#endif
}
