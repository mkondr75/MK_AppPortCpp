#include "../include/config.hpp"
#include <ws2tcpip.h>

extern PROCESS_INFORMATION nodeProcess;
PROCESS_INFORMATION nodeProcess = {}; // Теперь живёт здесь!
DWORD GetNodePID() { return nodeProcess.dwProcessId; }

// -------------------- wait for port --------------------
bool WaitForPort(int port, int timeoutMs) {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	DWORD start = GetTickCount();
	while (GetTickCount() - start < (DWORD)timeoutMs) {
		SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

		if (connect(sock, (sockaddr *)&addr, sizeof(addr)) == 0) {
			closesocket(sock);
			WSACleanup();
			return true;
		}

		closesocket(sock);
		Sleep(300); // важно
	}

	WSACleanup();
	return false;
}

// -------------------- node --------------------
void StartNode(Config &cfg) {
	STARTUPINFOW si = {sizeof(si)};
	// LPVOID lpEnvironment;

	// STARTUPINFOW si = {};
	// si.cb = sizeof(si);
	////////////////////////////////////////////////////////////
	// node logging
	////////////////////////////////////////////////////////////
	HANDLE node_log = INVALID_HANDLE_VALUE;
	SECURITY_ATTRIBUTES sa = {};
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = nullptr;
	sa.bInheritHandle = TRUE;
	////////////////////////////////////////////////////////////
	// redirect stdout/stderr to file
	////////////////////////////////////////////////////////////
	if (cfg.logs.node == LogMode::FILE_MODE) {
		cfg.logs.node_handle = CreateFileW(cfg.logs.node_file.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (cfg.logs.node_handle == INVALID_HANDLE_VALUE) {
			wprintf(L"ERROR CreateFileW node log: %ls\n", cfg.logs.node_file.c_str());
			return;
		}
		si.dwFlags |= STARTF_USESTDHANDLES;
		si.hStdOutput = cfg.logs.node_handle;
		si.hStdError = cfg.logs.node_handle;
	}

	// wchar_t cmd[] = L"\"C:\\Program Files\\nodejs\\node.exe\" --permission
	// --allow-fs-read=./* --allow-fs-write=./*
	// --allow-fs-read=C:\\dev_node\\trilium_dev
	// --allow-fs-write=C:\\dev_node\\trilium_dev --allow-child-process
	// --allow-addons -e \"import { fileURLToPath } from
	// 'url';console.log(fileURLToPath(import.meta.url));
	// console.log(process.env);
	// console.log(process.execArgv);process.stdin.resume();
	// process.stdin.on('data', ()=>process.exit(0));\"";
	// wchar_t cmd[] = L"\"node.exe\" --max-old-space-size=192 --max-semi-space-size=16 "
	// 				L"--expose-gc --permission --allow-fs-read=./* --allow-fs-write=./* "
	// 				L"--allow-fs-read=C:\\dev_node\\trilium_dev "
	// 				L"--allow-fs-write=C:\\dev_node\\trilium_dev --allow-child-process "
	// 				L"--allow-addons .\\apps\\server\\dist\\main.cjs";

	std::wstring cmd_buf = L"\"" + cfg.node.exe + L"\" " + cfg.node.args;
	std::vector<wchar_t> cmd_vector(cmd_buf.begin(), cmd_buf.end());
	cmd_vector.push_back(L'\0');
	LPWSTR cmd = cmd_vector.data();

	LPWCH base = GetEnvironmentStringsW();
	FreeEnvironmentStringsW(base);

	for (const auto &e : cfg.node.node_env) { SetEnvironmentVariableW(e.key.c_str(), e.value.c_str()); }

	// SetEnvironmentVariableW(L"APPDATA", L"C:\\dev_node\\trilium_dev\\profile\\AppData\\Roaming");
	// SetEnvironmentVariableW(L"ComSpec", L"C:\\WINDOWS\\system32\\cmd.exe");
	// SetEnvironmentVariableW(L"HOST", L"127.0.0.1");
	// SetEnvironmentVariableW(L"LOCALAPPDATA", L"C:\\dev_node\\trilium_dev\\profile\\AppData\\Local");
	// SetEnvironmentVariableW(L"PATH", L"C:\\msys64\\home\\maxko\\dev\\dev_node\\node_bin;C:\\WINDOWS\\system32");
	// SetEnvironmentVariableW(L"SYSTEMROOT", L"C:\\WINDOWS");
	// SetEnvironmentVariableW(L"TEMP", L"C:\\dev_node\\trilium_dev\\profile\\.tmp");
	// SetEnvironmentVariableW(L"TMP", L"C:\\dev_node\\trilium_dev\\profile\\.tmp");
	// SetEnvironmentVariableW(L"TRILIUM_DATA_DIR", L"C:\\dev_node\\trilium_dev\\data_sandbox");
	// SetEnvironmentVariableW(L"TRILIUM_PORT", L"9090");
	// SetEnvironmentVariableW(L"USERPROFILE", L"C:\\dev_node\\trilium_dev\\profile");

	// wchar_t sysroot[MAX_PATH];
	// GetEnvironmentVariableW(L"SystemRoot", sysroot, MAX_PATH);
	// wchar_t comspec[MAX_PATH];
	// GetEnvironmentVariableW(L"ComSpec", comspec, MAX_PATH);
	// wprintf(L"comspec=%ls\n", comspec);
	// wprintf(L"SystemRoot=%ls\n", sysroot);
	BOOL ok = false;
	if (cfg.logs.node == LogMode::FILE_MODE) {
		ok = CreateProcessW(NULL, cmd, NULL, NULL,
							1,				  //(cfg.logs.node == LogMode::FILE_MODE)
							CREATE_NO_WINDOW, // CREATE_NEW_CONSOLE,//
							NULL,			  // env,
							// L"C:\\dev_node\\trilium_dev", // ← рабочая папка
							cfg.node.working_dir.data(), &si, &nodeProcess);
	} else if (cfg.logs.node == LogMode::CONS) {
		ok = CreateProcessW(NULL, cmd, NULL, NULL,
							false,				//(cfg.logs.node == LogMode::FILE_MODE)
							CREATE_NEW_CONSOLE, // CREATE_NO_WINDOW,//
							NULL,				// env,
							// L"C:\\dev_node\\trilium_dev", // ← рабочая папка
							cfg.node.working_dir.data(), &si, &nodeProcess);
	} else {
		ok = CreateProcessW(NULL, cmd, NULL, NULL,
							false,			  //(cfg.logs.node == LogMode::FILE_MODE)
							CREATE_NO_WINDOW, // CREATE_NEW_CONSOLE,//
							NULL,			  // env,
							// L"C:\\dev_node\\trilium_dev", // ← рабочая папка
							cfg.node.working_dir.data(), &si, &nodeProcess);
	}

	if (!ok) {
		DWORD err = GetLastError();
		wchar_t buf[256];
		wsprintf(buf, L"Error: %lu", err);
		MessageBoxW(NULL, buf, L"CreateProcess", MB_OK);
		if (node_log != INVALID_HANDLE_VALUE) { CloseHandle(node_log); }
	}
	// else{
	// DWORD err = GetLastError();
	// wchar_t buf[256];
	// wsprintf(buf, L"Error: %lu", err);
	// MessageBoxW(NULL, buf, L"CreateProcess", MB_OK);
	// }
}

void StopNode() {
	if (nodeProcess.hProcess) {
		TerminateProcess(nodeProcess.hProcess, 0);
		CloseHandle(nodeProcess.hProcess);
		CloseHandle(nodeProcess.hThread);
	}
}
