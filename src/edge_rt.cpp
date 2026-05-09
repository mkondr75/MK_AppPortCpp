/// \file edge_rt.cpp
#include "../include/edge_rt.hpp"
#include <webview2.h>
// #include <iostream>
extern void PrintMemoryBreakdown();
// -------------------- Stealth OLE Globals --------------------
typedef HRESULT(WINAPI *PFN_CoInitializeEx)(LPVOID, DWORD);
typedef void(WINAPI *PFN_CoUninitialize)(void);
typedef LPVOID(WINAPI *PFN_CoTaskMemAlloc)(SIZE_T);
typedef void(WINAPI *PFN_CoTaskMemFree)(LPVOID); // Добавлено для освобождения строк JS

PFN_CoInitializeEx pCoInitializeEx = nullptr;
PFN_CoUninitialize pCoUninitialize = nullptr;
PFN_CoTaskMemAlloc pCoTaskMemAlloc = nullptr;
PFN_CoTaskMemFree pCoTaskMemFree = nullptr;
HMODULE hOle32 = nullptr;

bool InitStealthOLE() {
	hOle32 = GetModuleHandleW(L"ole32.dll");
	if (!hOle32) hOle32 = LoadLibraryW(L"ole32.dll");

	if (hOle32) {
		pCoInitializeEx = (PFN_CoInitializeEx)GetProcAddress(hOle32, "CoInitializeEx");
		pCoUninitialize = (PFN_CoUninitialize)GetProcAddress(hOle32, "CoUninitialize");
		pCoTaskMemAlloc = (PFN_CoTaskMemAlloc)GetProcAddress(hOle32, "CoTaskMemAlloc");
		pCoTaskMemFree = (PFN_CoTaskMemFree)GetProcAddress(hOle32, "CoTaskMemFree");
	}
	if (pCoInitializeEx && pCoUninitialize && pCoTaskMemAlloc && pCoTaskMemFree) {
		pCoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
		return true;
	}
	return false;
	// return (pCoInitializeEx && pCoUninitialize && pCoTaskMemAlloc && pCoTaskMemFree);
}

void CleanupStealthOLE() {
	if (pCoUninitialize) pCoUninitialize();
}

LPWSTR AllocString(const std::wstring &str) {
	if (!pCoTaskMemAlloc) return nullptr;
	LPWSTR ptr = (LPWSTR)pCoTaskMemAlloc((str.length() + 1) * sizeof(WCHAR));
	if (ptr) wcscpy(ptr, str.c_str());
	return ptr;
}

// -------------------- GUIDs & COM Helpers --------------------
constexpr GUID HARDCODED_IID_EnvOptions = {0x2fde08a8, 0x1e9a, 0x4766, {0x8c, 0x05, 0x95, 0xa9, 0xce, 0xb9, 0xd1, 0xc5}};
constexpr GUID HARDCODED_IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

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

// -------------------- Globals for Single Window --------------------
static ICoreWebView2Controller *g_controller = nullptr;
static ICoreWebView2 *g_webview = nullptr;
static JSMessageHandler g_jsHandler = nullptr;

