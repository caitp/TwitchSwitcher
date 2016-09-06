// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.
// ----------------------------------------------------------------------------
// Portions of this code have been extracted from WebKit/Source/WTTF/wtf/Ref.h
// and WebKit/Source/WTF/wtf/WeakPtr.h, which were both governed under 2-clause
// BSD licenses. The following header grants copyright and attribution to the
// authors.
//
// Copyright (C) 2013 Google, Inc. All Rights Reserved.
// Copyright (C) 2015 Apple Inc. All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <twitchsw/compiler.h>
#include <twitchsw/twitchsw.h>

#include <algorithm>

namespace twitchsw {

inline void adopted(const void*) { }

template <typename T> class Ref;

enum class RefEmptyConstructor { Tag };

template <typename T> class Ref;
template <typename T> Ref<T> adoptRef(T&);
template <typename T> class RefPtr;
template <typename T> RefPtr<T> adoptRef(T* p);
template <typename T> class PassRefPtr;

template <typename T>
ALWAYS_INLINE void refIfNotNull(T* ptr)
{
    if (LIKELY(ptr != nullptr))
        ptr->ref();
}

template <typename T>
ALWAYS_INLINE void derefIfNotNull(T* ptr)
{
    if (LIKELY(ptr != nullptr))
        ptr->deref();
}
template <typename T> class ThreadSafeRefCounted;
class ThreadSafeRefCountedBase {
public:
    ThreadSafeRefCountedBase(ThreadSafeRefCountedBase&&) = delete;
    ThreadSafeRefCountedBase(const ThreadSafeRefCountedBase&) = delete;
    ThreadSafeRefCountedBase& operator=(ThreadSafeRefCountedBase&&) = delete;
    ThreadSafeRefCountedBase& operator=(const ThreadSafeRefCountedBase&) = delete;

    ThreadSafeRefCountedBase(int initialRefCount = 1)
        : m_refCount(initialRefCount)
    {
    }

    void ref()
    {
        ++m_refCount;
    }

    bool hasOneRef()
    {
        return refCount() == 1;
    }

    int refCount() const
    {
        return m_refCount;
    }

protected:
    // Unlike WTF conventions, in TSW deref() returns false if the pointer should
    // be freed, not true.
    bool derefBase()
    {
        if (--m_refCount == 0)
            return false;
        return true;
    }

private:
    template <typename T> friend class ThreadSafeRefCounted;
    std::atomic<int> m_refCount;
};

template <class T>
class ThreadSafeRefCounted : public ThreadSafeRefCountedBase {
public:
    void deref()
    {
        if (!derefBase())
            delete static_cast<T*>(this);
    }

protected:
    ThreadSafeRefCounted()
    {
    }
};

class RefCountedBase {
public:
    void ref() const
    {
        ++m_refCount;
    }

    bool hasOneRef() const
    {
        return m_refCount == 1;
    }

    unsigned refCount() const
    {
        return m_refCount;
    }

protected:
    RefCountedBase()
        : m_refCount(1)
    {
    }

    bool derefBase() const
    {
        unsigned tempRefCount = m_refCount - 1;
        if (tempRefCount == 0)
            return false;
        m_refCount = tempRefCount;
        return true;
    }

private:
    template <typename T> friend class RefCounted;
    mutable unsigned m_refCount;
};

template <typename T>
class RefCounted : public RefCountedBase {
public:
    RefCounted(const RefCounted&) = delete;
    RefCounted(RefCounted&&) = delete;
    RefCounted& operator=(const RefCounted&) = delete;
    RefCounted& operator=(RefCounted&&) = delete;

    void deref() const
    {
        if (!derefBase())
            delete static_cast<const T*>(this);
    }

protected:
    RefCounted() { }
    ~RefCounted() { }
};

template <typename T>
class Ref {
public:
    static constexpr bool isRef = true;

    Ref() = delete;
    Ref(T& object)
        : m_data(&object)
    {
        m_data->ref();
    }

    Ref(Ref&& other)
        : m_data(&other.leakRef())
    {
    }

    ~Ref()
    {
#if ASAN_ENABLED
        if (__asan_address_is_poisoned(this))
            __asan_unpoison_memory_region(this, sizeof(*this));
#endif
        if (m_data)
            m_data->deref();
    }

