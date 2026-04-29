// C:\msys64\home\maxko\dev\AppPortCpp\src\save_config.cpp
#include "../include/save_config.hpp"

// bool SaveConfigJson(const Config &cfg){}
// #include "save_config.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>

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