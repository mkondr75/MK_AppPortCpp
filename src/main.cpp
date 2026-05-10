/// \file main.cpp
/// \brief Оркестратор запуска: Config -> Node -> Edge -> MessageLoop
#include "../include/config.hpp"
#include "../include/edge_rt.hpp"
#include "../include/node_rt.hpp"
#include "../include/env_check.hpp"
#include "../include/mem_tool.hpp"
#include "../include/node_question.hpp"
#include "../include/wk_flags.hpp"
#include "../include/init_config.hpp"
#include "../include/save_config.hpp"
#include "../include/load_config.hpp"
#include <windows.h>

Config cfg;

void First_slide(HINSTANCE hInstance) {
	NodeQuestion q;
	q.system_variant_available = true;
	q.system_node_path = L"c:\\program files\\nodejs\\node.exe";
	q.system_node_version = L"v24.13.1";
	q.custom_node_path = L"c:\\custom\\node.exe";

	if (ShowNodeQuestion(hInstance, q)) {
		LogLauncherInfo(q.use_system_variant ? q.system_node_path.c_str() : q.custom_node_path.c_str());
	} else {
		LogLauncherError(L"user cancelled node selection");
	}
}

// VOID CALLBACK MemTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
// 	LogLauncherInfo(L"launcher Memory usage tick (60s)");
// 	PrintMemoryBreakdown();
// }
// 1. Создай функцию-приемник JS
void OnJSMessage(const std::wstring &msg) { LogLauncherInfo((L"MESSAGE_FROM_JS: " + msg).c_str()); }

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	// 1. Инициализация COM (Скрытая внутри Edge_RT)
	if (!InitStealthOLE()) {
		MessageBoxW(NULL, L"Failed to load OLE32 functions", L"Critical Error", MB_ICONERROR);
		return 1;
	}

	// 2. Парсинг CLI аргументов через позднее связывание
	int argc = 0;
	LPWSTR *argv = nullptr;
	HMODULE hKernel32 = LoadLibraryW(L"kernel32.dll");
	HMODULE hShell32 = LoadLibraryW(L"shell32.dll");
	if (hKernel32 && hShell32) {
		typedef LPWSTR(WINAPI * PFN_GetCommandLineW)(void);
		typedef LPWSTR *(WINAPI * PFN_CommandLineToArgvW)(LPCWSTR, int *);
		auto pGetCommandLineW = (PFN_GetCommandLineW)GetProcAddress(hKernel32, "GetCommandLineW");
		auto pCommandLineToArgvW = (PFN_CommandLineToArgvW)GetProcAddress(hShell32, "CommandLineToArgvW");
		if (pGetCommandLineW && pCommandLineToArgvW) { argv = pCommandLineToArgvW(pGetCommandLineW(), &argc); }
	}
	cfg = ParseFlags(argc, argv);
	if (argv) LocalFree(argv);
	if (hShell32) FreeLibrary(hShell32);
	if (hKernel32) FreeLibrary(hKernel32);

	// 3. Логирование
	cfg.logs.node = LogMode::OFF;
	if (cfg.logs.launcher == LogMode::CONS) {
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
	InitLogging(cfg.logs);
	LogLauncherInfo(L"launcher started");
	PrintMemoryBreakdown();

	// 4. Разведка окружения
	ProbeResult probe{};
	RunEnvironmentProbe(probe);
	PrintProbeResult(probe);

	// Тест конфигов
	Config cfg_tst;
	FillDefaultConfig(cfg_tst);
	SaveConfigJson(cfg_tst);
	Config cfg_tst2;
	cfg_tst2.paths.config_file = cfg_tst.paths.config_file;
	LoadConfigJson(cfg_tst2);

	// 5. GUI Setup & Node Start
	First_slide(hInstance);
	Sleep(1000);
	PrintMemoryBreakdown();

	StartNode(cfg);
	Sleep(1000);
	if (!WaitForPort(9090, 15000)) { MessageBoxW(NULL, L"Server did not start", L"Error", MB_OK); }
	Sleep(1000);
	PrintMemoryBreakdown();
	// 7. Тримминг памяти (агрессивный)
	// TrimMemory(GetCurrentProcess());
	TrimTree(GetCurrentProcessId());
	// TrimTree(GetNodePID());

	// 6. Инициализация UI (Edge_RT)
	EdgeRT edge(hInstance);
	if (!edge.Start(cfg, nCmdShow)) {
		LogLauncherError(L"Failed to start WebView2 window");
		return 1;
	}
	edge.SetMessageHandler(OnJSMessage); // Подключаем мост

	PrintMemoryBreakdown();

	// Ждем пару секунд, чтобы WebView2 успел поднять DOM ???
	// Sleep(60000); жопа вешаем всех

	// Передаем таймер окну WebView2 (для логов памяти по WM_TIMER)
	// SetTimer(edge.GetHWND(), 1, 60000, NULL);
	UINT_PTR memTimerId = SetTimer(NULL, 1000, 10000, NULL);
	// 8. Цикл сообщений
	// int tmp = 0;
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		// Ловим наш таймер потока до того, как он уйдет в Dispatch
		if (msg.message == WM_TIMER && msg.wParam == memTimerId) {
			LogLauncherInfo(L"launcher Memory usage tick (60s)");
			PrintMemoryBreakdown();
			// if (tmp == 0) edge.ExecuteJS(L"alert('Bridge C++ to JS is online!');");
			// tmp = 1;
			continue; // Пропускаем Translate/Dispatch, мы уже обработали
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 9. Очистка
	if (memTimerId) KillTimer(NULL, memTimerId);
	// 9. Очистка
	// KillTimer(edge.GetHWND(), 1);
	// if (memTimerId) KillTimer(NULL, memTimerId); обвал рама до гига???
	StopNode();
	CleanupStealthOLE();

	// Закрытие хэндлов логов
	if (cfg.logs.launcher_handle != INVALID_HANDLE_VALUE) CloseHandle(cfg.logs.launcher_handle);
	if (cfg.logs.node_handle != INVALID_HANDLE_VALUE) CloseHandle(cfg.logs.node_handle);
	if (cfg.logs.webview_handle != INVALID_HANDLE_VALUE) CloseHandle(cfg.logs.webview_handle);
	if (cfg.logs.crash_handle != INVALID_HANDLE_VALUE) CloseHandle(cfg.logs.crash_handle);

	return 0;
}