    Ref& operator=(T& object)
    {
        object.ref();
        if (m_data)
            m_data->deref();
        m_data = &object;
        return *this;
    }

    Ref& operator=(const Ref&) = delete;
    template <typename U> Ref& operator=(const Ref<U>&) = delete;

    template <typename U>
    Ref& operator=(Ref<U>&& ref)
    {
        T* old = m_data;
        m_data = &ref.leakRef();
        if (old)
            old->deref();
        return *this;
    }

    inline T& operator*()
    {
        // TODO(caitp): assert(m_data)
        return *m_data;
    }

    inline const T& operator*() const
    {
        // TODO(caitp): assert(m_data)
        return *m_data;
    }

    const T* operator->() const
    {
        return m_data;
    }

    T* operator->()
    {
        return m_data;
    }

    const T* ptr() const
    {
        return m_data;
    }

    T* ptr()
    {
        return m_data;
    }

    const T& get() const
    {
        // TODO(caitp): assert(m_data);
        return *m_data;
    }

    T& get()
    {
        // TODO(caitp): assert(m_data);
        return *m_data;
    }

    operator T&()
    {
        // TODO(caitp): assert(m_data);
        return *m_data;
    }

    operator const T&() const
    {
        // TODO(caitp): assert(m_data);
        return *m_data;
    }

    template<typename U> Ref<T> replace(Ref<U>&&);
    template<typename U>
    Ref<U> cast()
    {
        return Ref<U>(static_cast<U&>(get()));
    }

#if TSW_COMPILER_SUPPORTS(CXX_REFERENCE_QUALIFIED_FUNCTIONS)
    // Restrict copyRef from being used on the result of a function call which
    // returns Ref<T>.
    Ref copyRef() && = delete;
    Ref copyRef() const& { return Ref(*m_data); }
#else
    Ref copyRef() const { return Ref(*m_data); }
#endif

    T& leakRef()
    {
        T& result = *std::exchange(m_data, nullptr);
#if ASAN_ENABLED
        __asan_poison_memory_region(this, sizeof(*this));
#endif
        return result;
    }

private:
    friend Ref adoptRef<T>(T&);

    enum AdoptTag { Adopt };
    Ref(T& object, AdoptTag)
        : m_data(&object)
    {
    }

    T* m_data;
};

template<typename T>
inline Ref<T> adoptRef(T& reference)
{
    adopted(&reference);
    return Ref<T>(reference, Ref<T>::Adopt);
}

template <typename T>
template <typename U>
inline Ref<T> Ref<T>::replace(Ref<U>&& ref)
{
    auto old = adoptRef(*m_data);
    m_data = &ref.leakRef();
    return old;
}

#define TSW_BASIC_ALLOCATOR(Type) \
template <typename... TArgs> \
static Type* allocate(TArgs... args) { return new Type(args...); } \
static void deallocate(Type* data) { if (data != nullptr) delete data; }

// RefPtr
template <typename T>
class RefPtr {
public:
    typedef T value_type;
    typedef T* pointer_type;

    static constexpr bool isRefPtr = true;

    ALWAYS_INLINE RefPtr() : m_data(nullptr) { }
    ALWAYS_INLINE RefPtr(T* ptr) : m_data(ptr) { refIfNotNull(ptr); }
    ALWAYS_INLINE RefPtr(const RefPtr& o) : m_data(o.get()) { refIfNotNull(m_data); }
    template <typename U>
    RefPtr(const RefPtr<U>& o) : m_data(o.get()) { refIfNotNull(m_data); }

    ALWAYS_INLINE RefPtr(RefPtr&& o) : m_data(o.leakRef()) { }
    template <typename U> RefPtr(RefPtr<U>&& o) : m_data(o.leakRef()) { }

    template <typename U> RefPtr(const PassRefPtr<U>&);
    template <typename U> RefPtr(Ref<U>&&);

    ALWAYS_INLINE ~RefPtr()
    {
        derefIfNotNull(std::exchange(m_data, nullptr));
    }

    ALWAYS_INLINE T* get() const { return m_data; }

    T* leakRef();

    T& operator*() const
    {
        // TODO(caitp): assert(m_data)
        return *m_data;
    }

