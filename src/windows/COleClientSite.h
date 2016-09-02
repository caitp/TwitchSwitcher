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

class COleClientSite : public IOleClientSite {
public:
    TSW_DECLARE_VIRTUAL_IUNKOWN_METHODS();

    virtual HRESULT STDMETHODCALLTYPE SaveObject();
    virtual HRESULT STDMETHODCALLTYPE GetMoniker(DWORD assign, DWORD which, IMoniker** result);
    virtual HRESULT STDMETHODCALLTYPE GetContainer(IOleContainer** result);
    virtual HRESULT STDMETHODCALLTYPE ShowObject();
    virtual HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL show);
    virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout();
};

}  // namespace twitchsw

#endif  // TSW_WIN32
