#pragma once

#include <string>

enum class EdgeMode
{
    OFF,
    NORMAL
};

struct EdgeConfig
{
    EdgeMode mode = EdgeMode::NORMAL;
    bool devtools = false;
    bool verbose_logging = false;
    bool use_system_webview2;   // cfg.edge.use_system_webview2 = true;
    std::wstring user_data_dir; // cfg.edge.user_data_dir = L".\\edge_profile";
};