    ALWAYS_INLINE T* operator->() const
    {
        return m_data;
    }

    template <typename U>
    inline RefPtr<U> cast()
    {
        return adoptRef<U>(leakRef());
    }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef T* (RefPtr::*UnspecifiedBoolType);
    operator UnspecifiedBoolType() const { return m_data ? &RefPtr::m_data : nullptr; }

    bool isNull() const { return m_data == nullptr; }

    RefPtr& operator=(const RefPtr&);
    RefPtr& operator=(T*);
    RefPtr& operator=(std::nullptr_t);
    RefPtr& operator=(const PassRefPtr<T>&);
    template <typename U> RefPtr& operator=(const RefPtr<U>&);
    template <typename U> RefPtr& operator=(const PassRefPtr<U>&);
    RefPtr& operator=(RefPtr&&);
    template <typename U> RefPtr& operator=(RefPtr<U>&&);
    template <typename U> RefPtr& operator=(Ref<U>&&);

    void swap(RefPtr&);

#if TSW_COMPILER_SUPPORTS(CXX_REFERENCE_QUALIFIED_FUNCTIONS)
    RefPtr copyRef()&& = delete;
    RefPtr copyRef() const & { return RefPtr(m_data); }
#else
    RefPtr copyRef const { return RefPtr(m_data); }
#endif  // TSW_COMPILER_SUPPORTS(CXX_REFERENCE_QUALIFIED_FUNCTIONS)

private:
    friend RefPtr adoptRef<T>(T*);

    enum AdoptTag { Adopt };
    RefPtr(T* ptr, AdoptTag) : m_data(ptr) { }

    T* m_data;
};

// PassRefPtr
template <typename T>
class PassRefPtr {
public:
    typedef T value_type;
    typedef T* pointer_type;

    PassRefPtr() : m_data(nullptr) {}
    PassRefPtr(T* ptr) : m_data(ptr)
    {
        refIfNotNull(ptr);
    }

    PassRefPtr(const PassRefPtr& o)
        : m_data(o.leakRef())
    {
    }

    template <typename U>
    PassRefPtr(const PassRefPtr<U>& o)
        : m_data(o.leakRef())
    {
    }

    ALWAYS_INLINE ~PassRefPtr()
    {
        derefIfNotNull(std::exchange(m_data, nullptr));
    }

    template <typename U> PassRefPtr(const RefPtr<U>&);
    template <typename U> PassRefPtr(Ref<U>&& ref) : m_data(&ref.leakRef()) { }
    template <typename U> PassRefPtr(RefPtr<U>&& ref) : m_data(ref.leakRef()) { }

    T* get() const { return m_data; }
    T* leakRef() const;

    T& operator*() const { return *m_data; }
    T* operator->() const { return m_data; }

    ALWAYS_INLINE bool operator!() const { return !m_data; }
    ALWAYS_INLINE bool isNull() const { return m_data == nullptr; }

    // This conversion operator allows implicit conversion to bool but not to other integer types.
    typedef T* (PassRefPtr::*UnspecifiedBoolType);
    operator UnspecifiedBoolType() const { return m_data ? &PassRefPtr::m_data : nullptr; }

private:
    enum AdoptTag { Adopt };
    PassRefPtr(T* ptr, AdoptTag) : m_data(ptr) { }

