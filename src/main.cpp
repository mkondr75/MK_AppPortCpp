#include "../include/main.hpp"

// #pragma comment(lib, "ws2_32.lib")

// -------------------- globals --------------------
HWND hwnd = nullptr;
ICoreWebView2Controller *controller = nullptr;
ICoreWebView2 *webview = nullptr;
Config cfg;

PROCESS_INFORMATION nodeProcess = {};

// -------------------- COM helper --------------------
template <typename T>
class CallbackImpl : public T
{
public:
	virtual ~CallbackImpl() = default;
	ULONG ref = 1;
	STDMETHODIMP QueryInterface(
		REFIID riid,
		void **ppvObject) override
	{
		if (riid == IID_IUnknown)
		{
			*ppvObject = static_cast<T *>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}
	STDMETHODIMP_(ULONG)
	AddRef() override
	{
		return ++ref;
	}
	STDMETHODIMP_(ULONG)
	Release() override
	{
		ULONG r = --ref;
		if (r == 0)
		{
			delete this;
		}
		return r;
	}
};
// -------------------- handlers --------------------
class CtrlHandler : public CallbackImpl<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>
{
public:
	STDMETHODIMP Invoke(HRESULT, ICoreWebView2Controller *ctrl) override
	{
		controller = ctrl;
		controller->AddRef();
		controller->get_CoreWebView2(&webview);
		RECT bounds;
		GetClientRect(hwnd, &bounds);
		controller->put_Bounds(bounds);
		webview->Navigate(L"http://127.0.0.1:9090");
		return S_OK;
	}
};
class EnvHandler : public CallbackImpl<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>
{
public:
	STDMETHODIMP Invoke(HRESULT, ICoreWebView2Environment *env) override
	{
		env->CreateCoreWebView2Controller(hwnd, new CtrlHandler());
		// env->remove_NewBrowserVersionAvailable();
		return S_OK;
	}
	virtual ~EnvHandler() = default;
};
// memory trim
void TrimMemory(HANDLE hProcess)
{
	// Выгружает все возможные страницы процесса в файл подкачки (Standby/Free list)
	SetProcessWorkingSetSize(hProcess, (SIZE_T)-1, (SIZE_T)-1);
	EmptyWorkingSet(hProcess);
}

void TrimTree(DWORD parentId)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return;

	PROCESSENTRY32W pe;
	pe.dwSize = sizeof(pe);

	if (Process32FirstW(hSnap, &pe))
	{
		do
		{
			if (pe.th32ParentProcessID == parentId)
			{
				HANDLE hChild = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
				if (hChild)
				{
					SetProcessWorkingSetSize(hChild, (SIZE_T)-1, (SIZE_T)-1);
					TrimTree(pe.th32ProcessID); // Рекурсивно для вложенных
					CloseHandle(hChild);
				}
			}
		} while (Process32NextW(hSnap, &pe));
	}
	CloseHandle(hSnap);
}

// -------------------- window --------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
		if (controller)
		{
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			controller->put_Bounds(bounds);
		}
		if (wParam == SIZE_MINIMIZED)
		{
			TrimTree(GetCurrentProcessId());
			TrimTree(nodeProcess.dwProcessId);
		}
		return 0;

	case WM_DESTROY:
		StopNode();
		if (cfg.logs.launcher_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(cfg.logs.launcher_handle);
			cfg.logs.launcher_handle = INVALID_HANDLE_VALUE;
		}

		if (cfg.logs.node_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(cfg.logs.node_handle);
			cfg.logs.node_handle = INVALID_HANDLE_VALUE;
		}

		if (cfg.logs.webview_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(cfg.logs.webview_handle);
			cfg.logs.webview_handle = INVALID_HANDLE_VALUE;
		}

		if (cfg.logs.crash_handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(cfg.logs.crash_handle);
			cfg.logs.crash_handle = INVALID_HANDLE_VALUE;
		}
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

/// GUI setup
void First_slide(HINSTANCE hInstance)
{
	NodeQuestion q;
	q.system_variant_available = true;
	q.system_node_path = L"c:\\program files\\nodejs\\node.exe";
	q.system_node_version = L"v24.13.1";
	q.custom_node_path = L"c:\\custom\\node.exe";

	if (ShowNodeQuestion(hInstance, q))
	{
		if (q.use_system_variant)
		{
			LogLauncherInfo(q.system_node_path.c_str());
		}
		else
		{
			LogLauncherInfo(q.custom_node_path.c_str());
		}
	}
	else
	{
		LogLauncherError(L"user cancelled node selection");
	}
}

// -------------------- main --------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

	const wchar_t CLASS_NAME[] = L"AppGui";

	WNDCLASSW wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	////////////////////////////
	// AllocConsole();
	// freopen("CONOUT$", "w", stdout);
	// freopen("CONOUT$", "w", stderr);
	////////////////////////////
	// ParseFlags();
	int argc = 0;
	LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	// Config
	cfg = ParseFlags(argc, argv);
	//	cfg.launcher - отладка
	cfg.logs.launcher = LogMode::FILE_MODE;
	// cfg.logs.launcher = LogMode::CONS;
	// cfg.logs.launcher = LogMode::OFF; // defolt
	//	cfg.node - отладка
	cfg.logs.node = LogMode::FILE_MODE;
	// cfg.logs.node = LogMode::CONS;
	// cfg.logs.node = LogMode::OFF; // defolt
	if (cfg.logs.launcher == LogMode::CONS)
	{
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
	}
	InitLogging(cfg.logs);
	LogLauncherInfo(L"launcher started");
	// wprintf(L"This writed to logfile=%ls\n", cfg.paths.root_dir.c_str());
	//////////////////Loging test for launcher
	LogLauncherError(L"test launcher error");
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
	///////////////////////
	// ---- start server ----
	StartNode(cfg);
	// даём Node стартануть
	Sleep(1000); // ← обязательно

	if (!WaitForPort(9090, 15000))
	{
		MessageBoxW(NULL, L"Server did not start", L"Error", MB_OK);
	}
	//	StartEdge(hInstance, nCmdShow);
	//////////////////////////
	// edge creator
	RegisterClassW(&wc);
	SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", L"--disable-features=LayoutNGPrinting,Printing --js-flags=\"--max-semi-space-size=8\"");
	hwnd = CreateWindowExW(
		0, CLASS_NAME, L"AppGui",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,
		NULL, NULL, hInstance, NULL);
	Sleep(1000); // ← обязательно

	// Применение к Node и самому себе
	TrimMemory(nodeProcess.hProcess);
	TrimTree(nodeProcess.dwProcessId);
	TrimMemory(GetCurrentProcess());
	TrimTree(GetCurrentProcessId());
	// Для WebView2 нужно перечислить все дочерние процессы через EnumProcesses или CreateToolhelp32Snapshot
	ShowWindow(hwnd, nCmdShow);

	wchar_t sysroot[MAX_PATH];
	GetEnvironmentVariableW(L"SystemRoot", sysroot, MAX_PATH);
	wchar_t comspec[MAX_PATH];
	GetEnvironmentVariableW(L"ComSpec", comspec, MAX_PATH);
	wprintf(L"comspec=%ls\n", comspec);
	wprintf(L"SystemRoot=%ls\n", sysroot);

	// ---- WebView2 ----
	CreateCoreWebView2EnvironmentWithOptions(
		NULL, NULL, NULL,
		new EnvHandler());

	// ---- loop ----
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}