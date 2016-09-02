// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include <twitchsw/string.h>

namespace twitchsw {

void StringImpl::deallocate(StringImpl* ptr) {
    if (ptr->m_buffer != ptr->m_bytes)
        return;
    ::free(ptr);
}

StringImpl::~StringImpl() {
}

bool StringImpl::startsWith(const std::string& str) const {
    if (length() < str.length()) return false;
    return std::equal(str.cbegin(), str.cend(), cbegin());
}

bool StringImpl::startsWith(const String& str) const {
    if (length() < str->length()) return false;
    return std::equal(str->cbegin(), str->cend(), cbegin());
}
bool StringImpl::startsWith(const Ref<String>& str) const {
    if (length() < str->length()) return false;
    return std::equal(str->cbegin(), str->cend(), cbegin());
}
bool StringImpl::startsWith(const char* str) const {
    return startsWith(std::string(str));
}

bool StringImpl::endsWith(const std::string& str) const {
    if (length() < str.length()) return false;
    return std::equal(str.crbegin(), str.crend(), crbegin());
}
bool StringImpl::endsWith(const String& str) const {
    if (length() < str->length()) return false;
    return std::equal(str->crbegin(), str->crend(), crbegin());
}
bool StringImpl::endsWith(const Ref<String>& str) const {
    if (length() < str->length()) return false;
    return std::equal(str->crbegin(), str->crend(), crbegin());
}
bool StringImpl::endsWith(const char* str) const {
    return endsWith(std::string(str));
}

bool StringImpl::equals(const std::string& str) const {
    if (length() != str.length()) return false;
    return std::equal(str.cbegin(), str.cend(), cbegin());
}
bool StringImpl::equals(const String& str) const {
    if (length() != str->length()) return false;
    return std::equal(str->cbegin(), str->cend(), cbegin());
}
bool StringImpl::equals(const Ref<String>& str) const {
    if (length() != str->length()) return false;
    return std::equal(str->cbegin(), str->cend(), cbegin());
}
bool StringImpl::equals(const char* str) const {
    return equals(std::string(str));
}

StringImpl StringImpl::g_emptyString(StaticString::Tag, 0, "");

StringImpl* StringImpl::allocate(const String& other) {
    const StringImpl* const_ptr = other.toPtr();
    StringImpl* ptr = const_cast<StringImpl*>(const_ptr);
    ptr->ref();
    return ptr;
}

StringImpl* StringImpl::allocate(const Ref<String>& other) {
    const StringImpl* const_ptr = other.toPtr();
    StringImpl* ptr = const_cast<StringImpl*>(const_ptr);
    ptr->ref();
    return ptr;
}

StringImpl* StringImpl::allocate(String&& other) {
    other.ref();
    return other.toPtr();
}

StringImpl* StringImpl::allocate(Ref<String>&& other) {
    other.ref();
    return other.toPtr();
}

}  // namespace twitchsw
