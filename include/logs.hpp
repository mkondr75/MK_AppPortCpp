// C:\msys64\home\maxko\dev\AppPortCpp\include\logs.hpp
#pragma once
#include <windows.h>
#include <string>

enum class LogMode { OFF, CONS, FILE_MODE };

struct LogConfig {
	LogMode launcher = LogMode::OFF;
	LogMode node = LogMode::OFF;
	LogMode webview = LogMode::OFF;
	std::wstring session_datetime;
	std::wstring launcher_file;
	std::wstring node_file;
	std::wstring webview_file;
	std::wstring crash_file;
	HANDLE launcher_handle = INVALID_HANDLE_VALUE;
	HANDLE node_handle = INVALID_HANDLE_VALUE;
	HANDLE webview_handle = INVALID_HANDLE_VALUE;
	HANDLE crash_handle = INVALID_HANDLE_VALUE;
};

bool InitLogging(LogConfig &cfg); // const
void LogLauncherInfo(const wchar_t *msg);
void LogLauncherError(const wchar_t *msg);