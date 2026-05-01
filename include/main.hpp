#include <windows.h>
#include <winsock2.h> // ВАЖНО: раньше windows.h
#include <ws2tcpip.h>

#define INITGUID
#include <guiddef.h>
// #include <stdio.h>
// #include <iostream> - not used!!!
// #include <vector>
// #include <string>
// #include <algorithm>
// #include <wrl.h>
// #include "WebView2EnvironmentOptions.h"
#include "WebView2.h"
#include <psapi.h>
#include <tlhelp32.h>


///////// My include
#include "config.hpp"
#include "edge_rt.hpp"
#include "env_check.hpp"
#include "init_config.hpp"
#include "load_config.hpp"
#include "node_question.hpp"
#include "node_rt.hpp"
#include "save_config.hpp"
#include "wk_flags.hpp"

