// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <string>

#include <util/base.h>

#include <twitchsw/string.h>

#ifdef LOG
#undef LOG
#endif
#define LOG(type, ...) blog((type), "[TwitchSwitcher] " __VA_ARGS__)
#define LOG_HTTP(type, ...) LOG((type), "[HTTP] " __VA_ARGS__)

#define TSW_CLIENT_ID "ne4a7nx7ne8yntm226fqlp59lilwxeh"
#define TSW_PERMISSIONS_SCOPE "channel_read channel_editor"

struct obs_source;
typedef struct obs_source obs_source_t;
namespace twitchsw {

class TwitchSwitcher {
public:
    static void initializeSceneItem();
    static void addSettingsIfNeeded(obs_source_t* source);

    static void prettyPrintJSON(const std::string& string);
    static void prettyPrintJSON(Ref<String> string);
    static void prettyPrintJSON(const char* str, size_t length);
};

}  // namespace twitchsw
