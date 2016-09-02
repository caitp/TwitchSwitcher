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

class COleInPlaceSite : public IOleInPlaceSite {
public:
    TSW_DECLARE_VIRTUAL_IUNKOWN_METHODS();

    // IOleWindow
    virtual HRESULT STDMETHODCALLTYPE GetWindow(HWND* result);
    virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL enterMode);

    // IOleInPlaceSite
    virtual HRESULT STDMETHODCALLTYPE CanInPlaceActivate();
    virtual HRESULT STDMETHODCALLTYPE OnInPlaceActivate();
    virtual HRESULT STDMETHODCALLTYPE OnUIActivate();
    virtual HRESULT STDMETHODCALLTYPE GetWindowContext(IOleInPlaceFrame** frame, IOleInPlaceUIWindow** doc, LPRECT posRect, LPRECT clipRect, LPOLEINPLACEFRAMEINFO info);
    virtual HRESULT STDMETHODCALLTYPE Scroll(SIZE scrollExtent);
    virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL undoable);
    virtual HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate();
    virtual HRESULT STDMETHODCALLTYPE DiscardUndoState();
    virtual HRESULT STDMETHODCALLTYPE DeactivateAndUndo();
    virtual HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT posRect);
};

}  // namespace twitchsw

#endif  // TSW_WIN32
