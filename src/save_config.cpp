// C:\msys64\home\maxko\dev\AppPortCpp\src\save_config.cpp
#include "../include/save_config.hpp"

static const wchar_t *LogModeToString(LogMode mode) {
	switch (mode) {
	case LogMode::OFF:
		return L"OFF";
	case LogMode::CONS:
		return L"CONS";
	case LogMode::FILE_MODE:
		return L"FILE_MODE";
	}
	return L"UNKNOWN";
}

bool SaveConfigJson(const Config &cfg) {
	HANDLE file = CreateFileW(cfg.paths.config_file.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE) return false;
	std::wstring node_env_json;

	for (size_t i = 0; i < cfg.node.node_env.size(); ++i) {
		const auto &e = cfg.node.node_env[i];
		node_env_json += L"      {\n"
						 L"        \"key\": \"" +
						 e.key +
						 L"\",\n"
						 L"        \"value\": \"" +
						 e.value +
						 L"\"\n"
						 L"      }";
		if (i + 1 < cfg.node.node_env.size()) node_env_json += L",";
		node_env_json += L"\n";
	}
	// cfg.node.mode = NodeMode::NORMAL;

	wchar_t json[8192];
	int len = swprintf_s(json,
						 L"{\n"
						 L"  \"paths\": {\n"
						 L"    \"root_dir\": \"%ls\",\n"
						 L"    \"bin_dir\": \"%ls\",\n"
						 L"    \"logs_dir\": \"%ls\",\n"
						 L"    \"config_file\": \"%ls\",\n"
						 L"    \"node\": \"%ls\"\n"
						 L"  },\n"
						 L"  \"logs\": {\n"
						 L"    \"launcher\": \"%ls\",\n"
						 L"    \"node\": \"%ls\",\n"
						 L"    \"webview\": \"%ls\"\n"
						 L"  },\n"
						 L"  \"node\": {\n"
						 L"    \"exe\": \"%ls\",\n"
						 L"    \"working_dir\": \"%ls\",\n"
						 L"    \"entry_script\": \"%ls\",\n"
						 L"    \"args\": \"%ls\"\n"
						 //
						 L"    \"node_env\": [\n" //
						 L"%ls"					  //
						 L"    ]\n"				  //
						 L"  },\n"
						 L"  \"edge\": {\n"
						 L"    \"use_system_webview2\": %ls,\n"
						 L"    \"user_data_dir\": \"%ls\"\n"
						 L"  }\n"
						 L"}\n",
						 cfg.paths.root_dir.c_str(), cfg.paths.bin_dir.c_str(), cfg.paths.logs_dir.c_str(), cfg.paths.config_file.c_str(), cfg.paths.node.c_str(), LogModeToString(cfg.logs.launcher),
						 LogModeToString(cfg.logs.node), LogModeToString(cfg.logs.webview), cfg.node.exe.c_str(), cfg.node.working_dir.c_str(), cfg.node.entry_script.c_str(), cfg.node.args.c_str(),
						 node_env_json.c_str(), cfg.edge.use_system_webview2 ? L"true" : L"false", cfg.edge.user_data_dir.c_str());
	if (len <= 0) {
		CloseHandle(file);
		return false;
	}

	DWORD written = 0;

	BOOL ok = WriteFile(file, json, len * sizeof(wchar_t), &written, NULL);

	CloseHandle(file);

	return ok == TRUE;
}