    mutable T* m_data;
};

template <typename T>
template <typename U>
inline RefPtr<T>::RefPtr(const PassRefPtr<U>& o)
    : m_data(o.leakRef())
{
}

template <typename T>
template <typename U>
inline RefPtr<T>::RefPtr(Ref<U>&& ref)
    : m_data(&ref.leakRef())
{
}

template <typename T>
inline T* RefPtr<T>::leakRef()
{
    return std::exchange(m_data, nullptr);
}

template <typename T>
inline RefPtr<T>& RefPtr<T>::operator=(const RefPtr& o)
{
    RefPtr ptr = o;
    swap(ptr);
    return *this;
}

template <typename T>
template <typename U>
inline RefPtr<T>& RefPtr<T>::operator=(const RefPtr<U>& o)
{
    RefPtr ptr = o;
    swap(ptr);
    return *this;
}

template <typename T>
inline RefPtr<T>& RefPtr<T>::operator=(T* optr)
{
    RefPtr ptr = optr;
    swap(ptr);
    return *this;
}

template <typename T>
inline RefPtr<T>& RefPtr<T>::operator=(std::nullptr_t)
{
    derefIfNotNull(std::exchange(m_data, nullptr));
    return *this;
}

template <typename T>
inline RefPtr<T>& RefPtr<T>::operator=(const PassRefPtr<T>& o)
{
    RefPtr ptr = o;
    swap(ptr);
    return *this;
}

template <typename T>
template <typename U>
inline RefPtr<T>& RefPtr<T>::operator=(const PassRefPtr<U>& o)
{
    RefPtr ptr = o;
    swap(ptr);
    return *this;
}

template <typename T>
inline RefPtr<T>& RefPtr<T>::operator=(RefPtr&& o)
{
    RefPtr ptr = std::move(o);
    swap(ptr);
    return *this;
}

template <typename T>
template <typename U>
inline RefPtr<T>& RefPtr<T>::operator=(RefPtr<U>&& o)
{
    RefPtr ptr = std::move(o);
    swap(ptr);
    return *this;
}

template<typename T>
template<typename U>
inline RefPtr<T>& RefPtr<T>::operator=(Ref<U>&& ref)
{
    RefPtr ptr = std::move(ref);
    swap(ptr);
    return *this;
}

template <typename T>
inline void RefPtr<T>::swap(RefPtr& o)
{
    std::swap(m_data, o.m_data);
}

template <typename T>
void swap(RefPtr<T>& a, RefPtr<T>& b)
{
    a.swap(b);
}

template <typename T, typename U>
inline bool operator==(const RefPtr<T>& a, const RefPtr<U>& b)
{
    return a.get() == b.get();
}

template <typename T, typename U>
inline bool operator==(const RefPtr<T>& a, U* b)
{
    return a.get() == b;
}

template <typename T, typename U>
inline bool operator==(T* a, const RefPtr<U>& b)
{
    return a == b.get();
}

template <typename T, typename U>
inline bool operator!=(const RefPtr<T>& a, const RefPtr<U>& b)
{
    return a.get() != b.get();
}

template <typename T, typename U>
inline bool operator!=(const RefPtr<T>& a, U* b)
{
    return a.get() != b;
}

template <typename T, typename U>
inline bool operator!=(T* a, const RefPtr<U>& b)
{
    return a != b.get();
}

template<typename T>
inline RefPtr<T> adoptRef(T* p)
{
    adopted(p);
    return RefPtr<T>(p, RefPtr<T>::Adopt);
}

template <typename T>
template <typename U>
inline PassRefPtr<T>::PassRefPtr(const RefPtr<U>& o)
    : m_data(o.get())
{
    T* ptr = m_data;
    refIfNotNull(ptr);
}

template <typename T>
inline T* PassRefPtr<T>::leakRef() const
{
    return std::exchange(m_data, nullptr);
}

template <typename T, typename U>
inline bool operator==(const PassRefPtr<T>& a, const PassRefPtr<T>& b)
{
    return a.get() == b.get();
}

template<typename T, typename U>
inline bool operator==(const PassRefPtr<T>& a, const RefPtr<U>& b)
{
    return a.get() == b.get();
}

template<typename T, typename U>
inline bool operator==(const RefPtr<T>& a, const PassRefPtr<U>& b)
{
    return a.get() == b.get();
}

template<typename T, typename U>
inline bool operator==(const PassRefPtr<T>& a, U* b)
{
    return a.get() == b;
}

template<typename T, typename U>
inline bool operator==(T* a, const PassRefPtr<U>& b)
{
    return a == b.get();
}

template<typename T, typename U>
inline bool operator!=(const PassRefPtr<T>& a, const PassRefPtr<U>& b)
{
    return a.get() != b.get();
}

template<typename T, typename U>
inline bool operator!=(const PassRefPtr<T>& a, const RefPtr<U>& b)
{
    return a.get() != b.get();
}

template<typename T, typename U>
inline bool operator!=(const RefPtr<T>& a, const PassRefPtr<U>& b)
{
    return a.get() != b.get();
}

template<typename T, typename U>
inline bool operator!=(const PassRefPtr<T>& a, U* b)
{
    return a.get() != b;
}

template<typename T, typename U>
inline bool operator!=(T* a, const PassRefPtr<U>& b)
{
    return a != b.get();
}

// WeakPtr implementation
template <typename T> class WeakPtr;
template <typename T> class WeakPtrFactory;
namespace detail {

template <typename T>
class WeakReference : public RefCounted<WeakReference<T>> {
public:
    WeakReference(const WeakReference&) = delete;
    WeakReference(WeakReference&&) = delete;
    WeakReference& operator=(const WeakReference&) = delete;
    WeakReference& operator=(WeakReference&&) = delete;

