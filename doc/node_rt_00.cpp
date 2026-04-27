#include <windows.h>
#include <cstdio>
#include "../include/config.hpp"
// ============================================================
// globals
// ============================================================
static PROCESS_INFORMATION g_node_process = {};
// ============================================================
// start node
// ============================================================
void StartNode(const Config& cfg) {
    ////////////////////////////////////////////////////////////
    // startup info
    ////////////////////////////////////////////////////////////
    STARTUPINFOW si = {};
    si.cb = sizeof(si);
    ////////////////////////////////////////////////////////////
    // node logging
    ////////////////////////////////////////////////////////////
    HANDLE node_log = INVALID_HANDLE_VALUE;
    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;
    ////////////////////////////////////////////////////////////
    // redirect stdout/stderr to file
    ////////////////////////////////////////////////////////////
    if (cfg.logs.node == LogMode::FILE_MODE) {
        node_log = CreateFileW(
            cfg.logs.node_file.c_str(),
            FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            &sa,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        if (node_log == INVALID_HANDLE_VALUE) {
            wprintf(L"ERROR CreateFileW node log: %ls\n",
                cfg.logs.node_file.c_str());
            return;
        }
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = node_log;
        si.hStdError = node_log;
    }
    ////////////////////////////////////////////////////////////
    // command line
    ////////////////////////////////////////////////////////////
    wchar_t cmd[2048] = {0};
    swprintf(cmd,2048,L"node.exe server.js");
    ////////////////////////////////////////////////////////////
    // create process
    ////////////////////////////////////////////////////////////
    BOOL ok = CreateProcessW(
		nullptr,
        cmd,
        nullptr,
        nullptr,
        // inherit handles
        (cfg.logs.node == LogMode::FILE_MODE),
        CREATE_NEW_CONSOLE,
        nullptr,
        // working dir
        L"C:\\dev_node\\trilium_dev",
        &si,
        &g_node_process
    );
    ////////////////////////////////////////////////////////////
    // result
    ////////////////////////////////////////////////////////////
    if (!ok) {
        DWORD err = GetLastError();
        wprintf(L"CreateProcessW failed err=%lu\n", err);
        if (node_log != INVALID_HANDLE_VALUE) {
            CloseHandle(node_log);
        }
        return;
    }
    ////////////////////////////////////////////////////////////
    // success
    ////////////////////////////////////////////////////////////
    wprintf(L"Node started PID=%lu\n", g_node_process.dwProcessId);
    ////////////////////////////////////////////////////////////
    // cleanup local handles
    ////////////////////////////////////////////////////////////
    if (node_log != INVALID_HANDLE_VALUE) {
        CloseHandle(node_log);
    }
    CloseHandle(g_node_process.hThread);
}