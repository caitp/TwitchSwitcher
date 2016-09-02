// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#ifdef TSW_WIN32

#include "windows/COleInPlaceFrame.h"

namespace twitchsw {

// IOleWindow
HRESULT STDMETHODCALLTYPE COleInPlaceFrame::ContextSensitiveHelp(BOOL enterMode) {
    return E_NOTIMPL;
}

// IOleInPlaceUIWindow
HRESULT STDMETHODCALLTYPE COleInPlaceFrame::GetBorder(LPRECT border) {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleInPlaceFrame::RequestBorderSpace(LPCBORDERWIDTHS borderWidths) {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleInPlaceFrame::SetBorderSpace(LPCBORDERWIDTHS borderWidths) {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleInPlaceFrame::SetActiveObject(IOleInPlaceActiveObject* activeObject, LPCOLESTR objName) {
    return S_OK;
}

// IOleInPlaceFrame
HRESULT STDMETHODCALLTYPE COleInPlaceFrame::InsertMenus(HMENU shared, LPOLEMENUGROUPWIDTHS menuWidths) {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleInPlaceFrame::SetMenu(HMENU shared, HOLEMENU menu, HWND activeObject) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COleInPlaceFrame::RemoveMenus(HMENU shared) {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleInPlaceFrame::SetStatusText(LPCOLESTR statusText) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COleInPlaceFrame::EnableModeless(BOOL enable) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COleInPlaceFrame::TranslateAccelerator(LPMSG msg, WORD id) {
    return E_NOTIMPL;
}

}  // namespace twitchsw

#endif  // TSW_WIN32