    T* get() const
    {
        return m_data;
    }

    void clear()
    {
        m_data = nullptr;
    }

private:
    friend class WeakPtr<T>;
    friend class WeakPtrFactory<T>;

    static Ref<WeakReference<T>> create(T* ptr) { return adoptRef(*new WeakReference(ptr)); }
    explicit WeakReference(T* ptr)
        : m_data(ptr)
    {
    }

    T* m_data;
};

}  // namespace details

template <typename T> class WeakPtrFactory;
template <typename T>
class WeakPtr {
public:
    WeakPtr() : m_ref(detail::WeakReference<T>::create(nullptr)) { }
    WeakPtr(const WeakPtr& o) : m_ref(o.m_ref.copyRef()) { }
    template <typename U>
    WeakPtr(const WeakPtr<U>& o)
        : m_ref(o.m_ref.copyref())
    {
    }

    T* get() const { return m_ref->get(); }
    operator bool() const { return m_ref->get(); }
    bool isNull() const { return get() == nullptr; }

    WeakPtr& operator=(const WeakPtr& o) { m_ref = o.m_ref.copyRef(); return *this; }
    WeakPtr& operator=(std::nullptr_t) { m_ref = detail::WeakReference<T>::create(nullptr); return *this; }
    WeakPtr& operator=(Ref<T>& o) { WeakPtr ref = o->createWeakPtr(); m_ref = ref.m_ref.copyRef(); return *this; }

    T* operator->() const { return m_ref->get(); }

    void clear() { m_ref = detail::WeakReference<T>::create(nullptr); }

private:
    friend class WeakPtrFactory<T>;
    WeakPtr(Ref<detail::WeakReference<T>>&& ref) : m_ref(std::forward<Ref<detail::WeakReference<T>>>(ref)) { }

    Ref<detail::WeakReference<T>> m_ref;
};

template <typename T>
class WeakPtrFactory {
public:
    WeakPtrFactory(const WeakPtrFactory&) = delete;
    WeakPtrFactory(WeakPtrFactory&&) = delete;
    WeakPtrFactory& operator=(const WeakPtrFactory&) = delete;
    WeakPtrFactory& operator=(WeakPtrFactory&&) = delete;

    explicit WeakPtrFactory(T* ptr) : m_ref(detail::WeakReference<T>::create(ptr)) { }
    ~WeakPtrFactory() { m_ref->clear(); }

    WeakPtr<T> createWeakPtr() const { return WeakPtr<T>(m_ref.copyRef()); }

    void revokeAll()
    {
        T* ptr = m_ref->get();
        m_ref->clear();
        m_ref = detail::WeakReference<T>::create(ptr);
    }

private:
    Ref<detail::WeakReference<T>> m_ref;
};

template<typename T, typename U>
inline bool operator==(const WeakPtr<T>& a, const WeakPtr<U>& b)
{
    return a.get() == b.get();
}

template<typename T, typename U>
inline bool operator==(const WeakPtr<T>& a, U* b)
{
    return a.get() == b;
}

template<typename T, typename U>
inline bool operator==(T* a, const WeakPtr<U>& b)
{
    return a == b.get();
}

template<typename T, typename U>
inline bool operator!=(const WeakPtr<T>& a, const WeakPtr<U>& b)
{
    return a.get() != b.get();
}

template<typename T, typename U>
inline bool operator!=(const WeakPtr<T>& a, U* b)
{
    return a.get() != b;
}

template<typename T, typename U>
inline bool operator!=(T* a, const WeakPtr<U>& b)
{
    return a != b.get();
}

}  // namespace twitchsw
