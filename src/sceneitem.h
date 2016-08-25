// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <list>
#include <mutex>

#include <obs.h>
#include <obs-source.h>

#include "string.h"

namespace twitchsw {

class TSWSceneItem {
public:
    explicit TSWSceneItem(obs_data_t* settings, obs_source_t* source);
    ~TSWSceneItem();

    obs_properties_t* toProperties() const;

    bool getTwitchCredentials(Ref<String>& key) const;

private:
    obs_source_t* m_source;
    Ref<String> m_game;
    Ref<String> m_title;

    static std::mutex g_globalMutex;
    static std::list<String> g_games;
};

}  // namespace twitchsw
