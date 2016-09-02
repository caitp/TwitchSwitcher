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

class CDocHostShowUI : public IDocHostShowUI {
public:
    TSW_DECLARE_VIRTUAL_IUNKOWN_METHODS();

    virtual HRESULT STDMETHODCALLTYPE ShowMessage(HWND hwnd, LPOLESTR text, LPOLESTR caption, DWORD type, LPOLESTR helpFile, DWORD helpContext, LRESULT* result);
    virtual HRESULT STDMETHODCALLTYPE ShowHelp(HWND hwnd, LPOLESTR helpFile, UINT command, DWORD data, POINT mousePos, IDispatch* dispatchObjectHit);
};

}  // namespace twitchsw

#endif  // TSW_WIN32
