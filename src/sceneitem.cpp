// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include "twitchsw.h"
#include "sceneitem.h"
#include "http.h"
#include "scenewatcher.h"

namespace twitchsw {

template <typename T>
struct Setting {
    const char* name;
    T defaultValue;
};

static Setting<const char*> g_stringSettings[] = {
    { "game", "" },
    { "title", "" },
};

static const char* doGetName(void* type_data) {
    return "TwitchSwitcher";
}

static void* doCreate(obs_data_t* settings, obs_source_t* source) {
    return static_cast<void*>(new TSWSceneItem(settings, source));
}

static void doDestroy(void* data) {
    delete static_cast<TSWSceneItem*>(data);
}

static void doGetDefaults(obs_data_t* settings) {
    for (int i = 0; i < sizeof(g_stringSettings) / sizeof(*g_stringSettings); ++i) {
        auto setting = g_stringSettings[i];
        obs_data_set_default_string(settings, setting.name, setting.defaultValue);
    }
}

static obs_properties_t* doGetProperties(void* data) {
    return static_cast<TSWSceneItem*>(data)->toProperties();
}

static void doUpdate(void* data, obs_data_t* settings) {
    return;
}

static void doSave(void* data, obs_data_t* settings) {
    return;
}

static void doLoad(void* data, obs_data_t* settings) {
    return;
}



static uint32_t doReturn0(void* data) { return 0; }

// Using C file to initialize obs_source_info, for the convenience of designated initializers.
static struct obs_source_info g_sceneItem = {
    /* ----------------------------------------------------------------- */
    /* Required implementation*/

    /* id                  */ "twitchsw",
    /* type                */ OBS_SOURCE_TYPE_INPUT,
    /* output_flags        */ 0,
    /* get_name            */ doGetName,
    /* create              */ doCreate,
    /* destroy             */ doDestroy,
    /* get_width           */ doReturn0,
    /* get_height          */ doReturn0,

    /* ----------------------------------------------------------------- */
    /* Optional implementation */

    /* get_defaults        */ doGetDefaults,
    /* get_properties      */ doGetProperties,
    /* update              */ doUpdate,
    /* activate            */ 0,
    /* deactivate          */ 0,
    /* show                */ 0,
    /* hide                */ 0,
    /* video_tick          */ 0,
    /* video_render        */ 0,
    /* filter_video        */ 0,
    /* filter_audio        */ 0,
    /* enum_active_sources */ 0,
    /* save                */ doSave,
    /* load                */ doLoad,
    /* mouse_click         */ 0,
    /* mouse_move          */ 0,
    /* mouse_wheel         */ 0,
    /* focus               */ 0,
    /* key_click           */ 0,
    /* filter_remove       */ 0,
    /* type_data           */ 0,
    /* free_type_data      */ 0,
    /* audio_render        */ 0,
};


void TwitchSwitcher::initializeSceneItem() {
    obs_register_source(&g_sceneItem);
}

std::mutex TSWSceneItem::g_globalMutex;
std::list<String> TSWSceneItem::g_games;

TSWSceneItem::TSWSceneItem(obs_data_t* settings, obs_source_t* source)
    : m_source(source) {
    m_game = String();
    m_title = String();
}

TSWSceneItem::~TSWSceneItem() {

}

/*
std::list<String> TSWSceneItem::getGamesList() const {
    Ref<String> key;
    std::list<String> games;
    std::unique_lock<std::mutex> lock(g_globalMutex);
    if (g_games.empty()) {
        if (!getTwitchCredentials(key))
            return games;

        Http http;
        auto response = http.
            setHeader("Authorization", "OAuth " + std::string(key->c_str(), key->length())).
            setHeader("Client-Id", TSW_CLIENT_ID).
            setHeader("Content-Type", "application/json").
            setHeader("Accept", "application/vnd.twitchtv.v3+json").
            setHeader("charsets", "utf-8").
            get("https://api.twitch.tv/kraken/games/top");

        return games;
    }
    return games;
}
*/

bool TSWSceneItem::getTwitchCredentials(Ref<String>& key) const {
    return SceneWatcher::getTwitchCredentials(key);
}

obs_properties_t* TSWSceneItem::toProperties() const {
    obs_properties_t* props = obs_properties_create();
    //obs_properties_add_text(props, "game", "Twitch Game Name", OBS_TEXT_DEFAULT);
    obs_property_t* games = obs_properties_add_list(props, "game", "Twitch Game Name", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(games, "value", m_game->c_str());
    return props;
}

}  // namespace twitchsw
