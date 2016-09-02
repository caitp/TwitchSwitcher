// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#ifdef TSW_WIN32

#include <crtdbg.h>
#include <windows.h>
#include <stdint.h>

namespace twitchsw {

#define TSW_DECLARE_IUNKNOWN_MEMBERS() \
    ULONG m_refs \

#define TSW_DECLARE_VIRTUAL_IUNKOWN_METHODS() \
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** result) = 0; \
    virtual ULONG STDMETHODCALLTYPE AddRef() = 0; \
    virtual ULONG STDMETHODCALLTYPE Release() = 0

#define TSW_IMPLEMENT_COMMON_METHODS() \
    STDMETHOD_(ULONG, AddRef)() {     \
        InterlockedIncrement(&m_refs); \
        return m_refs; \
    } \
    STDMETHOD_(ULONG, Release)() { \
        InterlockedDecrement(&m_refs); \
        if (!m_refs) { \
            delete this; \
            return 0; \
        } \
        return m_refs; \
    }

// COM event handling
enum ExtendedVariantTypes {
    TSW_VARIANTREF = 0x800
};

namespace COM {

template<VARTYPE type>
struct COMTypeInfo {
    typedef VARIANT ResultType;
    void extract(VARIANT* in, ResultType& out) { out = *in; }
};

#define TSW_COM_TYPEINFO(VT, TYPE, MEMBER) \
template<> \
struct COMTypeInfo<VT> { \
        typedef TYPE ResultType; \
        static void extract(VARIANT* in, ResultType& out) { out = static_cast<ResultType>(in->MEMBER); } \
}; \
template<> \
struct COMTypeInfo<VT | TSW_VARIANTREF> { \
        typedef TYPE ResultType; \
        static void extract(VARIANT* in, ResultType& out) { out = static_cast<ResultType>(in->MEMBER); } \
};

#define TSW_COM_TYPEINFO_REF(VT, TYPE, MEMBER) \
template<> \
struct COMTypeInfo<VT | VT_BYREF> { \
    typedef TYPE* ResultType; \
    static void extract(VARIANT* in, ResultType& out) { out = reinterpret_cast<ResultType>(in->MEMBER); } \
};

#define TSW_DECLARE_COM_TYPEINFO(VT, TYPE, MEMBER) \
    TSW_COM_TYPEINFO(VT, TYPE, MEMBER); \
    TSW_COM_TYPEINFO_REF(VT, TYPE, p##MEMBER);

TSW_DECLARE_COM_TYPEINFO(VT_BSTR, BSTR, bstrVal);
TSW_DECLARE_COM_TYPEINFO(VT_DISPATCH, IDispatch*, pdispVal);

// Avoid VARIANT_BOOL performance warning
#pragma warning(suppress : 4800)
TSW_DECLARE_COM_TYPEINFO(VT_BOOL, bool, boolVal);

TSW_DECLARE_COM_TYPEINFO(VT_I8, int64_t, llVal);
TSW_DECLARE_COM_TYPEINFO(VT_I4, int32_t, intVal);
TSW_DECLARE_COM_TYPEINFO(VT_I2, int16_t, iVal);
TSW_DECLARE_COM_TYPEINFO(VT_I1, int8_t, cVal);
TSW_DECLARE_COM_TYPEINFO(VT_UI8, uint64_t, ullVal);
TSW_DECLARE_COM_TYPEINFO(VT_UI4, uint32_t, uintVal);
TSW_DECLARE_COM_TYPEINFO(VT_UI2, uint16_t, uiVal);
TSW_DECLARE_COM_TYPEINFO(VT_UI1, uint8_t, bVal);
TSW_COM_TYPEINFO(VT_VARIANT, VARIANT*, pvarVal);
TSW_COM_TYPEINFO_REF(VT_VARIANT, VARIANT, pvarVal);

template <VARTYPE expected>
bool maybeGetParameter(DISPPARAMS* params, int pos, typename COMTypeInfo<expected>::ResultType& result, bool optional = false) {
    VARIANTARG* param = params->rgvarg + (params->cArgs - (pos + 1));
    const VARTYPE kExpectedType = expected;
    if ((param->vt & ~VT_BYREF) == VT_EMPTY && optional)
        return true;
    if (param->vt != expected)
        return false;
    COMTypeInfo<expected>::extract(param, result);
    return true;
}

template <VARTYPE expected>
typename COMTypeInfo<expected>::ResultType getParameter(DISPPARAMS* params, int pos) {
    typename COMTypeInfo<expected & ~TSW_VARIANTREF>::ResultType result;
    if (expected & TSW_VARIANTREF) {
        VARIANT* var = nullptr;
        bool status = maybeGetParameter<VT_VARIANT | VT_BYREF>(params, pos, var, false);
        _ASSERT(status);
        _ASSERT(var->vt == (expected & ~TSW_VARIANTREF));
        COMTypeInfo<expected & ~TSW_VARIANTREF>::extract(var, result);
        return result;
    } else {
        bool status = maybeGetParameter<expected>(params, pos, result, false);
        _ASSERT(status);
        return result;
    }
}

template <VARTYPE expected>
bool maybeGetVariantParameter(DISPPARAMS* params, int pos, VARIANT*& result, bool optional = false) {
    VARIANTARG* param = params->rgvarg + (params->cArgs - (pos + 1));
    if (param->vt != expected) {
        result = nullptr;
        return false;
    }
    result = param;
    return true;
}

template <VARTYPE expected>
bool maybeGetVariantRefParameter(DISPPARAMS* params, int pos, typename COMTypeInfo<expected>::ResultType& result, bool optional = false) {
    VARIANTARG* param = params->rgvarg + (params->cArgs - (pos + 1));
    const VARTYPE kExpectedType = expected;
    if ((param->vt & ~VT_BYREF) == VT_EMPTY && optional)
        return true;
    if (param->vt == (VT_VARIANT | VT_BYREF)) {
        param = param->pvarVal;
        if (param->vt != expected)
            return false;
        COMTypeInfo<expected>::extract(param, result);
        return true;
    }

    return false;
}

#undef TSW_DECLARE_COM_TYPEINFO
#undef TSW_COM_TYPEINFO
#undef TSW_COM_TYPEINFO_REF

}  // namespace COM
}  // namespace twitchsw

#endif  // TSW_WIN32
