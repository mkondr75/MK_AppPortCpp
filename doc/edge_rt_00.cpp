// edge_rt.cpp
#include "../include/edge_rt.hpp"
#include <windows.h>
#include "WebView2.h"
// ============================================================
// globals
// ============================================================
static const wchar_t CLASS_NAME[] = L"AppGui";
static HWND g_hwnd = nullptr;
// ============================================================
// window proc
// ============================================================
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd,uMsg,wParam,lParam);
}
// ============================================================
// start edge/webview2
// ============================================================
bool StartEdge(HINSTANCE hInstance,int nCmdShow)
{
    ////////////////////////////////////////////////////////////
    // window class
    ////////////////////////////////////////////////////////////
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    ////////////////////////////////////////////////////////////
    // register class
    ////////////////////////////////////////////////////////////
    RegisterClassW(&wc);
    ////////////////////////////////////////////////////////////
    // edge flags
    ////////////////////////////////////////////////////////////
    SetEnvironmentVariableW(
        L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS",
        L"--disable-features=LayoutNGPrinting,Printing "
        L"--js-flags=\"--max-semi-space-size=8\""
    );
    ////////////////////////////////////////////////////////////
    // create window
    ////////////////////////////////////////////////////////////
    g_hwnd = CreateWindowExW(0, CLASS_NAME,
        L"AppGui",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1200,
        800,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    ////////////////////////////////////////////////////////////
    // create failed
    ////////////////////////////////////////////////////////////
    if (!g_hwnd) {
        wprintf(L"CreateWindowExW failed\n");
        return false;
    }
    ////////////////////////////////////////////////////////////
    // show window
    ////////////////////////////////////////////////////////////
    ShowWindow(g_hwnd, nCmdShow);
    ////////////////////////////////////////////////////////////
    // create webview2 env
    ////////////////////////////////////////////////////////////
    HRESULT hr =
        CreateCoreWebView2EnvironmentWithOptions(
            nullptr,
            nullptr,
            nullptr,
            new EnvHandler()
        );
    ////////////////////////////////////////////////////////////
    // failed
    ////////////////////////////////////////////////////////////
    if (FAILED(hr)) {
        wprintf(L"CreateCoreWebView2EnvironmentWithOptions failed\n");
        return false;
    }
    return true;
}