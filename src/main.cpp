/// \file main.cpp
/// \brief место входа в проэкт .
/// \see main.hpp
#include "../include/main.hpp"
// #include <commctrl.h>
// #include <dwmapi.h>
#include <guiddef.h> // Обязательно для IsEqualGUID
#include <windows.h>

// -------------------- globals --------------------
HWND hwnd = nullptr;
ICoreWebView2Controller *controller = nullptr;
ICoreWebView2 *webview = nullptr;
Config cfg;
PROCESS_INFORMATION nodeProcess = {};

// 1. Ручная реализация ICoreWebView2EnvironmentOptions
constexpr GUID HARDCODED_IID_EnvOptions = {0x2fde08a8, 0x1e9a, 0x4766, {0x8c, 0x05, 0x95, 0xa9, 0xce, 0xb9, 0xd1, 0xc5}};
// 1. Базовый интерфейс всех COM-объектов (IUnknown)
constexpr GUID HARDCODED_IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};
// 1. ручная реализация ICoreWebView2EnvironmentOptions
class AppEnvOptions : public ICoreWebView2EnvironmentOptions {
	std::wstring m_args;
	ULONG m_refCount = 1;
	// Вспомогательный аллокатор COM-строк
	LPWSTR AllocString(const std::wstring &str) {
		LPWSTR ptr = (LPWSTR)CoTaskMemAlloc((str.length() + 1) * sizeof(WCHAR));
		if (ptr) wcscpy(ptr, str.c_str());
		return ptr;
	}

