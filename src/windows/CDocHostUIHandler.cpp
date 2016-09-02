// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#ifdef TSW_WIN32

#include "windows/CDocHostUIHandler.h"

namespace twitchsw {

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::ShowContextMenu(DWORD id, POINT* mousePos, IUnknown* cmdtReserved, IDispatch* dispReserved) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::GetHostInfo(DOCHOSTUIINFO* result) {
    // Called at initialization of the browser object UI. We can set various
    // features of the browser object here.
    // We can do disable the 3D border (DOCHOSTUIFLAG_NO3DOUTERBORDER) and
    // other things like hide the scroll bar (DOCHOSTUIFLAG_SCROLL_NO), display
    // picture display (DOCHOSTUIFLAG_NOPICS), disable any script running when
    // the page is loaded (DOCHOSTUIFLAG_DISABLE_SCRIPT_INACTIVE), open a site
    // in a new browser window when the user clicks on some link
    // (DOCHOSTUIFLAG_OPENNEWWIN), and lots of other things. See the MSDN docs
    // on the DOCHOSTUIINFO struct passed to us.
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::ShowUI(DWORD id, IOleInPlaceActiveObject* activeObject, IOleCommandTarget* commandTarget, IOleInPlaceFrame* frame,
    IOleInPlaceUIWindow* doc) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::HideUI() {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::UpdateUI() {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::EnableModeless(BOOL enable) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::OnDocWindowActivate(BOOL activate) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::OnFrameWindowActivate(BOOL activate) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::ResizeBorder(LPCRECT border, IOleInPlaceUIWindow* window, BOOL frameWindow) {
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::TranslateAccelerator(LPMSG msg, const GUID* cmdGroup, DWORD count) {
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::GetOptionKeyPath(LPOLESTR* key, DWORD dw) {
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::GetDropTarget(IDropTarget* dropTarget, IDropTarget** result) {
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::GetExternal(IDispatch** result) {
    *result = nullptr;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::TranslateUrl(DWORD translate, OLECHAR* url, OLECHAR** result) {
    *result = nullptr;
    return S_FALSE;
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::FilterDataObject(IDataObject*dataObject, IDataObject** result) {
    *result = nullptr;
    return S_FALSE;
}

}  // namespace twitchsw

#endif  // TSW_WIN32
