#pragma once

#include <string>

struct RuntimePaths
{
    std::wstring root_dir;    // c:\msys64\home\maxko\dev\AppPortCpp - база проэкта лаунчера
    std::wstring bin_dir;     // c:\msys64\home\maxko\dev\AppPortCpp\bin - база лаунчера
    std::wstring logs_dir;    // c:\msys64\home\maxko\dev\AppPortCpp\log - база loger лаунчера
    std::wstring config_file; // == bin_dir+ L"\\app.ini"; (defolt)
    std::wstring node;        // defolt L"\0";
    std::wstring setup;       // defolt L"\0";
    // std::wstring node_script; // целевое приложение - перехало в ноде.хпп
};