/// \file edge_rt.cpp
#include "../include/edge_rt.hpp"
#include <webview2.h>

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
	const wchar_t *webviewArgs3 =
		// ГРУППА 1: Архитектура процесса (Высший приоритет)
		L"--single-process "		   // Все подсистемы в один EXE
		L"--renderer-process-limit=1 " // Жесткий лимит на потоки отрисовки
		L"--in-process-gpu "		   // GPU-поток внутри родителя

		// ГРУППА 2: Полное подавление графики (Убираем SwiftShader и GPU)
		L"--disable-gpu "					// Отключаем аппаратное ускорение
		L"--disable-gpu-compositing "		// Запрет на композицию через GPU
		L"--disable-software-rasterizer "	// Отключаем программный эмулятор
		L"--disable-gpu-sandbox "			// Убираем лишние барьеры для встроеного GPU
		L"--disable-gpu-shader-disk-cache " // [AA_] Запрет на запись кэша шейдеров на диск

		// ГРУППА 3: Сеть и внешние сервисы (Минимизация трафика и потоков)
		L"--disable-background-networking " // Глушим сетевой шум
		L"--disable-sync "					// Отключаем синхронизацию профиля
		L"--disable-speech-api "			// Убираем распознавание речи
		L"--disable-domain-reliability "	// [AA_] Отключаем проверку доступности доменов

		// ГРУППА 4: Изоляция и безопасность (Снижение оверхеда)
		L"--disable-site-isolation-trials " // Отключаем изоляцию сайтов (для лаунчера не нужно)
		L"--disable-webassembly "			// Экономим на WASM-движке
		L"--disable-plugins "				// Никаких плагинов
		L"--disable-extensions "			// Полный запрет расширений
		L"--disable-dev-tools "				// Отключаем инструменты отладки в релизе
		// Добавь эти флаги в AA_edge_rt.cpp
		L"--disable-font-subpixel-positioning " // Упрощает отрисовку текста
		L"--disable-lcd-text "					// Еще меньше памяти на шрифты
		L"--disable-notifications "				// Отключаем систему уведомлений
		L"--disable-remote-core "				// Убираем возможность удаленного управления
		// L"--no-sandbox "						// (ТОЛЬКО ДЛЯ ТЕСТА) Схлопывает границы безопасности, может убрать еще 5.8 MB(пруф)
		// ГРУППА 5: Тонкая настройка V8 (JS-диета)
		L"--js-flags=\"--max-semi-space-size=8 --max-old-space-size=128 --lite-mode --expose-gc\" " // [AA_] Добавлен ручной GC

		// ГРУППА 6: Отключение тяжелых фич (Компоненты)
		L"--disable-features=AudioServiceOutOfProcess,MediaSessionService,LayoutNGPrinting,"
		L"Printing,Translate,Extensions,GlobalMediaControls,PictureInPicture,WebRtc"; //
	SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", webviewArgs3);

	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(NULL, L".\\WebView2_Profile", NULL, new EnvHandler(m_hwnd));
	if (FAILED(hr)) { MessageBoxW(m_hwnd, L"Ошибка старта WebView2!", L"Crash", MB_ICONERROR); }

	// dtbugg
	wchar_t AA_envCheck[1024];
	if (GetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", AA_envCheck, 1024)) {
		// Мы должны увидеть наши флаги в консоли/логе ДО вызова Create...
		OutputDebugStringW(L"[AA_DEBUG] Flags found: ");
		OutputDebugStringW(AA_envCheck);
	} else {
		OutputDebugStringW(L"[AA_DEBUG] CRITICAL: Flags are missing!");
	}
}

bool EdgeRT::Start(const Config &cfg, int nCmdShow) {
	const wchar_t CLASS_NAME[] = L"AppGui";
	WNDCLASSW wc = {};
	wc.lpfnWndProc = EdgeRT::AA_WndProc;
	wc.hInstance = m_hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
	RegisterClassW(&wc);
	m_hwnd = CreateWindowExW(0, CLASS_NAME, L"AppGui", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800, NULL, NULL, m_hInstance, this); // Передаем this
	if (!m_hwnd) return false;
	ApplyDarkMode();
	ShowWindow(m_hwnd, nCmdShow);
	UpdateWindow(m_hwnd);

	InitializeWebView(cfg);
	return true;
}

LRESULT CALLBACK EdgeRT::AA_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	// Вытаскиваем "хозяина" окна из системного кармана
	EdgeRT *AA_pInstance = (EdgeRT *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
	// Фокус инициализации: ловим момент рождения окна
	if (msg == WM_NCCREATE) {
		CREATESTRUCTW *AA_pCreateData = (CREATESTRUCTW *)lParam;
		AA_pInstance = (EdgeRT *)AA_pCreateData->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)AA_pInstance);
	}
	// Если связь установлена, работаем через AA_pInstance
	if (AA_pInstance) {
		switch (msg) {
		case WM_DESTROY:
			if (AA_pInstance->m_exitHandler) { AA_pInstance->m_exitHandler(); }
			PostQuitMessage(0);
			return 0;
		case WM_SIZE:
			// Здесь мы теперь можем безопасно обращаться к AA_m_hwnd или другим членам
			if (g_controller) {
				RECT AA_rectBounds;
				GetClientRect(hWnd, &AA_rectBounds);
				g_controller->put_Bounds(AA_rectBounds);
			}
			return 0;
		}
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}
