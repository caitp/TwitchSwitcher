// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#include "mac/webview-wkwebview.h"

#ifdef TSW_WEBVIEW_WKWEBVIEW

namespace twitchsw {

// ??? obs_create_ui doesn't seem to matter anyways, investigate this...
const char* WebView::kOBS_UI_TYPE = "mac";

static WKProcessPool* g_processPool = nullptr;

void WebViewImpl::initialize() {
    g_processPool = [[WKProcessPool alloc] init];
}
void WebViewImpl::shutdown() {
    [g_processPool release];
    g_processPool = nullptr;
}

WebViewImpl::WebViewImpl()
    : m_controller(nil)
    , m_window(nil)
{
}

WebViewImpl::~WebViewImpl()
{
}

void* WebViewImpl::nativeHandle()
{
    return (m_window == nil || m_controller == nil) ? nullptr : m_window;
}


bool WebViewImpl::ensureUI() {
    if (m_window == nil) {
        NSInteger windowStyleFlags = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask | NSFullSizeContentViewWindowMask;
        m_window = [[NSWindow alloc] initWithContentRect:CGRectMake(0, 0, 800, 600)
                                     styleMask:windowStyleFlags
                                     backing:NSBackingStoreBuffered
                                     defer:YES];
        m_window.windowController = [[NSWindowController alloc] initWithWindow:m_window];
        [NSApp setWindowsNeedUpdate:YES];
    }

    if (m_controller == nil) {
        m_controller = [[WebViewController alloc] initWithHolder:this processPool:g_processPool];
        if (m_controller == nil)
            return false;
    }

    if (m_window.contentViewController != m_controller)
        m_window.contentViewController = m_controller;
    return true;
}

void WebViewImpl::open(const std::string& url, const HttpRequestOptions& options)
{
    if (!ensureUI())
        return;

    m_onRedirect = options.onRedirect();

    NSURL* URL = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];

    [m_controller loadURL:URL withOptions:options];
}

void WebViewImpl::close()
{
    auto window = m_window;
    m_window = nil;
    if (window != nil) {
         dispatch_async(dispatch_get_main_queue(), ^{
            [window close];
        });
    }
    if (m_onDestroyed)
        m_onDestroyed();
}

void WebViewImpl::setTitle(const std::string& title)
{
    if (m_window != nil)
        m_window.title = [NSString stringWithUTF8String:title.c_str()];
    m_title = title;
}

void WebViewImpl::show()
{
    if (!ensureUI())
        return;

    auto windowController = m_window.windowController;

    m_window.title = [NSString stringWithUTF8String:m_title.c_str()];
    m_window.titleVisibility = NSWindowTitleVisible;
    dispatch_async(dispatch_get_main_queue(), ^{
        [windowController showWindow:nil];
    });

    //[m_window makeKeyAndOrderFront:m_controller];
    //[NSApp activateIgnoringOtherApps:YES];
}

void WebViewImpl::setOnRedirect(const OnRedirectCallback& callback)
{
    m_onRedirect = callback;
}

void WebViewImpl::setOnComplete(const _OnCompleteCallback& callback)
{
    m_onComplete = callback;
}

void WebViewImpl::setOnWebViewDestroyed(const _OnWebViewDestroyed& callback)
{
    m_onDestroyed = callback;
}

void WebViewImpl::setOnAbort(const _OnAbortCallback& callback)
{
    m_onAbort = callback;
}

}  // namespace twitchsw

#endif  // TSW_WEBVIEW_WKWEBVIEW
