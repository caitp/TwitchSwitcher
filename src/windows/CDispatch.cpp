// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#ifdef TSW_WIN32

#include "windows/CDispatch.h"

namespace twitchsw {

HRESULT STDMETHODCALLTYPE CDispatch::GetTypeInfoCount(UINT* info) {
    if (!info)
        return E_INVALIDARG;
    *info = 0;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDispatch::GetTypeInfo(UINT index, LCID lcid, ITypeInfo** result) {
    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDispatch::GetIDsOfNames(REFIID riid, LPOLESTR* names, UINT count, LCID lcid, DISPID* result) {
    return E_FAIL;
}

HRESULT STDMETHODCALLTYPE CDispatch::Invoke(DISPID member, REFIID riid, LCID lcid, WORD flags, DISPPARAMS* params,
                                            VARIANT* result, EXCEPINFO* excepInfo, UINT* argErr) {
    return DISP_E_MEMBERNOTFOUND;
}

}  // namespace twitchsw

#endif  // TSW_WIN32
