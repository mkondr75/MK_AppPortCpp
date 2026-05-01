/*
#include <windows.h>
#include <shellapi.h>
#include <cwchar>
#include <cstdio>

void ParseFlags()
{
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(
        GetCommandLineW(),
        &argc
    );
    for (int i = 0; i < argc; i++) {
        if (wcscmp(argv[i], L"--debug") == 0) {
            AllocConsole();
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
        }
    }
    LocalFree(argv);
}
*/
#include "..\include\wk_flags.hpp"

Config ParseFlags(int argc, wchar_t *argv[])
{
    Config cfg;
    cfg.logs.launcher = LogMode::OFF;
    cfg.logs.node = LogMode::OFF;
    cfg.logs.webview = LogMode::OFF;

    for (int i = 1; i < argc; ++i)
    {
        if (wcscmp(argv[i], L"--log-launcher=cons") == 0)
        {
            cfg.logs.launcher = LogMode::CONS;
        }
        else if (wcscmp(argv[i], L"--log-launcher=file") == 0)
        {
            cfg.logs.launcher = LogMode::FILE_MODE;
        }
        else if (wcscmp(argv[i], L"--log-node=cons") == 0)
        {
            cfg.logs.node = LogMode::CONS;
        }
        else if (wcscmp(argv[i], L"--log-node=file") == 0)
        {
            cfg.logs.node = LogMode::FILE_MODE;
        }
        else if (wcscmp(argv[i], L"--setup") == 0)
        {
            cfg.paths.setup = L"setup";
        }
    }
    return cfg;
}