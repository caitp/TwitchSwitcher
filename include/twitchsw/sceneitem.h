// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <list>
#include <mutex>

#include <obs.h>
#include <obs-source.h>

#include <twitchsw/string.h>

namespace twitchsw {

class TSWSceneItem {
public:
    explicit TSWSceneItem(obs_data_t* settings, obs_source_t* source);
    ~TSWSceneItem();

    obs_properties_t* toProperties();

    bool getTwitchCredentials(Ref<String>& key) const;

    void connectSignalHandlers();
    void disconnectSignalHandlers();

    void didUpdateProperties(obs_data_t* settings);
    void didLoadProperties(obs_data_t* settings);
    void updateGameTitleTypeahead(obs_data_t* settings);

    Ref<String> game() const { return m_game; }
    Ref<String> title() const { return m_title; }

    template <typename T>
    struct Setting {
        size_t offset;
        const char* name;
        T defaultValue;
    };
    static const Setting<const char*> g_stringSettings[];

    static TSWSceneItem* fromSource(obs_source_t* source);
    static TSWSceneItem* fromSceneItem(obs_sceneitem_t* item);
private:
    obs_source_t* m_source;
    Ref<String> m_game;
    Ref<String> m_title;
};

}  // namespace twitchsw
