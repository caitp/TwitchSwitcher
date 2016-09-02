// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#ifdef TSW_WIN32

#include "windows/COleInPlaceSite.h"
#include <crtdbg.h>

#define NOTIMPLEMENTED _ASSERT(0); return E_NOTIMPL

namespace twitchsw {

// IOleWindow
HRESULT STDMETHODCALLTYPE COleInPlaceSite::GetWindow(HWND* result) {
    // Must be implemented
    NOTIMPLEMENTED;
}
HRESULT STDMETHODCALLTYPE COleInPlaceSite::ContextSensitiveHelp(BOOL enterMode) {
    return E_NOTIMPL;
}

// IOleInPlaceSite
HRESULT STDMETHODCALLTYPE COleInPlaceSite::CanInPlaceActivate() {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COleInPlaceSite::OnInPlaceActivate() {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COleInPlaceSite::OnUIActivate() {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COleInPlaceSite::GetWindowContext(IOleInPlaceFrame** frame, IOleInPlaceUIWindow** doc, LPRECT posRect, LPRECT clipRect, LPOLEINPLACEFRAMEINFO info) {
    // Must be implemented
    NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE COleInPlaceSite::Scroll(SIZE scrollExtent) {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleInPlaceSite::OnUIDeactivate(BOOL undoable) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COleInPlaceSite::OnInPlaceDeactivate() {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COleInPlaceSite::DiscardUndoState() {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleInPlaceSite::DeactivateAndUndo() {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleInPlaceSite::OnPosRectChange(LPCRECT posRect) {
    // Must be implemented
    NOTIMPLEMENTED;
}

}  // namespace twitchsw

#endif  // TSW_WIN32
