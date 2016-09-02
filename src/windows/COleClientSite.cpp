// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#ifdef TSW_WIN32

#include "windows/COleClientSite.h"
#include <crtdbg.h>

#define NOTIMPLEMENTED _ASSERT(0); return E_NOTIMPL

namespace twitchsw {

HRESULT STDMETHODCALLTYPE COleClientSite::SaveObject() {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleClientSite::GetMoniker(DWORD dwAssign,
    DWORD dwWhichMoniker, IMoniker **ppmk) {
    NOTIMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE COleClientSite::GetContainer(IOleContainer** result) {
    if (!result)
        return E_INVALIDARG;
    *result = nullptr;
    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE COleClientSite::ShowObject() {
    // This method is called when someone wants the web browser container to
    // display its object to the user.
    // A common reason this method is called is that someone called DoVerb with
    // a verb that requires the web browser to be visible. One such verb is
    // OLEIVERB_SHOW
    return S_OK;
}

HRESULT STDMETHODCALLTYPE COleClientSite::OnShowWindow(BOOL show) {
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE COleClientSite::RequestNewObjectLayout() {
    return E_NOTIMPL;
}


}  // namespace twitchsw

#endif  // TSW_WIN32