// -------------------- JS Message Handler --------------------
class WebMessageHandler : public CallbackImpl<ICoreWebView2WebMessageReceivedEventHandler> {
  public:
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override {
		if (!ppvObject) return E_POINTER;
		if (IsEqualGUID(riid, IID_IUnknown) || IsEqualGUID(riid, IID_ICoreWebView2WebMessageReceivedEventHandler)) {
			*ppvObject = static_cast<ICoreWebView2WebMessageReceivedEventHandler *>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHODIMP Invoke(ICoreWebView2 *sender, ICoreWebView2WebMessageReceivedEventArgs *args) override {
		LPWSTR message = nullptr;
		if (SUCCEEDED(args->TryGetWebMessageAsString(&message)) && message) {
			if (g_jsHandler) { g_jsHandler(std::wstring(message)); }
			if (pCoTaskMemFree) pCoTaskMemFree(message); // Используем наш стелс-указатель
		}
		return S_OK;
	}
};

// -------------------- WebView2 Handlers --------------------
class CtrlHandler : public CallbackImpl<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler> {
	HWND m_hwnd;

  public:
	CtrlHandler(HWND hwnd) : m_hwnd(hwnd) {}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override {
		if (!ppvObject) return E_POINTER;
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
		g_controller = ctrl;
		g_controller->AddRef();
		g_controller->get_CoreWebView2(&g_webview);

		ICoreWebView2Controller2 *ctrl2;
		if (SUCCEEDED(g_controller->QueryInterface(IID_ICoreWebView2Controller2, (void **)&ctrl2))) {
			COREWEBVIEW2_COLOR color = {255, 30, 30, 30};
			ctrl2->put_DefaultBackgroundColor(color);
			ctrl2->Release();
		}

		// Регистрируем мост JS -> C++
		EventRegistrationToken token;
		g_webview->add_WebMessageReceived(new WebMessageHandler(), &token);

		RECT bounds;
		GetClientRect(m_hwnd, &bounds);
		g_controller->put_Bounds(bounds);
		g_webview->Navigate(L"http://127.0.0.1:9090");

		PostMessageW(m_hwnd, WM_SIZE, SIZE_RESTORED, 0);
		return S_OK;
	}
};

class EnvHandler : public CallbackImpl<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler> {
	HWND m_hwnd;

  public:
	EnvHandler(HWND hwnd) : m_hwnd(hwnd) {}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) override {
		if (!ppvObject) return E_POINTER;
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
		env->CreateCoreWebView2Controller(m_hwnd, new CtrlHandler(m_hwnd));
		return S_OK;
	}
};

// -------------------- EdgeRT Implementation --------------------

EdgeRT::EdgeRT(HINSTANCE hInstance) : m_hwnd(nullptr), m_hInstance(hInstance) {}

EdgeRT::~EdgeRT() {
	if (g_controller) {
		g_controller->Release();
		g_controller = nullptr;
	}
}

HWND EdgeRT::GetHWND() const { return m_hwnd; }

void EdgeRT::SetMessageHandler(JSMessageHandler handler) { g_jsHandler = handler; }

bool EdgeRT::SendToJS(const std::wstring &message) {
	if (g_webview) {
		g_webview->PostWebMessageAsJson(message.c_str());
		return true;
	}
	return false;
}

bool EdgeRT::ExecuteJS(const std::wstring &script) {
	if (g_webview) {
		g_webview->ExecuteScript(script.c_str(), nullptr);
		return true;
	}
	return false;
}

void EdgeRT::ApplyDarkMode() {
	typedef HRESULT(WINAPI * PFN_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
	HMODULE hDwmApi = LoadLibraryW(L"dwmapi.dll");
	if (hDwmApi) {
		PFN_DwmSetWindowAttribute pDwmSetWindowAttribute = (PFN_DwmSetWindowAttribute)GetProcAddress(hDwmApi, "DwmSetWindowAttribute");
		if (pDwmSetWindowAttribute) {
			BOOL useDarkMode = TRUE;
			pDwmSetWindowAttribute(m_hwnd, 20, &useDarkMode, sizeof(useDarkMode));
		}
		FreeLibrary(hDwmApi);
	}
}

void EdgeRT::InitializeWebView(const Config &cfg) {
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
								  L"--in-process-gpu "
								  L"--use-angle=swiftshader "
								  L"--disable-gpu-sandbox "
								  L"--js-flags=\"--max-semi-space-size=8 --max-old-space-size=128 --lite-mode\" "
								  L"--disable-features=AudioServiceOutOfProcess,MediaSessionService,LayoutNGPrinting,Printing,Translate,Extensions,GlobalMediaControls,PictureInPicture,WebRtc";

	SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", webviewArgs3);

	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(NULL, L".\\WebView2_Profile", NULL, new EnvHandler(m_hwnd));
	if (FAILED(hr)) { MessageBoxW(m_hwnd, L"Ошибка старта WebView2!", L"Crash", MB_ICONERROR); }
}

bool EdgeRT::Start(const Config &cfg, int nCmdShow) {
	const wchar_t CLASS_NAME[] = L"AppGui";
	WNDCLASSW wc = {};
	wc.lpfnWndProc = EdgeRT::WndProc;
	wc.hInstance = m_hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
	RegisterClassW(&wc);

	m_hwnd = CreateWindowExW(0, CLASS_NAME, L"AppGui", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, NULL, NULL, m_hInstance, NULL);
	if (!m_hwnd) return false;

	ApplyDarkMode();
	ShowWindow(m_hwnd, nCmdShow);
	UpdateWindow(m_hwnd);

	InitializeWebView(cfg);
	return true;
}

LRESULT CALLBACK EdgeRT::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		// case WM_TIMER:
		// 	switch (wParam) {
		// 	case 1:
		// 		LogLauncherInfo(L"launcher Memory usage after WebView2 ");
		// 		PrintMemoryBreakdown();
		// 	}
		// 	return 0;

	case WM_SIZE:
		if (g_controller) {
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			g_controller->put_Bounds(bounds);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}