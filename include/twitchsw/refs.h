// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

namespace twitchsw {

template <typename T> class Ref;

enum class RefEmptyConstructor { Tag };

template <typename T>
class RefCountedBase {
public:
    typedef T RefCountedType;
    template <typename... TArgs>
    RefCountedBase(TArgs... args) {
        m_data = reinterpret_cast<T*>(T::allocate(args...));
    }
    template <>
    RefCountedBase(RefEmptyConstructor tag)
        : m_data(nullptr) {}

    RefCountedBase(const Ref<RefCountedBase<T>>& ref);

    ~RefCountedBase() {
        if (m_data != nullptr && !m_data->deref()) {
            T::deallocate(m_data);
            m_data = nullptr;
        }
    }

    static RefCountedBase<T> fromPtr(T*);
    inline bool isNull() const { return m_data == nullptr; }
    inline operator bool() const { return !isNull(); }
    inline void ref() { m_data->ref(); }
    inline void refIfNeeded() { if (m_data != nullptr) m_data->ref(); }
    inline bool deref() { return m_data->deref(); }
    inline operator T*() { return m_data; }
    inline operator T*() const { return m_data; }
    inline T* operator->() { return m_data; }
    inline const T* operator->() const { return m_data; }
    inline T* toPtr() { return m_data; }
    inline const T* toPtr() const { return m_data; }

    template <typename X>
    X cast() {
        return X::fromPtr(reinterpret_cast<X::RefCountedType*>(m_data));
    }

private:
    friend class Ref<RefCountedBase<T> >;
    T* m_data;
};

#define TSW_DECLARE_REF_CLASS(HolderClass, ImplClass) \
typedef RefCountedBase<ImplClass> HolderClass

template <typename T>
class Ref {
public:
    Ref()
        : m_data(nullptr) {
    }
    Ref(const RefCountedBase<typename T::RefCountedType>& other)
        : m_data(other.m_data) {
        refIfNotNull();
    }
    Ref(RefCountedBase<typename T::RefCountedType>&& other)
        : m_data(other.m_data) {
        refIfNotNull();
    }
    Ref(const Ref<T>& other)
        : m_data(other.m_data) {
        refIfNotNull();
    }
    Ref(Ref<T>&& other)
        : m_data(other.m_data) {
        refIfNotNull();
    }
    ~Ref() {
        derefIfNeeded();
    }

    Ref<T>& operator=(const Ref<T>& other) {
        if (other.m_data != m_data) {
            derefIfNeeded();
            m_data = other.m_data;
            refIfNotNull();
        }
        return *this;
    }

    Ref& operator=(Ref<T>&& other) {
        if (other.m_data != m_data) {
            derefIfNeeded();
            m_data = other.m_data;
            refIfNotNull();
        }
        return *this;
    }

    void refIfNotNull() { if (m_data != nullptr) m_data->ref(); }
    void refIfNeeded() { refIfNotNull(); }
    void ref() { m_data->ref(); }
    void derefIfNeeded() {
        if (m_data != nullptr) {
            if (!m_data->deref())
                T::RefCountedType::deallocate(m_data);
            m_data = nullptr;
        }
    }
    void deref() { m_data->deref(); }

    inline bool operator==(const Ref<T>& other) const {
        if (m_data == other.m_data)
            return true;
        // TODO(caitp): support using T::RefCountedType's operator== as fallback if the pointer check fails
        return false;
    }
    inline bool operator!=(const Ref<T>& other) const { return !(*this == other); }
    inline bool isNull() const { return m_data == nullptr; }
    inline operator bool() const { return !isNull(); }
    inline operator typename T::RefCountedType*() { return m_data; }
    inline operator typename T::RefCountedType*() const { return m_data; }
    inline typename T::RefCountedType* operator->() { return m_data; }
    inline const typename T::RefCountedType* operator->() const { return m_data; }
    inline typename T::RefCountedType* toPtr() { return m_data; }
    inline const typename T::RefCountedType* toPtr() const { return m_data; }

    static Ref<T> cast(typename T::RefCountedType* ptr) { return T::fromPtr(ptr); }

    template <typename X>
    Ref<X> cast() {
        return Ref<X>::cast(reinterpret_cast<typename X::RefCountedType*>(m_data));
    }

private:
    friend class RefCountedBase<typename T::RefCountedType>;
    typename T::RefCountedType* m_data;
};

template <typename T>
RefCountedBase<T> RefCountedBase<T>::fromPtr(T* data) {
    RefCountedBase<T> ref = RefEmptyConstructor::Tag;
    ref.m_data = data;
    ref.ref();
    return ref;
}

template <typename T>
RefCountedBase<T>::RefCountedBase(const Ref<RefCountedBase<T>>& ref) {
    m_data = ref.m_data;
    refIfNeeded();
}

#define TSW_BASIC_ALLOCATOR(Type) \
template <typename... TArgs> \
static Type* allocate(TArgs... args) { return new Type(args...); } \
static void deallocate(Type* data) { if (data != nullptr) delete data; }

template <class T>
class RefCounted {
public:
    explicit RefCounted() : m_refs(1) {}
    virtual ~RefCounted() {}

    TSW_BASIC_ALLOCATOR(T);

    void ref() { m_refs++; }
    bool deref() { return --m_refs > 0; }

private:
    long m_refs;
};

template <class T>
class RefCountedOrStatic {
public:
    explicit RefCountedOrStatic(long refs) : m_refs(refs) {}
    virtual ~RefCountedOrStatic() {}

    TSW_BASIC_ALLOCATOR(T);

    void ref() { m_refs += 2; }
    bool deref() {
        if (m_refs >= 2) m_refs -= 2;
        return m_refs != 0;
    }

private:
    long m_refs;
};

}  // namespace twitchsw
