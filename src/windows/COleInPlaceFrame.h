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

class COleInPlaceFrame : public IOleInPlaceFrame {
public:
    TSW_DECLARE_VIRTUAL_IUNKOWN_METHODS();

    // IOleWindow
    virtual HRESULT STDMETHODCALLTYPE GetWindow(HWND* result) = 0;
    virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL enterMode);

    // IOleInPlaceUIWindow
    virtual HRESULT STDMETHODCALLTYPE GetBorder(LPRECT border);
    virtual HRESULT STDMETHODCALLTYPE RequestBorderSpace(LPCBORDERWIDTHS borderWidths);
    virtual HRESULT STDMETHODCALLTYPE SetBorderSpace(LPCBORDERWIDTHS borderWidths);
    virtual HRESULT STDMETHODCALLTYPE SetActiveObject(IOleInPlaceActiveObject* activeObject, LPCOLESTR objName);

    // IOleInPlaceFrame
    virtual HRESULT STDMETHODCALLTYPE InsertMenus(HMENU shared, LPOLEMENUGROUPWIDTHS menuWidths);
    virtual HRESULT STDMETHODCALLTYPE SetMenu(HMENU shared, HOLEMENU menu, HWND activeObject);
    virtual HRESULT STDMETHODCALLTYPE RemoveMenus(HMENU shared);
    virtual HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR statusText);
    virtual HRESULT STDMETHODCALLTYPE EnableModeless(BOOL enable);
    virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG msg, WORD id);
};

}  // namespace twitchsw

#endif  // TSW_WIN32
