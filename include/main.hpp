#include <winsock2.h>   // ВАЖНО: раньше windows.h
#include <windows.h>
#include <ws2tcpip.h>
#define INITGUID
#include <guiddef.h>
#include <stdio.h>
//#include <iostream> - not used!!!
//#include <vector>
//#include <string>
//#include <algorithm>
//#include <wrl.h>
//#include "WebView2EnvironmentOptions.h"
#include <psapi.h>
#include <tlhelp32.h>
#include "WebView2.h"

///////// My include 
#include "config.hpp"
#include "wk_flags.hpp" 
#include "node_rt.hpp" 
#include "edge_rt.hpp"
