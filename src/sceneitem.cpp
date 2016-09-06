// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <ctime>

#include <twitchsw/twitchsw.h>
#include <twitchsw/sceneitem.h>
#include <twitchsw/http.h>
#include <twitchsw/scenewatcher.h>
#include <rapidjson/document.h>

#ifdef _DEBUG

#include <sstream>

#include <rapidjson/reader.h>
#include "rapidjson/prettywriter.h"
#endif

namespace twitchsw {

#define arraysize(x) (sizeof(x) / sizeof(*x))
#define TSW_SCENEITEM_ID "twitchsw"

enum SettingID {
    kGameSetting,
    kTitleSetting
};

const TSWSceneItem::Setting<const char*> TSWSceneItem::g_stringSettings[] = {
    { offsetof(TSWSceneItem, m_game), "game", "" },
    { offsetof(TSWSceneItem, m_title), "title", "" },
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
    for (int i = 0; i < arraysize(TSWSceneItem::g_stringSettings); ++i) {
        auto setting = TSWSceneItem::g_stringSettings[i];

        obs_data_set_default_string(settings, setting.name, setting.defaultValue);
    }
}

static obs_properties_t* doGetProperties(void* data) {
    return static_cast<TSWSceneItem*>(data)->toProperties();
}

static void doUpdate(void* data, obs_data_t* settings) {
    static_cast<TSWSceneItem*>(data)->didUpdateProperties(settings);
}

static void doSave(void* data, obs_data_t* settings) {
    return;
}

static void doLoad(void* data, obs_data_t* settings) {
    static_cast<TSWSceneItem*>(data)->didLoadProperties(settings);
}



static uint32_t doReturn0(void* data) { return 0; }

// Using C file to initialize obs_source_info, for the convenience of designated initializers.
static struct obs_source_info g_sceneItem = {
    /* ----------------------------------------------------------------- */
    /* Required implementation*/

