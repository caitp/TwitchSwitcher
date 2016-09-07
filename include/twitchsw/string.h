// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <twitchsw/refs.h>

#include "string-impl.h"

#include <string>
#include <cstdlib>

namespace twitchsw {

class String {
public:
    String() { }
    String(const char* characters);
    String(const char* characters, unsigned length);
    ALWAYS_INLINE ~String() { }

    String(StringImpl& impl)
        : m_impl(&impl)
    {
    }

    String(StringImpl* impl)
        : m_impl(impl)
    {
    }

    String(Ref<StringImpl>&& impl)
        : m_impl(std::move(impl))
    {
    }

    String(RefPtr<StringImpl>&& impl)
        : m_impl(std::move(impl))
    {
    }

    enum ConstructFromLiteralTag { ConstructFromLiteral };
    template <unsigned N>
    String(const char (&characters)[N], ConstructFromLiteralTag)
        : m_impl(StringImpl::createFromLiteral<N>(characters))
    {
    }

    String(const String& other)
        : m_impl(other.m_impl)
    {
    }

    String(String&& other)
        : m_impl(std::move(other.m_impl))
    {
    }

    String& operator=(const String& other) {
        m_impl = other.m_impl;
        return *this;
    }
    String& operator=(String&& other) {
        m_impl = std::move(other.m_impl);
        return* this;
    }

    void swap(String& o) { m_impl.swap(o.m_impl); }

    bool isNull() const { return !m_impl; }
    bool isEmpty() const { return !m_impl || !m_impl->length(); }
    unsigned refCount() const { return m_impl ? m_impl->refCount() : 0; }

    StringImpl* impl() const { return m_impl.get(); }
    RefPtr<StringImpl> releaseImpl() { return std::move(m_impl); }

    std::string toStdString() const {
        if (LIKELY(m_impl))
            return std::string(m_impl->characters(), m_impl->length());
        return std::string();
    }

    unsigned length() const
    {
        if (!m_impl)
            return 0;
        return m_impl->length();
    }

    const char* characters() const
    {
        if (!m_impl)
            return nullptr;
        return m_impl->characters();
    }

    const char* c_str() const { return characters(); }

    char at(unsigned index) const
    {
        if (!m_impl || index >= m_impl->length())
            return 0;
        return (*m_impl)[index];
    }

    char operator[](unsigned index) const { return at(index); }

    // startsWith
    bool startsWith(const std::string& o) const
    {
        if (LIKELY(m_impl))
            return m_impl->startsWith(o.c_str(), o.length());
        return o.length() == 0;
    }

    bool startsWith(const String& o) const
    {
        unsigned len = o.length();
        if (LIKELY(m_impl && len))
            return m_impl->startsWith(o.characters(), len);
        return len == 0;
    }
    bool startsWith(const char* o) const
    {
        if (UNLIKELY(!o || !*o))
            return true;
        return startsWith(o, static_cast<unsigned>(::strlen(o)));
    }

    bool startsWith(const char* o, unsigned len) const
    {
        // TODO(caitp): ASSERT(o != nullptr)
        if (LIKELY(m_impl))
            return m_impl->startsWith(o, len);
        return len == 0;
    }

    // endsWith
    bool endsWith(const std::string& o) const
    {
        if (LIKELY(m_impl))
            return m_impl->endsWith(o.c_str(), o.length());
        return o.length() == 0;
    }

    bool endsWith(const String& o) const
    {
        unsigned len = o.length();
        if (LIKELY(m_impl && len))
            return m_impl->endsWith(o.characters(), len);
        return len == 0;
    }
    bool endsWith(const char* o) const
    {
        if (UNLIKELY(!o || !*o))
            return true;
        return endsWith(o, static_cast<unsigned>(::strlen(o)));
    }

    bool endsWith(const char* o, unsigned len) const
    {
        // TODO(caitp): ASSERT(o != nullptr)
        if (LIKELY(m_impl))
            return m_impl->endsWith(o, len);
        return len == 0;
    }

    // equals
    bool equals(const std::string& o) const
    {
        if (LIKELY(m_impl))
            return m_impl->equals(o.c_str(), o.length());
        return o.length() == 0;
    }

    bool equals(const String& o) const
    {
        unsigned len = o.length();
        if (LIKELY(m_impl && len))
            return m_impl->equals(o.characters(), len);
        return len == 0;
    }
    bool equals(const char* o) const
    {
        if (UNLIKELY(!o || !*o))
            return length() == 0;
        return equals(o, static_cast<unsigned>(::strlen(o)));
    }

    bool equals(const char* o, unsigned len) const
    {
        // TODO(caitp): ASSERT(o != nullptr)
        if (LIKELY(m_impl))
            return m_impl->equals(o, len);
        return len == 0;
    }

private:
    RefPtr<StringImpl> m_impl;
};

}  // twitchsw
