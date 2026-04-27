#pragma once

#include <string>

enum class EdgeMode {
    OFF,
    NORMAL
};

struct EdgeConfig {

    EdgeMode mode = EdgeMode::NORMAL;

    bool devtools = false;

    bool verbose_logging = false;
};