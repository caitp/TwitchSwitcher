// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <twitchsw/string.h>

namespace twitchsw {

String::String(const char* characters)
{
    if (!characters)
        return;

    m_impl = StringImpl::create(characters, static_cast<unsigned>(::strlen(characters)));
}

String::String(const char* characters, unsigned length)
{
    if (characters)
        m_impl = StringImpl::create(characters, length);
}

}  // namespace twitchsw
