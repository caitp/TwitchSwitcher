// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#if TSW_MAC

#include <twitchsw/string.h>

#include "mac/webview-wkwebview.h"
#import "mac/WebViewController.h"

#import <WebKit/WebKit.h>

@interface WebViewController () <WKNavigationDelegate>
    @property twitchsw::WebViewImpl* holder;
    @property WKProcessPool* processPool;
@end

@implementation WebViewController
@synthesize webView = _webView;
@synthesize holder = _holder;
@synthesize processPool = _processPool;

- (void)dealloc
{
    LOG(LOG_INFO, "WebViewController::dealloc");
}

- (id)initWithHolder:(twitchsw::WebViewImpl*)holder processPool:(WKProcessPool*)pool
{
    if (self = [super initWithNibName:nil bundle:nil]) {
        self.holder = holder;
        self.processPool = pool;
        return self;
    }

    return nil;
}

- (void)loadView
{
    WKWebView* view = self.webView;
    if (view == nil) {
        CGRect frame { { 0, 0 }, { 400, 400 } };

        // TODO(caitp): Make this stuff configurable, especially the frame.
        WKPreferences* preferences = [[WKPreferences alloc] init];
        preferences.javaScriptEnabled = YES;
        preferences.javaScriptCanOpenWindowsAutomatically = NO;

        WKWebViewConfiguration* configuration = [[WKWebViewConfiguration alloc] init];
        configuration.applicationNameForUserAgent = @"TSW.TwitchSwitcher";
        configuration.preferences = preferences;
        configuration.processPool = self.processPool;
        view = [[WKWebView alloc] initWithFrame:frame configuration:configuration];
        view.navigationDelegate = self;
        self.webView = view;
    }
    self.view = view;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)viewDidAppear
{
}

- (void)viewDidDisappear
{
    auto impl = self.holder;
    if (impl)
        impl->didClose();
}

- (void)loadURL:(NSURL*)URL withOptions:(const twitchsw::HttpRequestOptions&)options
{

    NSMutableURLRequest* mutableRequest = [NSMutableURLRequest requestWithURL:URL];

    const std::map<std::string, std::string>& requestHeaders = options.headers();
    if (!requestHeaders.empty()) {
        for (auto pair : requestHeaders) {
            NSString* headerField = [NSString stringWithUTF8String:pair.first.c_str()];
            NSString* value = [NSString stringWithUTF8String:pair.second.c_str()];
            [mutableRequest setValue:value forHTTPHeaderField:headerField];
        }
    }

    [self.webView loadRequest:[mutableRequest copy]];
}

//
// WKNavigationDelegate
//
- (void)webView:(WKWebView *)webView didCommitNavigation:(WKNavigation *)navigation
{
}


- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error
{
    NSString* details = error.localizedDescription;
    if (error.localizedFailureReason && error.localizedFailureReason.length)
        details = [details stringByAppendingFormat:@" (%@)", error.localizedFailureReason];
    twitchsw::String url([webView.URL.absoluteString UTF8String]);
    twitchsw::String reason([details UTF8String]);

    LOG(LOG_INFO, "Failed to load `%s`: %s", url.characters(), reason.characters());
    auto impl = self.holder;
    if (impl) {
        if (!impl->hasWebView()) return;
        twitchsw::Ref<twitchsw::WebView> webView = impl->webView();
        auto onComplete = impl->onComplete();
        if (onComplete)
            onComplete(webView.get(), url);
    }
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation withError:(NSError *)error
{
    NSString* details = error.localizedDescription;
    if (error.localizedFailureReason && error.localizedFailureReason.length)
        details = [details stringByAppendingFormat:@" (%@)", error.localizedFailureReason];
    twitchsw::String reason([details UTF8String]);
    LOG(LOG_INFO, "WebVewController: webView:%p didFailNavigation:%p withError:%s", webView, navigation, reason.characters());
    auto impl = self.holder;
    if (impl) {
        if (!impl->hasWebView()) return;
        auto onComplete = impl->onComplete();
        if (onComplete) {
            twitchsw::Ref<twitchsw::WebView> webView = impl->webView();
            twitchsw::String url = [self.webView.URL.absoluteString UTF8String];
            onComplete(webView.get(), url);
        }
    }
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    auto impl = self.holder;
    if (impl) {
        if (!impl->hasWebView()) return;
        auto onComplete = impl->onComplete();
        if (onComplete) {
            twitchsw::Ref<twitchsw::WebView> webView = impl->webView();
            twitchsw::String url = [self.webView.URL.absoluteString UTF8String];
            LOG(LOG_INFO, "Finished navigation to %s", url.characters());
            onComplete(webView.get(), url);
        }
    }
}

- (void)webView:(WKWebView *)webView didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential *credential))completionHandler
{
    completionHandler(NSURLSessionAuthChallengePerformDefaultHandling, nil);
}

- (void)webView:(WKWebView *)webView didReceiveServerRedirectForProvisionalNavigation:(WKNavigation *)navigation
{
    std::string url = [webView.URL.absoluteString UTF8String];
    auto impl = self.holder;
    if (impl) {
        auto onRedirect = impl->onRedirect();
        if (onRedirect) {
            twitchsw::OnRedirect result = onRedirect(url, std::string());
            if (result != twitchsw::OnRedirect::Follow) {
                [webView stopLoading];
            }
        }
    }
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation
{
}

- (void)webViewWebContentProcessDidTerminate:(WKWebView *)webView
{
}

- (void)webView:(WKWebView *)webView decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler {
    NSURL *url = navigationAction.request.URL;
    NSString *hostname = @"localhost";
    if ([url.host isEqualToString:hostname] ) {
        decisionHandler(WKNavigationActionPolicyCancel);
        return;
    }
    decisionHandler(WKNavigationActionPolicyAllow);
}

@end

#endif  // TSW_MAC
