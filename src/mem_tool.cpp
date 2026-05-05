#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
extern void LogLauncherInfo(const wchar_t *msg);
std::vector<DWORD> GetChildProcesses(DWORD parentPid) {
	std::vector<DWORD> result;

	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snap, &pe)) {
		do {
			if (pe.th32ParentProcessID == parentPid) {
				result.push_back(pe.th32ProcessID);

				auto sub = GetChildProcesses(pe.th32ProcessID);
				result.insert(result.end(), sub.begin(), sub.end());
			}
		} while (Process32Next(snap, &pe));
	}

	CloseHandle(snap);
	return result;
}
#include <psapi.h>

SIZE_T GetProcessMemoryMB(DWORD pid) {
	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!h) return 0;

	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(h, &pmc, sizeof(pmc))) {
		CloseHandle(h);
		return pmc.WorkingSetSize / (1024 * 1024);
	}

	CloseHandle(h);
	return 0;
}

std::wstring GetProcessName(DWORD pid) {
	HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if (!h) return L"";

	wchar_t buffer[MAX_PATH];
	GetModuleBaseNameW(h, NULL, buffer, MAX_PATH);

	CloseHandle(h);
	return buffer;
}

void PrintMemoryBreakdown() {
	DWORD self = GetCurrentProcessId();

	auto children = GetChildProcesses(self);
	children.push_back(self);

	SIZE_T total = 0, webview = 0, node = 0, other = 0;

	for (DWORD pid : children) {
		SIZE_T mem = GetProcessMemoryMB(pid);
		std::wstring name = GetProcessName(pid);

		total += mem;

		if (name == L"msedgewebview2.exe")
			webview += mem;
		else if (name.find(L"node") != std::wstring::npos)
			node += mem;
		else
			other += mem;
	}
	wchar_t line[4096] = {0};
	swprintf(line, 4096, L"Total: %llu MB", total);
	LogLauncherInfo(line);
	swprintf(line, 4096, L"WebView: %llu MB", webview);
	LogLauncherInfo(line);
	swprintf(line, 4096, L"Node: %llu MB", node);
	LogLauncherInfo(line);
	swprintf(line, 4096, L"Other: %llu MB", other);
	LogLauncherInfo(line);
}
