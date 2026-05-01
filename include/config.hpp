#pragma once

#include "runtime.hpp"
#include "logs.hpp"
#include "node.hpp"
#include "edge.hpp"

struct Config
{
    RuntimePaths paths;
    LogConfig logs;
    NodeConfig node;
    EdgeConfig edge;
};