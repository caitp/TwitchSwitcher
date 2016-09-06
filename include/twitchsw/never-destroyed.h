// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.
// ----------------------------------------------------------------------------
// Code is taken from WebKit/Source/WTF/wtf/NeverDestroyed.h, governed by a
// 2 clause BSD license, which is copied below:
//
//
// Copyright (C) 2013 Apple Inc. All rights reserved.
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
// THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.

namespace twitchsw {

template<typename T> class NeverDestroyed {
public:
    NeverDestroyed(const NeverDestroyed&) = delete;
    NeverDestroyed(NeverDestroyed&&) = delete;
    NeverDestroyed& operator=(const NeverDestroyed&) = delete;
    NeverDestroyed& operator=(NeverDestroyed&&) = delete;

    template<typename... Args>
    NeverDestroyed(Args&&... args)
    {
        MaybeRelax<T>(new (asPtr()) T(std::forward<Args>(args)...));
    }

    operator T&() { return *asPtr(); }
    T& get() { return *asPtr(); }

private:
    typedef typename std::remove_const<T>::type* PointerType;

    PointerType asPtr() { return reinterpret_cast<PointerType>(&m_storage); }

    // FIXME: Investigate whether we should allocate a hunk of virtual memory
    // and hand out chunks of it to NeverDestroyed instead, to reduce fragmentation.
    typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type m_storage;

    template <typename PtrType, bool ShouldRelax = std::is_base_of<RefCountedBase, PtrType>::value>
    struct MaybeRelax {
        explicit MaybeRelax(PtrType*) { }
    };
    template <typename PtrType>
    struct MaybeRelax<PtrType, true> {
        explicit MaybeRelax(PtrType* ptr)
        {
            // ptr->relaxAdoptionRequirement();
        }
    };
};

}  // namespace twitchsw
