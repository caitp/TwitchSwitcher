// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#pragma once

#ifdef TSW_MAC

#include <twitchsw/http.h>

#import <AppKit/AppKit.h>

namespace twitchsw {

class WebViewImpl;

}  // namespace twitchsw

@class WKWebView;
@interface WebViewController : NSViewController

- (id)initWithHolder:(twitchsw::WebViewImpl*)holder processPool:(WKProcessPool*)proccessPool;
- (void)loadView;
- (void)loadURL:(NSURL*)URL withOptions:(const twitchsw::HttpRequestOptions&)options;
@property (nonatomic, strong) WKWebView* webView;

@end

#endif  // TSW_MAC
