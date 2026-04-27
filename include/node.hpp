#pragma once

#include <string>

enum class NodeMode {
    OFF,
    NORMAL
};

struct NodeConfig {

    NodeMode mode = NodeMode::NORMAL;

    bool restart_on_crash = false;

    bool inherit_env = true;
};