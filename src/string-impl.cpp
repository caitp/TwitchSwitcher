// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <twitchsw/string.h>
#include <twitchsw/never-destroyed.h>

namespace twitchsw {

StringImpl* StringImpl::empty()
{
    static NeverDestroyed<StringImpl> emptyString(ConstructEmptyString);
    return &emptyString.get();
}

// static
void StringImpl::destroy(StringImpl* stringImpl)
{
    stringImpl->~StringImpl();
    fastDealloc(stringImpl);
}

// static
inline Ref<StringImpl> StringImpl::createInternal(const char* characters, unsigned length)
{
    if (!characters || !length)
        return *empty();

    char* data;
    auto string = createUninitializedInternalNonEmpty(data, length);
    ::memcpy(data, characters, length * sizeof(char));
    return string;
}


// static
Ref<StringImpl> StringImpl::create(const char* characters, unsigned length)
{
    return createInternal(characters, length);
}

// static
Ref<StringImpl> StringImpl::createWithoutCopying(const char* characters, unsigned length)
{
    if (!length)
        return *empty();
    return adoptRef(*new StringImpl(characters, length, ConstructWithoutCopying));
}

// static
Ref<StringImpl> StringImpl::createUninitialized(char*& data, unsigned length)
{
    return createUninitializedInternal(data, length);
}

// static
Ref<StringImpl> StringImpl::createUninitializedInternal(char*& data, unsigned length)
{
    if (!length) {
        data = nullptr;
        return *empty();
    }
    return createUninitializedInternalNonEmpty(data, length);
}

// static
inline Ref<StringImpl> StringImpl::createUninitializedInternalNonEmpty(char*& data, unsigned length)
{
    // TODO(caitp): ASSERT(length);

    // Allocate a single buffer large enough to contain the StringImpl
    // struct as well as the data which it contains. This removes one
    // heap allocation from this call.

    // TODO(caitp): add webkit-like assertion to prevent allocation of too-large strings
    // if (length > ((std::numeric_limits<unsigned>::max() - sizeof(StringImpl)) / sizeof(char)))
    //     CRASH();
    StringImpl* string = static_cast<StringImpl*>(fastAllocZero(allocationSize<char>(length)));

    data = string->tailPointer<char>();
    return string->initializeFromUTF8(length);
}


bool StringImpl::startsWith(const char* characters, unsigned length) const
{
    if (m_length < length)
        return false;
    return ::strncmp(m_data, characters, length) == 0;
}

bool StringImpl::endsWith(const char* characters, unsigned length) const
{
    if (m_length < length)
        return false;
    return ::strncmp(m_data + (m_length - length), characters, length) == 0;
}

bool StringImpl::equals(const char* characters, unsigned length) const
{
    if (m_length != length)
        return false;
    return ::strncmp(m_data, characters, length) == 0;
}

}  // namespace twitchsw
