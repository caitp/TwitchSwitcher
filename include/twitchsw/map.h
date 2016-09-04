// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <map>
#include <unordered_map>
#include <utility>

namespace twitchsw {

// Helper which approximates C++17's std::map::insert_or_assign
template <class Map, class Value>
std::pair<typename Map::iterator, bool> insertOrAssign(Map& map, const typename Map::key_type& key, Value&& value) {
#if (defined(_MSC_VER) && _MSC_VER >= 1900) || (defined(__clang__) && __cplusplus >= 201406L) || (defined(__GNUC__) && __cplusplus >= 201700L)
    // Use libc++ implementation if it's available, as it's probably better
    return map.insert_or_assign(key, value);
#else
    typedef typename Map::iterator iterator;
    iterator p = map.find(key);
    if (p != map.end()) {
        p->second = std::forward<Value>(value);
        return std::make_pair(p, false);
    }
    return std::make_pair(map.emplace_hint(p, key, std::forward<Value>(value)), true);
#endif
}

}  // namespace twitchsw
