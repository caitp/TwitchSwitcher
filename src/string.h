// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#include "refs.h"

#include <string>

namespace twitchsw {

enum class StaticString { Tag };
class StringImpl;
TSW_DECLARE_REF_CLASS(String, StringImpl);
class StringImpl : public RefCountedOrStatic<StringImpl> {
public:
    StringImpl(StaticString tag, size_t length, const char* data)
        : RefCountedOrStatic(1), m_length(length), m_buffer(data) {}
    ~StringImpl();

    bool startsWith(const std::string&) const;
    bool startsWith(const String&) const;
    bool startsWith(const Ref<String>&) const;
    bool startsWith(const char*) const;

    bool endsWith(const std::string&) const;
    bool endsWith(const String&) const;
    bool endsWith(const Ref<String>&) const;
    bool endsWith(const char*) const;

    bool equals(const std::string&) const;
    bool equals(const String&) const;
    bool equals(const Ref<String>&) const;
    bool equals(const char*) const;

    size_t length() const { return m_length; }
    const char* c_str() const { return m_buffer; }

    static StringImpl* allocate() { return refEmptyString(); }
    static StringImpl* allocate(const char* cstring) {
        if (!cstring || !*cstring) return refEmptyString();
        size_t length = ::strlen(cstring);
        return StringImpl::allocate(cstring, length);
    }

    static StringImpl* allocate(const char* cstring, size_t length) {
        if (!cstring || !length) return refEmptyString();
        size_t words = (sizeof(StringImpl) + length) / sizeof(long) + 1;
        StringImpl* data = reinterpret_cast<StringImpl*>(::calloc(words, sizeof(long)));
        data->m_length = length;
        memcpy(data->m_bytes, cstring, length);
        data->m_bytes[length] = '\0';
        data->m_buffer = data->m_bytes;
        data->ref();
        return data;
    }

    static StringImpl* allocate(const std::string& string) {
        if (string.empty()) refEmptyString();
        return StringImpl::allocate(string.c_str(), string.length());
    }

    static StringImpl* allocate(const String&);
    static StringImpl* allocate(const Ref<String>&);
    static StringImpl* allocate(String&&);
    static StringImpl* allocate(Ref<String>&&);

    static void deallocate(StringImpl*);

    static StringImpl* refEmptyString() {
        StringImpl* data = &g_emptyString;
        data->ref();
        return data;
    }

    class reverse_iterator;
    class iterator : public std::iterator<std::random_access_iterator_tag, const char> {
    public:
        iterator() : m_pos(nullptr) {}
        explicit iterator(const char* pos) : m_pos(pos) {}
        iterator& operator++() { m_pos++; return *this; }
        iterator operator++(int) { iterator ret(m_pos); m_pos++; return ret; }
        bool operator==(iterator other) const { return m_pos == other.m_pos; }
        bool operator!=(iterator other) const { return !(*this == other); }
        reference operator*() const { return *m_pos; }
        iterator operator-(difference_type diff) const { return iterator(m_pos - diff); }
        iterator& operator-=(difference_type diff) { m_pos -= diff; return *this; }
        iterator operator+(difference_type diff) const { return iterator(m_pos + diff); }
        iterator& operator+=(difference_type diff) { m_pos += diff; return *this; }
        operator pointer() const { return m_pos; }
    private:
        friend class reverse_iterator;
        const char* m_pos;
    };

    class reverse_iterator : public std::reverse_iterator<iterator> {
    public:
        reverse_iterator() : m_pos(nullptr) {}
        explicit reverse_iterator(const char* pos) : m_pos(pos) {}
        reverse_iterator& operator--() { m_pos++; return *this; }
        reverse_iterator operator--(int) { reverse_iterator ret(m_pos); m_pos++; return ret; }
        reverse_iterator& operator++() { m_pos--; return *this; }
        reverse_iterator operator++(int) { reverse_iterator ret(m_pos); m_pos--; return ret; }
        bool operator==(reverse_iterator other) const { return m_pos == other.m_pos; }
        bool operator!=(reverse_iterator other) const { return !(*this == other); }
        reference operator*() const { return *m_pos; }
        reverse_iterator operator+(difference_type diff) const { return reverse_iterator(m_pos - diff); }
        reverse_iterator& operator+=(difference_type diff) { m_pos -= diff; return *this; }
        reverse_iterator operator-(difference_type diff) const { return reverse_iterator(m_pos + diff); }
        reverse_iterator& operator-=(difference_type diff) { m_pos += diff; return *this; }
        operator pointer() const { return m_pos; }

    private:
        friend class iterator;
        const char* m_pos;
    };

    typedef iterator const_iterator;
    typedef reverse_iterator const_reverse_iterator;

    iterator begin() { return iterator(m_buffer); }
    iterator end() { return iterator(m_buffer + m_length); }
    reverse_iterator rbegin() { return reverse_iterator(m_buffer + m_length - 1); }
    reverse_iterator rend() { return reverse_iterator(m_buffer); }

    const_iterator cbegin() const { return iterator(m_buffer); }
    const_iterator cend() const { return iterator(m_buffer + m_length); }
    const_reverse_iterator crbegin() const { return reverse_iterator(m_buffer + m_length - 1); }
    const_reverse_iterator crend() const { return reverse_iterator(m_buffer); }

private:
    static StringImpl g_emptyString;

    size_t m_length;
    const char* m_buffer;
    char m_bytes[1];
};

}  // twitchsw