  public:
	AppEnvOptions(const std::wstring &args) : m_args(args) {}
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override {
		if (!ppv) return E_POINTER;
		if (IsEqualGUID(riid, HARDCODED_IID_IUnknown) || IsEqualGUID(riid, HARDCODED_IID_EnvOptions)) {
			*ppv = static_cast<ICoreWebView2EnvironmentOptions *>(this);
			AddRef();
			return S_OK;
		}
		*ppv = nullptr;
		return E_NOINTERFACE;
	}
	ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&m_refCount); }
	ULONG STDMETHODCALLTYPE Release() override {
		ULONG ref = InterlockedDecrement(&m_refCount);
		if (ref == 0) delete this;
		return ref;
	}
	HRESULT STDMETHODCALLTYPE get_AdditionalBrowserArguments(LPWSTR *value) override {
		*value = AllocString(m_args);
		// Ловушка: раскомментируй, чтобы убедиться, что метод вызван
		// MessageBoxW(NULL, L"Edge забрал флаги!", L"DEBUG", MB_OK);
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE put_AdditionalBrowserArguments(LPCWSTR value) override { return S_OK; }
	// ИСПРАВЛЕНИЕ: Отдаем пустые строки вместо nullptr
	HRESULT STDMETHODCALLTYPE get_Language(LPWSTR *value) override {
		*value = AllocString(L"");
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE put_Language(LPCWSTR value) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE get_TargetCompatibleBrowserVersion(LPWSTR *value) override {
		*value = AllocString(L"");
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE put_TargetCompatibleBrowserVersion(LPCWSTR value) override { return S_OK; }
	HRESULT STDMETHODCALLTYPE get_AllowSingleSignOnUsingOSPrimaryAccount(BOOL *allow) override {
		*allow = FALSE;
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE put_AllowSingleSignOnUsingOSPrimaryAccount(BOOL allow) override { return S_OK; }
};
// -------------------- COM helper --------------------
template <typename T> class CallbackImpl : public T {
  public:
	virtual ~CallbackImpl() = default;
	ULONG ref = 1;
	STDMETHODIMP_(ULONG) AddRef() override { return ++ref; }
	STDMETHODIMP_(ULONG) Release() override {
		ULONG r = --ref;
		if (r == 0) { delete this; }
		return r;
	}
};
// -------------------- handlers --------------------
class CtrlHandler : public CallbackImpl<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler> {
  public:
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override {
		if (!ppvObject) return E_POINTER;
		// БЕЗОПАСНОЕ СРАВНЕНИЕ ДЛЯ MinGW:
		if (IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler)) {
			*ppvObject = static_cast<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler *>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}
	STDMETHODIMP Invoke(HRESULT res, ICoreWebView2Controller *ctrl) override {
		if (FAILED(res) || !ctrl) return S_OK;
		controller = ctrl;
		controller->AddRef();
		controller->get_CoreWebView2(&webview);
		ICoreWebView2Controller2 *ctrl2;
		if (SUCCEEDED(controller->QueryInterface(IID_ICoreWebView2Controller2, (void **)&ctrl2))) {
			COREWEBVIEW2_COLOR color = {255, 30, 30, 30};
			ctrl2->put_DefaultBackgroundColor(color);
			ctrl2->Release();
		}
		RECT bounds;
		GetClientRect(hwnd, &bounds);
		controller->put_Bounds(bounds);
		webview->Navigate(L"http://127.0.0.1:9090");

		PostMessageW(hwnd, WM_SIZE, SIZE_RESTORED, 0);
		return S_OK;
	}
};
class EnvHandler : public CallbackImpl<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler> {
  public:
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override {
		if (!ppvObject) return E_POINTER;
		// БЕЗОПАСНОЕ СРАВНЕНИЕ ДЛЯ MinGW:
		if (IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler)) {
			*ppvObject = static_cast<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler *>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}
	STDMETHODIMP Invoke(HRESULT res, ICoreWebView2Environment *env) override {
		if (FAILED(res) || !env) return S_OK;
		env->CreateCoreWebView2Controller(hwnd, new CtrlHandler());
		return S_OK;
	}
};

// -------------------- window --------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_SIZE:
		if (controller) {
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			controller->put_Bounds(bounds);
		}
		if (wParam == SIZE_MINIMIZED) {
			// TrimTree(GetCurrentProcessId());
			// TrimTree(nodeProcess.dwProcessId);
		}
		return 0;

	case WM_TIMER:
		switch (wParam) {
		case 1:
			LogLauncherInfo(L"launcher Memory usage after WebView2 ");
			PrintMemoryBreakdown();
		}
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, 1);
		StopNode();
		if (cfg.logs.launcher_handle != INVALID_HANDLE_VALUE) {
			CloseHandle(cfg.logs.launcher_handle);
			cfg.logs.launcher_handle = INVALID_HANDLE_VALUE;
		}

		if (cfg.logs.node_handle != INVALID_HANDLE_VALUE) {
			CloseHandle(cfg.logs.node_handle);
			cfg.logs.node_handle = INVALID_HANDLE_VALUE;
		}

		if (cfg.logs.webview_handle != INVALID_HANDLE_VALUE) {
			CloseHandle(cfg.logs.webview_handle);
			cfg.logs.webview_handle = INVALID_HANDLE_VALUE;
		}

		if (cfg.logs.crash_handle != INVALID_HANDLE_VALUE) {
			CloseHandle(cfg.logs.crash_handle);
			cfg.logs.crash_handle = INVALID_HANDLE_VALUE;
		}
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

/// GUI setup
void First_slide(HINSTANCE hInstance) {
	NodeQuestion q;
	q.system_variant_available = true;
	q.system_node_path = L"c:\\program files\\nodejs\\node.exe";
	q.system_node_version = L"v24.13.1";
	q.custom_node_path = L"c:\\custom\\node.exe";

	if (ShowNodeQuestion(hInstance, q)) {
		if (q.use_system_variant) {
			LogLauncherInfo(q.system_node_path.c_str());
		} else {
			LogLauncherInfo(q.custom_node_path.c_str());
		}
	} else {
		LogLauncherError(L"user cancelled node selection");
	}
}

// -------------------- main --------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	int argc = 0;

	const wchar_t CLASS_NAME[] = L"AppGui";
	WNDCLASSW wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	typedef LPWSTR(WINAPI * PFN_GetCommandLineW)(void);
	typedef LPWSTR *(WINAPI * PFN_CommandLineToArgvW)(LPCWSTR, int *);
	LPWSTR *argv = nullptr;
	HMODULE hKernel32 = LoadLibraryW(L"kernel32.dll");
	HMODULE hShell32 = LoadLibraryW(L"shell32.dll");
	if (hKernel32 && hShell32) {
		PFN_GetCommandLineW pGetCommandLineW = (PFN_GetCommandLineW)GetProcAddress(hKernel32, "GetCommandLineW");
		PFN_CommandLineToArgvW pCommandLineToArgvW = (PFN_CommandLineToArgvW)GetProcAddress(hShell32, "CommandLineToArgvW");
		if (pGetCommandLineW && pCommandLineToArgvW) {
			LPWSTR cmd = pGetCommandLineW();
			argv = pCommandLineToArgvW(cmd, &argc);
		}
	}
	cfg = ParseFlags(argc, argv);
	if (argv) { LocalFree(argv); }
	if (hShell32) { FreeLibrary(hShell32); }
	if (hKernel32) { FreeLibrary(hKernel32); }
	////////////////////////////////////////

	// Config
	// cfg = ParseFlags(argc, argv);
	//	cfg.launcher - отладка
	// cfg.logs.launcher = LogMode::FILE_MODE;
	// cfg.logs.launcher = LogMode::CONS;
	// cfg.logs.launcher = LogMode::OFF; // defolt
	//	cfg.node - отладка
	// cfg.logs.node = LogMode::FILE_MODE;
	// cfg.logs.node = LogMode::CONS;
	cfg.logs.node = LogMode::OFF; // defolt
	if (cfg.logs.launcher == LogMode::CONS) {
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
	InitLogging(cfg.logs);
	LogLauncherInfo(L"launcher started");
	LogLauncherInfo(L"launcher Memory usage on start");
	PrintMemoryBreakdown();
	// wprintf(L"This writed to logfile=%ls\n", cfg.paths.root_dir.c_str());
	//////////////////Loging test for launcher
	// LogLauncherError(L"test launcher error");
	///////////////////
	//////////test run
	// Config cfg_tst;
	ProbeResult probe{};
	RunEnvironmentProbe(probe);
	PrintProbeResult(probe);
	Config cfg_tst;
	FillDefaultConfig(cfg_tst);
	SaveConfigJson(cfg_tst);

	Config cfg_tst2;
	cfg_tst2.paths.config_file = cfg_tst.paths.config_file;
	LoadConfigJson(cfg_tst2);
	// if(cfg_tst == cfg_tst2){} - not work
	////////////////////////
	First_slide(hInstance);
	Sleep(1000); // ← обязательно
	LogLauncherInfo(L"launcher Memory usage on GUI_SETUP");
	PrintMemoryBreakdown();
	///////////////////////

	/**/
	// ---- start server ----
	StartNode(cfg);
	// даём Node стартануть
	Sleep(1000); // ← обязательно
	if (!WaitForPort(9090, 15000)) { MessageBoxW(NULL, L"Server did not start", L"Error", MB_OK); }
	Sleep(1000); // ← обязательно
	LogLauncherInfo(L"launcher Memory usage after nodeJS satrt");
	PrintMemoryBreakdown();

	//	StartEdge(hInstance, nCmdShow);
	//////////////////////////
	// edge creator
	// RegisterClassW(&wc);

	// SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS",
	// 						L"--disable-features=LayoutNGPrinting,Printing "
	// 						L"--js-flags=\"--max-semi-space-size=8\"");
	// hwnd = CreateWindowExW(0, CLASS_NAME, L"AppGui", WS_OVERLAPPEDWINDOW,
	// 					   CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, NULL, NULL,
	// 					   hInstance, NULL);
	// ////////////////////////////////////////////////////
	RegisterClassW(&wc);
	// Создаем родительское окно Host Process
	hwnd = CreateWindowExW(0, CLASS_NAME, L"AppGui", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, NULL, NULL, hInstance, NULL);
	Sleep(1000); // ← обязательно

	// ДВМапи
	// BOOL useDarkMode = TRUE;
	// DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
	// --- ПОЗДНЕЕ СВЯЗЫВАНИЕ DWMAPI ---
	typedef HRESULT(WINAPI * PFN_DwmSetWindowAttribute)(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute);
	PFN_DwmSetWindowAttribute DwmSetWindowAttributePtr = nullptr;
	HMODULE dwmapiModule = nullptr;
	HMODULE hDwmApi = LoadLibraryW(L"dwmapi.dll");
	if (hDwmApi) {
		PFN_DwmSetWindowAttribute pDwmSetWindowAttribute = (PFN_DwmSetWindowAttribute)GetProcAddress(hDwmApi, "DwmSetWindowAttribute");

		if (pDwmSetWindowAttribute) {
			BOOL useDarkMode = TRUE;
			pDwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
		}
		FreeLibrary(hDwmApi);
	}
	// --- ПОЗДНЕЕ СВЯЗЫВАНИЕ DWMAPI ---

	// Для WebView2 нужно перечислить все дочерние процессы через EnumProcesses
	// или CreateToolhelp32Snapshot
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd); // Принудительно рисуем темный фон
	// Применение к Node и самому себе
	TrimMemory(nodeProcess.hProcess);
	TrimTree(nodeProcess.dwProcessId);
	TrimMemory(GetCurrentProcess());
	TrimTree(GetCurrentProcessId());
	LogLauncherInfo(L"launcher Memory usage befor WebView2 ");
	PrintMemoryBreakdown();

	// wchar_t sysroot[MAX_PATH];
	// GetEnvironmentVariableW(L"SystemRoot", sysroot, MAX_PATH);
	// wchar_t comspec[MAX_PATH];
	// GetEnvironmentVariableW(L"ComSpec", comspec, MAX_PATH);
	// wprintf(L"comspec=%ls\n", comspec);
	// wprintf(L"SystemRoot=%ls\n", sysroot);

	// ---- WebView2 ----
	// Формируем монолитную строку аргументов для Chromium
	const wchar_t *webviewArgs3 = L"--disable-gpu "
								  L"--disable-gpu-compositing "
								  L"--disable-software-rasterizer "
								  L"--renderer-process-limit=1 "
								  L"--disable-speech-api "
								  L"--disable-background-networking "
								  L"--disable-sync "
								  L"--disable-default-apps "
								  L"--disable-dev-tools "
								  L"--disable-webassembly "
								  L"--disable-plugins "
								  L"--disable-extensions "
								  L"--disable-site-isolation-trials "
								  L"--in-process-gpu "		  //
								  L"--use-angle=swiftshader " //
								  L"--disable-gpu-sandbox "	  //
															  //   L"--enable-logging --v=1"//
															  //   L"--js-flags=--max-old-space-size=64 "
								  L"--js-flags=\"--max-semi-space-size=8 --max-old-space-size=128 --lite-mode\" "
								  L"--disable-features=AudioServiceOutOfProcess,MediaSessionService,LayoutNGPrinting,Printing,Translate,Extensions,GlobalMediaControls,PictureInPicture,WebRtc";
	// Устанавливаем переменную окружения для WebView2 Loader'а
	SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", webviewArgs3);
	// auto options = new AppEnvOptions(webviewArgs);
	auto envHandler = new EnvHandler();
	// Запуск с легальной передачей опций в COM!
	// HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(NULL, L".\\WebView2_Profile", options, envHandler);
	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(NULL, L".\\WebView2_Profile", NULL, envHandler);
	if (FAILED(hr)) {
		wchar_t buf[256];
		wsprintfW(buf, L"Синхронная ошибка старта WebView2!\nHRESULT: 0x%08lX", hr);
		MessageBoxW(hwnd, buf, L"Crash", MB_ICONERROR);
	}
	// options->Release();
	envHandler->Release();

	LogLauncherInfo(L"launcher Memory usage after WebView2 ");
	PrintMemoryBreakdown();
	// hwnd — владелец таймера
	// 1 — ID таймера (ключ!) ->case WM_TIMER:switch (wParam) { case 1(ключ!) : ....}
	// 60000 — интервал
	// NULL — без callback → через WM_TIMER
	SetTimer(hwnd, 1, 60000, NULL);
	// ---- loop ----
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	/**/
	CoUninitialize();
	return 0;
}