#pragma once

#include <windows.h>

#include <string>

struct NodeQuestion
{
    bool system_variant_available = false;
    std::wstring system_node_path;
    std::wstring system_node_version;
    bool use_system_variant = true;
    std::wstring custom_node_path;
    bool accepted = false;
};

bool ShowNodeQuestion(HINSTANCE hInstance, NodeQuestion &model);