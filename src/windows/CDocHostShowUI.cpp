// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#ifdef TSW_WIN32

#include "windows/CDocHostShowUI.h"

namespace twitchsw {

HRESULT STDMETHODCALLTYPE CDocHostShowUI::ShowMessage(HWND hwnd, LPOLESTR text, LPOLESTR caption, DWORD type, LPOLESTR helpFile, DWORD helpContext, LRESULT* result) {
    // Called by MSHTML to display a message box.
    // S_OK: Host displayed its UI. MSHTML does not display its message box.
    // S_FALSE: Host did not display its UI. MSHTML displays its message box.
    *result = IDCANCEL;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CDocHostShowUI::ShowHelp(HWND hwnd, LPOLESTR helpFile, UINT command, DWORD data, POINT mousePos, IDispatch* dispatchObjectHit) {
    return S_OK;
}

}  // namespace twitchsw

#endif  // TSW_WIN32
