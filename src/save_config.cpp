// C:\msys64\home\maxko\dev\AppPortCpp\src\save_config.cpp
#include "../include/save_config.hpp"
#include <cstdio>

// #include <fstream>
// #include <filesystem>

static const wchar_t *LogModeToString(LogMode mode)
{
    switch (mode)
    {
    case LogMode::OFF:
        return L"OFF";
    case LogMode::CONS:
        return L"CONS";
    case LogMode::FILE_MODE:
        return L"FILE_MODE";
    }
    return L"UNKNOWN";
}

bool SaveConfigJson(const Config &cfg)
{
    HANDLE file = CreateFileW(
        cfg.paths.config_file.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (file == INVALID_HANDLE_VALUE)
        return false;

    wchar_t json[8192];

    int len = swprintf_s(
        json,
        L"{\n"
        L"  \"paths\": {\n"
        L"    \"root_dir\": \"%ls\",\n"
        L"    \"bin_dir\": \"%ls\",\n"
        L"    \"logs_dir\": \"%ls\",\n"
        L"    \"config_file\": \"%ls\",\n"
        L"    \"node\": \"%ls\"\n"
        L"  },\n"
        L"  \"logs\": {\n"
        L"    \"launcher\": \"%ls\",\n"
        L"    \"node\": \"%ls\",\n"
        L"    \"webview\": \"%ls\"\n"
        L"  },\n"
        L"  \"node\": {\n"
        L"    \"exe\": \"%ls\",\n"
        L"    \"working_dir\": \"%ls\",\n"
        L"    \"entry_script\": \"%ls\",\n"
        L"    \"args\": \"%ls\"\n"
        L"  },\n"
        L"  \"edge\": {\n"
        L"    \"use_system_webview2\": %ls,\n"
        L"    \"user_data_dir\": \"%ls\"\n"
        L"  }\n"
        L"}\n",
        cfg.paths.root_dir.c_str(),
        cfg.paths.bin_dir.c_str(),
        cfg.paths.logs_dir.c_str(),
        cfg.paths.config_file.c_str(),
        cfg.paths.node.c_str(),
        LogModeToString(cfg.logs.launcher),
        LogModeToString(cfg.logs.node),
        LogModeToString(cfg.logs.webview),
        cfg.node.exe.c_str(),
        cfg.node.working_dir.c_str(),
        cfg.node.entry_script.c_str(),
        cfg.node.args.c_str(),
        cfg.edge.use_system_webview2 ? L"true" : L"false",
        cfg.edge.user_data_dir.c_str());

    if (len <= 0)
    {
        CloseHandle(file);
        return false;
    }

    DWORD written = 0;

    BOOL ok = WriteFile(
        file,
        json,
        len * sizeof(wchar_t),
        &written,
        NULL);

    CloseHandle(file);

    return ok == TRUE;
}

/*
bool SaveConfigJson(const Config &cfg)
{
    std::wstringstream json;
    json << L"{\n";
    json << L"  \"paths\": {\n";
    json << L"    \"root_dir\": \"" << cfg.paths.root_dir << L"\",\n";
    json << L"    \"bin_dir\": \"" << cfg.paths.bin_dir << L"\",\n";
    json << L"    \"logs_dir\": \"" << cfg.paths.logs_dir << L"\",\n";
    json << L"    \"config_file\": \"" << cfg.paths.config_file << L"\",\n";
    json << L"    \"node\": \"" << cfg.paths.node << L"\"\n";
    json << L"  },\n";
    json << L"  \"logs\": {\n";
    json << L"    \"launcher\": \"" << LogModeToString(cfg.logs.launcher) << L"\",\n";
    json << L"    \"node\": \"" << LogModeToString(cfg.logs.node) << L"\",\n";
    json << L"    \"webview\": \"" << LogModeToString(cfg.logs.webview) << L"\"\n";
    json << L"  },\n";
    json << L"  \"node\": {\n";
    json << L"    \"exe\": \"" << cfg.node.exe << L"\",\n";
    json << L"    \"working_dir\": \"" << cfg.node.working_dir << L"\",\n";
    json << L"    \"entry_script\": \"" << cfg.node.entry_script << L"\",\n";
    json << L"    \"args\": \"" << cfg.node.args << L"\"\n";
    json << L"  },\n";
    json << L"  \"edge\": {\n";
    json << L"    \"use_system_webview2\": " << (cfg.edge.use_system_webview2 ? L"true" : L"false") << L",\n";
    json << L"    \"user_data_dir\": \"" << cfg.edge.user_data_dir << L"\"\n";
    json << L"  }\n";
    json << L"}\n";
    // std::wofstream file(cfg.paths.config_file, std::ios::trunc);
    std::wofstream file(std::filesystem::path(cfg.paths.config_file), std::ios::trunc);
    if (!file.is_open())
        return false;
    file << json.str();
    file.close();
    return true;
}
*/