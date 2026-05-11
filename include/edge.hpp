#pragma once
#include <string>
enum class EdgeMode { OFF, NORMAL };

struct EdgeConfig {
	EdgeMode mode = EdgeMode::NORMAL; //off - not run
	bool devtools = false;			  //depr
	bool verbose_logging = false;	  //??
	bool use_system_webview2;		  // defolt == true;
	std::wstring path;				  //  = L"";
	std::wstring user_data_dir;		  //  = L".\\edge_profile";
	std::wstring flags;				  // = L"--";
	std::wstring navigate;			  // = L"http://127.0.0.1:9090";
};