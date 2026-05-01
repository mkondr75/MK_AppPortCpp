// logsystem.cpp

#include "../include/logs.hpp"
#include <string>
#include <windows.h>

// ============================================================
// globals
// ============================================================
static HANDLE g_launcher_log = INVALID_HANDLE_VALUE;
static LogMode g_launcher_mode = LogMode::OFF;
// ============================================================
// helpers
// ============================================================
static std::wstring GetIsoTimestampUTC() {
  SYSTEMTIME st;
  GetSystemTime(&st);
  wchar_t buffer[64] = {0};
  // launcher_2026-04-26T11:13:04.674Z.log
  swprintf(buffer, 64, L"%04d-%02d-%02d-%02d-%02d-%02d-%03d", st.wYear,
           st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
           st.wMilliseconds);
  return std::wstring(buffer);
}

static bool WriteWideLineToHandle(HANDLE h, const std::wstring &text) {
  if (h == INVALID_HANDLE_VALUE) {
    return false;
  }
  int utf8_size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0,
                                      nullptr, nullptr);
  if (utf8_size <= 0) {
    return false;
  }
  std::string utf8;
  utf8.resize(utf8_size - 1);
  WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, utf8.data(), utf8_size,
                      nullptr, nullptr);
  DWORD written = 0;
  BOOL ok = WriteFile(h, utf8.c_str(), (DWORD)utf8.size(), &written, nullptr);
  return ok == TRUE;
}

static void WriteLauncherLog(const wchar_t *level, const wchar_t *msg) {
  if (g_launcher_mode == LogMode::OFF) {
    return;
  }
  std::wstring ts = GetIsoTimestampUTC();
  wchar_t line[4096] = {0};
  swprintf(line, 4096,
           L"{\"ts\":\"%ls\",\"src\":\"launcher\",\"lvl\":\"%ls\",\"msg\":\"%"
           L"ls\"}\n",
           ts.c_str(), level, msg);
  // ========================================================
  // console
  // ========================================================
  if (g_launcher_mode == LogMode::CONS) {
    wprintf(L"%ls", line);
    return;
  }
  // ========================================================
  // file
  // ========================================================
  if (g_launcher_mode == LogMode::FILE_MODE) {
    WriteWideLineToHandle(g_launcher_log, line);
    return;
  }
}

// ============================================================
// public api
// ============================================================
bool InitLogging(LogConfig &cfg) { // const
  g_launcher_mode = cfg.launcher;
  std::wstring ts = GetIsoTimestampUTC();
  cfg.session_datetime = ts;
  // cfg.logs.launcher_file = L"logs\\launcher_test.log";
  cfg.launcher_file = L"logs\\launcher_" + cfg.session_datetime + L".log";
  cfg.node_file = L"logs\\node_" + cfg.session_datetime + L".log";
  cfg.webview_file = L"logs\\webview_" + cfg.session_datetime + L".log";
  cfg.crash_file = L"logs\\crash_" + cfg.session_datetime + L".log";
  wprintf(L"logfile_launcher=%ls\n", cfg.launcher_file.c_str());

  // ========================================================
  // OFF
  // ========================================================
  if (g_launcher_mode == LogMode::OFF) {
    return true;
  }
  // ========================================================
  // FILE_MODE
  // ========================================================
  if (g_launcher_mode == LogMode::FILE_MODE) {
    CreateDirectoryW(L"logs", nullptr);
    g_launcher_log = CreateFileW(cfg.launcher_file.c_str(), FILE_APPEND_DATA,
                                 FILE_SHARE_READ, nullptr, OPEN_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL, nullptr);
    if (g_launcher_log == INVALID_HANDLE_VALUE) {
      wprintf(L"ErroCreating logfile_launcher=%ls\n",
              cfg.launcher_file.c_str());
      return false;
    } else {
      wprintf(L"file was created logfile_launcher=%ls\n",
              cfg.launcher_file.c_str());
    }
  }
  LogLauncherInfo(L"logging initialized");
  return true;
}

void LogLauncherInfo(const wchar_t *msg) { WriteLauncherLog(L"info", msg); }
void LogLauncherError(const wchar_t *msg) { WriteLauncherLog(L"error", msg); }