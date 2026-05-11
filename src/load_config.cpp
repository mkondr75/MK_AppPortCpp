// C:\msys64\home\maxko\dev\AppPortCpp\src\load_config.cpp
#include "..\include\load_config.hpp"

static std::wstring ReadFileText(const std::wstring &path) {
	HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) return L"";
	LARGE_INTEGER size;
	if (!GetFileSizeEx(file, &size)) {
		CloseHandle(file);
		return L"";
	}
	if (size.QuadPart <= 0) {
		CloseHandle(file);
		return L"";
	}
	std::wstring text;
	text.resize((size_t)(size.QuadPart / sizeof(wchar_t)));
	DWORD readed = 0;
	BOOL ok = ReadFile(file, text.data(), (DWORD)size.QuadPart, &readed, NULL);
	CloseHandle(file);
	if (!ok) return L"";
	text.resize(readed / sizeof(wchar_t));
	return text;
}

static std::wstring JsonReadString(const std::wstring &json, const std::wstring &key) {
	std::wstring pattern = L"\"" + key + L"\"";
	size_t key_pos = json.find(pattern);
	if (key_pos == std::wstring::npos) return L"";
	size_t colon_pos = json.find(L":", key_pos);
	if (colon_pos == std::wstring::npos) return L"";
	size_t first_quote = json.find(L"\"", colon_pos + 1);
	if (first_quote == std::wstring::npos) return L"";
	size_t second_quote = json.find(L"\"", first_quote + 1);
	if (second_quote == std::wstring::npos) return L"";
	return json.substr(first_quote + 1, second_quote - first_quote - 1);
}
void read_env(Config &cfg, const std::wstring &json) {
	cfg.node.node_env.clear();
	size_t pos = json.find(L"\"node_env\"");
	if (pos != std::wstring::npos) {
		size_t arr_begin = json.find(L"[", pos);
		size_t arr_end = json.find(L"]", arr_begin);
		if (arr_begin != std::wstring::npos && arr_end != std::wstring::npos) {
			std::wstring arr = json.substr(arr_begin + 1, arr_end - arr_begin - 1);
			size_t obj_pos = 0;
			while ((obj_pos = arr.find(L"{", obj_pos)) != std::wstring::npos) {
				size_t obj_end = arr.find(L"}", obj_pos);
				if (obj_end == std::wstring::npos) break;
				std::wstring obj = arr.substr(obj_pos, obj_end - obj_pos + 1);
				NodeEnvPair env;
				env.key = JsonReadString(obj, L"key");
				env.value = JsonReadString(obj, L"value");
				if (!env.key.empty()) { cfg.node.node_env.push_back(env); }
				obj_pos = obj_end + 1;
			}
		}
	}
}
static bool JsonReadBool(const std::wstring &json, const std::wstring &key) {
	std::wstring pattern = L"\"" + key + L"\"";
	size_t key_pos = json.find(pattern);
	if (key_pos == std::wstring::npos) return false;
	size_t colon_pos = json.find(L":", key_pos);
	if (colon_pos == std::wstring::npos) return false;
	size_t true_pos = json.find(L"true", colon_pos);
	if (true_pos != std::wstring::npos && true_pos - colon_pos < 16) return true;
	return false;
}

bool LoadConfigJson(Config &cfg) {
	std::wstring json = ReadFileText(cfg.paths.config_file);
	if (json.empty()) return false;
	cfg.paths.root_dir = JsonReadString(json, L"root_dir");
	cfg.paths.bin_dir = JsonReadString(json, L"bin_dir");
	cfg.paths.logs_dir = JsonReadString(json, L"logs_dir");
	cfg.paths.config_file = JsonReadString(json, L"config_file");
	cfg.paths.node = JsonReadString(json, L"node");
	cfg.node.exe = JsonReadString(json, L"exe");
	cfg.node.working_dir = JsonReadString(json, L"working_dir");
	cfg.node.entry_script = JsonReadString(json, L"entry_script");
	cfg.node.args = JsonReadString(json, L"args");
	read_env(cfg, json); // fill env for node
	cfg.edge.user_data_dir = JsonReadString(json, L"user_data_dir");
	cfg.edge.use_system_webview2 = JsonReadBool(json, L"use_system_webview2");
	return true;
}