    /* id                  */ TSW_SCENEITEM_ID,
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

static std::list<TSWSceneItem*> g_sceneItems;
TSWSceneItem::TSWSceneItem(obs_data_t* settings, obs_source_t* source)
    : m_source(source) {
    m_game = String();
    m_title = String();
    g_sceneItems.push_back(this);
    connectSignalHandlers();
    LOG(LOG_INFO, "TSWSceneItem holding source %p", source);
}

TSWSceneItem::~TSWSceneItem() {
    for (auto it = g_sceneItems.begin(); it != g_sceneItems.end(); ++it) {
        if (*it == this) {
            g_sceneItems.erase(it);
            break;
        }
    }
    disconnectSignalHandlers();
}

void TSWSceneItem::connectSignalHandlers() {
    signal_handler_t* signals = obs_source_get_signal_handler(m_source);
    if (!signals) {
        // FIXME: Use obs localization API
        LOG(LOG_WARNING, "Unable to connect signal handlers for SceneItem '%s'", obs_source_get_name(m_source));
        return;
    }
}

void TSWSceneItem::disconnectSignalHandlers() {
    signal_handler_t* signals = obs_source_get_signal_handler(m_source);
    if (!signals) {
        // FIXME: Use obs localization API
        LOG(LOG_WARNING, "Unable to disconnect signal handlers for SceneItem '%s'", obs_source_get_name(m_source));
        return;
    }
}

void TSWSceneItem::didUpdateProperties(obs_data_t* settings) {
    for (int i = 0; i < arraysize(g_stringSettings); ++i) {
        auto setting = g_stringSettings[i];
        String* result = reinterpret_cast<String*>((reinterpret_cast<char*>(this) + setting.offset));
        String newValue = String(obs_data_get_string(settings, setting.name));
        if (!newValue.equals(*result)) {
            *result = newValue;
            if (i == kGameSetting) {
                updateGameTitleTypeahead(settings);
            }
        }
    }
}

void TSWSceneItem::didLoadProperties(obs_data_t* settings) {
    for (int i = 0; i < arraysize(g_stringSettings); ++i) {
        auto setting = g_stringSettings[i];
        String* result = reinterpret_cast<String*>((reinterpret_cast<char*>(this) + setting.offset));
        *result = String(obs_data_get_string(settings, setting.name));
    }
}

void TSWSceneItem::updateGameTitleTypeahead(obs_data_t* settings) {
    // TODO(caitp): Currently, the API does not support generating typeahead-style UIs.
    // Bug has been filed at https://obsproject.com/mantis/view.php?id=601, to revisit
    // at a later date.
    return;
#pragma region TODO
    // TODO(caitp): do this more efficiently. This will also block the entire
    // settings dialog, which is bad.
    static std::time_t lastTypeaheadTime = 0;
    std::time_t now = std::time(nullptr);

    if ((now - lastTypeaheadTime) < 5)
        return;

    lastTypeaheadTime = now;

    String key;
    if (!getTwitchCredentials(key))
        return;

    Http http;

    auto response = http.
        request().
        setHeader("Authorization", "OAuth " + key.toStdString()).
        setHeader("Client-Id", TSW_CLIENT_ID).
        setHeader("Content-Type", "application/json").
        setHeader("Accept", "application/vnd.twitchtv.v3+json").
        setHeader("charsets", "utf-8").
        setParameter("q", game().toStdString()).
        setParameter("type", "suggest").
        get("https://api.twitch.tv/kraken/search/games");

    if (response.status() != 200) {
        // FIXME: Use obs localization API
        LOG(LOG_WARNING, "Bad HTTP response. Possibly rate-limited by Twitch API. Consider filing a bug at https://github.com/caitp/TwitchSwitcher");
        return;
    }
    std::string content = response.content();

    std::list<std::string> options;
    {
        using namespace rapidjson;

        // Print prettified JSON string (Debug only)
        TwitchSwitcher::prettyPrintJSON(content);

        Document doc;
        ParseResult ok = doc.Parse(content);
        if (!ok) {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Unexpected data from api.twitch.tv/kraken/search/games. Please file a bug at https://github.com/caitp/TwitchSwitcher");
            return;
        }

        auto it = doc.FindMember("games");
        bool expectedFormat = it != doc.MemberEnd();
        if (expectedFormat) {
            auto& games = it->value;
            expectedFormat = games.IsArray();
            if (expectedFormat) {
                for (auto it = games.Begin(); it != games.End(); ++it) {
                    auto& game = *it;
                    if (!game.IsObject()) {
                        expectedFormat = false;
                        continue;
                    }
                    auto name = game.FindMember("name");
                    if (name == game.MemberEnd()) {
                        expectedFormat = false;
                        continue;
                    }
                    if (!name->value.IsString())
                        expectedFormat = false;
                    else
                        options.push_back(name->value.GetString());
                }
            }
        }
        if (!expectedFormat) {
            // FIXME: Use obs localization API
            LOG(LOG_WARNING, "Unexpected JSON schema from api.twitch.tv/kraken/search/games. Please file a bug at https://github.com/caitp/TwitchSwitcher");
        }
    }
#pragma endregion TODO
}

bool TSWSceneItem::getTwitchCredentials(String& key) const {
    return SceneWatcher::getTwitchCredentials(key);
}

obs_properties_t* TSWSceneItem::toProperties() {
    obs_properties_t* props = obs_properties_create();
    obs_properties_add_list(props, "game", "Twitch Game Name", OBS_COMBO_TYPE_EDITABLE, OBS_COMBO_FORMAT_STRING);
    obs_properties_add_text(props, "title", "Twitch Channel Name", OBS_TEXT_DEFAULT);

    // TODO(caitp): When typeahead support is possible, didUpdateProperties will need an update to avoid
    // starting the typeahead on save.
    obs_properties_set_flags(props, OBS_PROPERTIES_DEFER_UPDATE);
    return props;
}

// static
TSWSceneItem* TSWSceneItem::fromSource(obs_source_t* source) {
    if (source && std::string(obs_source_get_id(source)) == TSW_SCENEITEM_ID) {
        // TODO(caitp): DO NOT use non-public API to do this. Worried the struct packing or ordering could change
        // in a non-binary-compatible way.
        struct OBSSourceHeader { // obs_context_data, assuming
            char* name;
            void* data;
            obs_data_t* settings;
            signal_handler_t* signals;
            proc_handler_t* procs;
            enum obs_obj_type type;
        };
        OBSSourceHeader* header = reinterpret_cast<OBSSourceHeader*>(source);

        if (header->signals == obs_source_get_signal_handler(source) && header->name == obs_source_get_name(source)) {
            return static_cast<TSWSceneItem*>(header->data);
        } else {
            static bool didLogBinaryCompatError = false;
            if (!didLogBinaryCompatError) {
                // FIXME: Use obs localization API
                LOG(LOG_WARNING, "libobs binary compat has changed. Please file a bug at https://github.com/caitp/TwitchSwitcher.");
                didLogBinaryCompatError = true;
            }
            for (auto item : g_sceneItems) {
                if (item->m_source == source)
                    return item;
            }
        }
    }
    return nullptr;
}

// static
TSWSceneItem* TSWSceneItem::fromSceneItem(obs_sceneitem_t* item) {
    if (item == nullptr) return nullptr;
    obs_source_t* source = obs_sceneitem_get_source(item);
    return fromSource(source);
}

}  // namespace twitchsw
