// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <string>

#include <util/base.h>

#include <twitchsw/string.h>

#include <array>

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

template <typename T>
T&& useVariableImpl(T&& variable) { return variable; }

#define TSW_UNUSED(X) twitchsw::useVariableImpl((X))

template <typename T>
constexpr auto makeArrayN(std::integral_constant<std::size_t, 0>, T&&) {
    return std::array<std::decay_t<T>, 0> {};
}
template <std::size_t size, typename T, std::size_t... indexes>
constexpr auto makeArrayN(T&& value, std::index_sequence<indexes...>) {
    return std::array<std::decay_t<T>, size>{ (static_cast<void>(indexes), value)..., std::forward<T>(value) };
}

template <size_t size, typename T>
static constexpr auto makeArrayN(std::integral_constant<std::size_t, size>, T&& value) {
    return makeArrayN<size>(std::forward<T>(value), std::make_index_sequence<size - 1>{});
}

template <std::size_t size, typename T>
constexpr auto makeArrayN(T&& value) {
    return makeArrayN(std::integral_constant<std::size_t, size>{}, std::forward<T>(value));
}


class TwitchSwitcher {
public:
    enum Setting {
        kUpdateWithoutStreaming,

        kNumSettings
    };

    static bool isEnabled(Setting setting);
    static inline bool isDisabled(Setting setting) {
        return !isEnabled(setting);
    }

    static void initializeSceneItem();
    static void addSettingsIfNeeded(obs_source_t* source);

    static void prettyPrintJSON(const std::string& string);
    static void prettyPrintJSON(Ref<String> string);
    static void prettyPrintJSON(const char* str, size_t length);
};

}  // namespace twitchsw
