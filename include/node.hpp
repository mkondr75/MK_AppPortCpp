#pragma once

#include <string>

enum class NodeMode
{
    OFF,
    NORMAL
};

struct NodeConfig
{
    NodeMode mode = NodeMode::NORMAL;
    bool restart_on_crash = false;
    bool inherit_env = true;
    std::wstring exe;          // cfg.node.exe = L"node.exe";
    std::wstring working_dir;  // cfg.node.working_dir = L"C:\\dev_node\\trilium_dev";
    std::wstring entry_script; // cfg.node.entry_script = L".\\apps\\server\\dist\\main.cjs";
    std::wstring args;         // cfg.node.args =
};