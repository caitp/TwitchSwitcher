// Copyright (C) 2016 Caitlin Potter & Contributors. All rights reserved.
// Use of this source is governed by the Apache License, Version 2.0 that
// can be found in the LICENSE file, which must be distributed with this
// software.

#if TSW_MAC

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
    LOG(LOG_INFO, "WebViewController: loadView");
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
    LOG(LOG_INFO, "WebViewController: viewDidLoad");
    [super viewDidLoad];
}

- (void)viewDidAppear
{
    LOG(LOG_INFO, "WebViewController: viewDidAppear");
    NSWindow* window = self.view.window;
    if (window == nil)
        LOG(LOG_INFO, " >>> view.window is nil");
    else if (window.visible == NO)
        LOG(LOG_INFO, " >>> view.window is not visible!");
    else {
        CGRect rect = window.frame;
        std::string rectStr = [NSStringFromRect(rect) UTF8String];
        LOG(LOG_INFO, " >>> view.window.frame == %s", rectStr.c_str());
    }
}

- (void)viewDidDisappear
{
    LOG(LOG_INFO, "WebViewController: viewDidDisappear");
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
    LOG(LOG_INFO, "WebViewController: webView:%p didCommitNavigation:%p", webView, navigation);
}


- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error
{
    std::string url = [webView.URL.absoluteString UTF8String];
    std::string description = [error.localizedDescription UTF8String];
    std::string reason = [error.localizedFailureReason UTF8String];

    std::string details = description;
    if (reason.length())
        details += " (" + reason + ")";

    LOG(LOG_INFO, "Failed to load `%s`: %s", url.c_str(), details.c_str());
    auto impl = self.holder;
    if (impl) {
        auto onComplete = impl->onComplete();
        if (onComplete)
            onComplete();
    }
}

- (void)webView:(WKWebView *)webView didFailProvisionalNavigation:(WKNavigation *)navigation withError:(NSError *)error
{
    std::string description = [error.localizedDescription UTF8String];
    std::string reason = [error.localizedFailureReason UTF8String];
    std::string details = description;
    if (reason.length())
        details += " (" + reason + ")";

    LOG(LOG_INFO, "WebVewController: webView:%p didFailNavigation:%p withError:%s", webView, navigation, details.c_str());
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
    std::string url = [webView.URL.absoluteString UTF8String];

    LOG(LOG_INFO, "Finished loading `%s`", url.c_str());
    auto impl = self.holder;
    if (impl) {
        auto onComplete = impl->onComplete();
        if (onComplete)
            onComplete();
    }
}

- (void)webView:(WKWebView *)webView didReceiveAuthenticationChallenge:(NSURLAuthenticationChallenge *)challenge completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition disposition, NSURLCredential *credential))completionHandler
{
    LOG(LOG_INFO, "WebViewController: webView:%p didReceiveAuthenticationChallenge:%p", webView, challenge);
    completionHandler(NSURLSessionAuthChallengePerformDefaultHandling, nil);
}

- (void)webView:(WKWebView *)webView didReceiveServerRedirectForProvisionalNavigation:(WKNavigation *)navigation
{
    std::string url = [webView.URL.absoluteString UTF8String];
    LOG(LOG_INFO, "Requested redirect %s", url.c_str());
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
    LOG(LOG_INFO, "WebViewController: webView:%p didStartProvisionalNavigation:%p", webView, navigation);
}

- (void)webViewWebContentProcessDidTerminate:(WKWebView *)webView
{
    LOG(LOG_INFO, "WebViewController: webViewWebContentProcessDidTerminate:%p", webView);
}

@end

#endif  // TSW_MAC
