#include "../include/env_check.hpp"

extern void LogLauncherInfo(const wchar_t *msg);
extern void LogLauncherError(const wchar_t *msg);

static std::wstring Utf8ToWide(const std::string &str) {
	if (str.empty()) return L"";

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);

	std::wstring result(size_needed, 0);

	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], size_needed);

	return result;
}

bool ProbeNodeVersionCreateProcess(ProbeStatus &out_result) {
	SECURITY_ATTRIBUTES sa{};
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	HANDLE read_pipe = NULL;
	HANDLE write_pipe = NULL;
	if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) {
		out_result.ok = false;
		out_result.diagnostic_message = L"CreatePipe failed";
		return false;
	}

	SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
	STARTUPINFOW si{};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = write_pipe;
	si.hStdError = write_pipe;
	PROCESS_INFORMATION pi{};
	wchar_t cmd[] = L"node.exe --version";
	BOOL ok = CreateProcessW(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
	CloseHandle(write_pipe);
	if (!ok) {
		DWORD err = GetLastError();
		out_result.ok = false;
		out_result.diagnostic_message = L"CreateProcessW failed";
		wchar_t errbuf[64];
		swprintf_s(errbuf, L"GetLastError=%lu", err);
		out_result.detected_value = errbuf;
		CloseHandle(read_pipe);
		return false;
	}
	std::string output;
	char buffer[256];
	DWORD readed = 0;
	while (ReadFile(read_pipe, buffer, sizeof(buffer) - 1, &readed, NULL) && readed > 0) {
		buffer[readed] = 0;
		output += buffer;
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exit_code = 0;
	GetExitCodeProcess(pi.hProcess, &exit_code);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	CloseHandle(read_pipe);
	std::wstring woutput = Utf8ToWide(output);
	while (!woutput.empty() && (woutput.back() == L'\n' || woutput.back() == L'\r')) { woutput.pop_back(); }
	out_result.detected_value = woutput;
	if (exit_code == 0) {
		out_result.ok = true;
		out_result.diagnostic_message = L"node process executed";
		return true;
	}
	out_result.ok = false;
	out_result.diagnostic_message = L"node process returned error";
	return false;
}

bool ProbeWebView2Version(ProbeStatus &out_result) {
	// Локальная загрузка для автономности чекера
	HMODULE hCom = LoadLibraryW(L"combase.dll");
	HMODULE hLoader = GetModuleHandleW(L"WebView2Loader.dll");
	if (!hLoader) hLoader = LoadLibraryW(L"WebView2Loader.dll");
	if (!hCom || !hLoader) {
		out_result.ok = false;
		out_result.diagnostic_message = L"AA_ERR: OLE or Loader.dll missing";
		if (hCom) FreeLibrary(hCom);
		return false;
	}
	auto pInit = (HRESULT(WINAPI *)(LPVOID, DWORD))GetProcAddress(hCom, "CoInitializeEx");
	auto pUninit = (void(WINAPI *)(void))GetProcAddress(hCom, "CoUninitialize");
	auto pFree = (void(WINAPI *)(LPVOID))GetProcAddress(hCom, "CoTaskMemFree");
	auto pGetVer = (HRESULT(WINAPI *)(LPCWSTR, LPWSTR *))GetProcAddress(hLoader, "GetAvailableCoreWebView2BrowserVersionString");
	if (!pInit || !pUninit || !pGetVer) {
		out_result.ok = false;
		out_result.diagnostic_message = L"AA_ERR: Symbols not found";
		FreeLibrary(hCom);
		return false;
	}
	HRESULT hr = pInit(nullptr, 0x2); // COINIT_APARTMENTTHREADED
	LPWSTR version = nullptr;
	hr = pGetVer(nullptr, &version);
	if (SUCCEEDED(hr) && version) {
		out_result.ok = true;
		out_result.detected_value = version;
		out_result.diagnostic_message = L"WebView2 OK";
		if (pFree) pFree(version);
	} else {
		out_result.ok = false;
		out_result.diagnostic_message = L"WebView2 Runtime not found";
	}
	pUninit();

	if (hLoader) {
		FreeLibrary(hLoader); // Теперь чекер не оставляет следов в RAM
	}
	if (hCom) { FreeLibrary(hCom); }
	return out_result.ok;
}

bool ProbeDiskSpace(const std::wstring &path, DiskProbeResult &out_result) {
	ULARGE_INTEGER free_bytes_available{};
	ULARGE_INTEGER total_number_of_bytes{};
	ULARGE_INTEGER total_number_of_free_bytes{};
	BOOL ok = GetDiskFreeSpaceExW(path.c_str(), &free_bytes_available, &total_number_of_bytes, &total_number_of_free_bytes);
	if (!ok) return false;
	out_result.free_bytes = total_number_of_free_bytes.QuadPart;
	out_result.total_bytes = total_number_of_bytes.QuadPart;
	return true;
}

bool ProbeRequiredDlls(const std::wstring &dir, std::vector<DllProbeResult> &out_result) {
	static const wchar_t *required_dlls[] = {L"libwinpthread-1.dll", L"libgcc_s_seh-1.dll", L"libstdc++-6.dll", L"WebView2Loader.dll"};
	bool all_ok = true;
	for (const auto *dll : required_dlls) {
		DllProbeResult item;
		item.dll_name = dll;
		std::wstring full_path = dir + L"\\" + dll;
		DWORD attrs = GetFileAttributesW(full_path.c_str());
		item.exists = (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
		if (!item.exists) all_ok = false;
		out_result.push_back(item);
	}
	return all_ok;
}

void RunEnvironmentProbe(ProbeResult &result) {
	ProbeNodeVersionCreateProcess(result.node);

	ProbeWebView2Version(result.webview2);

	ProbeDiskSpace(L".", result.disk);

	ProbeRequiredDlls(L".", result.dlls);
}
void PrintProbeResult(const ProbeResult &result) {
	LogLauncherInfo(L"=== ENV PROBE BEGIN ===");
	{
		std::wstring msg = L"probe.node"
						   L" ok=" +
						   std::wstring(result.node.ok ? L"true" : L"false") + L" value=\"" + result.node.detected_value + L"\" diagnostic=\"" + result.node.diagnostic_message + L"\"";
		if (result.node.ok)
			LogLauncherInfo(msg.c_str());
		else
			LogLauncherError(msg.c_str());
	}
	{
		std::wstring msg = L"probe.webview2"
						   L" ok=" +
						   std::wstring(result.webview2.ok ? L"true" : L"false") + L" value=\"" + result.webview2.detected_value + L"\" diagnostic=\"" + result.webview2.diagnostic_message + L"\"";

		if (result.webview2.ok)
			LogLauncherInfo(msg.c_str());
		else
			LogLauncherError(msg.c_str());
	}
	{
		double free_gb = (double)result.disk.free_bytes / 1024.0 / 1024.0 / 1024.0;
		double total_gb = (double)result.disk.total_bytes / 1024.0 / 1024.0 / 1024.0;
		wchar_t buffer[256];
		swprintf_s(buffer, L"probe.disk free_gb=%.3f total_gb=%.3f", free_gb, total_gb);
		LogLauncherInfo(buffer);
	}
	for (const auto &dll : result.dlls) {
		std::wstring msg = L"probe.dll"
						   L" name=\"" +
						   dll.dll_name + L"\" exists=" + std::wstring(dll.exists ? L"true" : L"false");
		if (dll.exists)
			LogLauncherInfo(msg.c_str());
		else
			LogLauncherError(msg.c_str());
	}
	LogLauncherInfo(L"=== ENV PROBE END ===");
}
