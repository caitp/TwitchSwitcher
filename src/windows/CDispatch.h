// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#ifdef TSW_WIN32

#include <windows.h>
#include <mshtmhst.h>

#include "windows/COM.h"

namespace twitchsw {

class CDispatch : public IDispatch {
public:
    TSW_DECLARE_VIRTUAL_IUNKOWN_METHODS();

    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT* result);
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT index, LCID lcid, ITypeInfo** result);
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR* names, UINT count, LCID lcid, DISPID* result);
    virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID member, REFIID riid, LCID lcid, WORD flags, DISPPARAMS* params, VARIANT* result, EXCEPINFO* excepInfo, UINT* argErr);
};

}  // namespace twitchsw

#endif  // TSW_WIN32
