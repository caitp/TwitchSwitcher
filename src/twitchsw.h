// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <util/base.h>
#include "refs.h"

#ifdef LOG
#undef LOG
#endif
#define LOG(type, ...) blog((type), "[TwitchSwitcher] " __VA_ARGS__)
#define LOG_HTTP(type, ...) LOG((type), "[HTTP] " __VA_ARGS__)

#define TSW_CLIENT_ID "ne4a7nx7ne8yntm226fqlp59lilwxeh"

struct obs_source;
typedef struct obs_source obs_source_t;
namespace twitchsw {

class TwitchSwitcher {
public:
    static void initializeSceneItem();
    static void addSettingsIfNeeded(obs_source_t* source);
};

}
