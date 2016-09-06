// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include <twitchsw/twitchsw.h>
#include <twitchsw/refs.h>

namespace twitchsw {

template <typename T> class NeverDestroyed;

class StringImpl {
    static const int kStaticStringFlag = 1;
    static const int kRefCountIncrement = 2;
public:
    StringImpl(const StringImpl&) = delete;
    StringImpl(StringImpl&&) = delete;
    StringImpl& operator=(const StringImpl&) = delete;
    StringImpl& operator=(StringImpl&&) = delete;

    static Ref<StringImpl> create(const char* characters, unsigned length);
    static Ref<StringImpl> createWithoutCopying(const char* characters, unsigned length);
    static Ref<StringImpl> createUninitialized(char*& data, unsigned length);

    static void destroy(StringImpl*);

private:
    static Ref<StringImpl> createUninitializedInternal(char*& data, unsigned length);
    static Ref<StringImpl> createUninitializedInternalNonEmpty(char*& data, unsigned length);
    static Ref<StringImpl> createInternal(const char* characters, unsigned length);

    Ref<StringImpl> initializeFromUTF8(unsigned length)
    {
        m_refCount = kRefCountIncrement;
        m_length = length;
        m_data = tailPointer<char>();
        return adoptRef(*this);
    }

    enum ConstructWithoutCopyingTag { ConstructWithoutCopying };
    StringImpl(const char* characters, unsigned length, ConstructWithoutCopyingTag)
        : m_refCount(kRefCountIncrement)
        , m_length(length)
        , m_data(characters)
        , m_unused(0)
    {
    }

    // Used to construct static strings, which have an special refCount that can never hit zero.
    // This means that the static string will never be destroyed, which is important because
    // static strings will be shared across threads & ref-counted in a non-threadsafe manner.
    friend class NeverDestroyed<StringImpl>;
    enum ConstructEmptyStringTag { ConstructEmptyString };
    StringImpl(ConstructEmptyStringTag)
        : m_refCount(kStaticStringFlag)
        , m_length(0)
        , m_data(reinterpret_cast<const char*>(&m_length))
        , m_unused(0)
    {
    }

public:

    template <unsigned charactersCount>
    ALWAYS_INLINE static Ref<StringImpl> createFromLiteral(const char (&characters)[charactersCount])
    {
        static_assert(charactersCount > 1, "StringImpl::createFromLiteral must not be called on an empty string");
        static_assert((characters - 1 <= ((unsigned(~0) - sizeof(StringImpl) / sizeof(char)))), "StringImpl::createFromLiteral cannot construct a string this large.");

        return createWithoutCopying(reinterpret_cast<const char*>(characters), charactersCount - 1);
    }

    unsigned length() const { return m_length; }
    const char* characters() const { return m_data; }
    bool startsWith(const char* characters, unsigned length) const;
    bool endsWith(const char* characters, unsigned length) const;
    bool equals(const char* characters, unsigned length) const;

    char at(unsigned i) const
    {
        return m_data[i];
    }
    char operator[](unsigned i) const { return at(i); }

    inline bool isStatic() const { return m_refCount & kStaticStringFlag; }

    inline size_t refCount() const
    {
        return m_refCount >> 1;
    }

    inline bool hasOneRef() const
    {
        return m_refCount == kRefCountIncrement;
    }

    inline bool hasAtLeastOneRef() const
    {
        return !!m_refCount;
    }

    inline void ref()
    {
        m_refCount += kRefCountIncrement;
    }

    inline void deref()
    {
        unsigned tempRefCount = m_refCount - kRefCountIncrement;
        if (!tempRefCount)
            return StringImpl::destroy(this);
        m_refCount = tempRefCount;
    }

    static StringImpl* empty();

private:
    template<typename T>
    static size_t allocationSize(unsigned tailElementCount)
    {
        return tailOffset<T>() + tailElementCount * sizeof(T);
    }

    template<typename T>
    static ptrdiff_t tailOffset()
    {
#if TSW_COMPILER(MSVC)
        // MSVC doesn't support alignof yet.
        return roundUpToMultipleOf<sizeof(T)>(sizeof(StringImpl));
#else
        return roundUpToMultipleOf<alignof(T)>(offsetof(StringImpl, m_unused) + sizeof(StringImpl::m_unused));
#endif
    }

    template<typename T>
    const T* tailPointer() const
    {
        return reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(this) + tailOffset<T>());
    }

    template<typename T>
    T* tailPointer()
    {
        return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(this) + tailOffset<T>());
    }

private:
    unsigned m_refCount;
    unsigned m_length;
    const char* m_data;
    mutable unsigned m_unused;
};

}  // namespace twitchsw
