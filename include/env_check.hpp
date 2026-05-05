#pragma once

#include <windows.h>

#include <string>
#include <vector>

struct ProbeStatus {
  bool ok = false;
  std::wstring detected_value;
  std::wstring diagnostic_message;
};

struct DllProbeResult {
  std::wstring dll_name;
  bool exists = false;
};

struct DiskProbeResult {
  ULONGLONG free_bytes = 0;
  ULONGLONG total_bytes = 0;
};

struct ProbeResult {
  ProbeStatus node;
  ProbeStatus webview2;
  ProbeStatus writable;
  DiskProbeResult disk;
  std::vector<DllProbeResult> dlls;
};

bool ProbeNodeVersion(ProbeStatus &out_result);
bool ProbeNodeVersionCreateProcess(ProbeStatus &out_result);
bool ProbeWebView2Version(ProbeStatus &out_result);
bool ProbeDiskSpace(const std::wstring &path, DiskProbeResult &out_result);
bool ProbeRequiredDlls(const std::wstring &dir,
                       std::vector<DllProbeResult> &out_result);
void RunEnvironmentProbe(ProbeResult &result);
void PrintProbeResult(const ProbeResult &result);