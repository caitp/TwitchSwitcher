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

class CDocHostUIHandler : public IDocHostUIHandler {
public:
    TSW_DECLARE_VIRTUAL_IUNKOWN_METHODS();

    virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD id, POINT* mousePos, IUnknown* cmdtReserved, IDispatch* dispReserved);
    virtual HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO* result);
    virtual HRESULT STDMETHODCALLTYPE ShowUI(DWORD id, IOleInPlaceActiveObject* activeObject, IOleCommandTarget* commandTarget, IOleInPlaceFrame* frame,
                                             IOleInPlaceUIWindow* doc);
    virtual HRESULT STDMETHODCALLTYPE HideUI();
    virtual HRESULT STDMETHODCALLTYPE UpdateUI();
    virtual HRESULT STDMETHODCALLTYPE EnableModeless(BOOL enable);
    virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate(BOOL activate);
    virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(BOOL activate);
    virtual HRESULT STDMETHODCALLTYPE ResizeBorder(LPCRECT border, IOleInPlaceUIWindow* window, BOOL frameWindow);
    virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG msg, const GUID* cmdGroup, DWORD count);
    virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath(LPOLESTR* key, DWORD dw);
    virtual HRESULT STDMETHODCALLTYPE GetDropTarget(IDropTarget* dropTarget, IDropTarget** result);
    virtual HRESULT STDMETHODCALLTYPE GetExternal(IDispatch** result);
    virtual HRESULT STDMETHODCALLTYPE TranslateUrl(DWORD translate, OLECHAR* url, OLECHAR** result);
    virtual HRESULT STDMETHODCALLTYPE FilterDataObject(IDataObject*dataObject, IDataObject** result);
};

}  // namespace twitchsw

#endif  // TSW_WIN32
