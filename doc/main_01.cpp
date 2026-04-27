#include "../include/main.hpp"

//#pragma comment(lib, "ws2_32.lib")

// -------------------- globals --------------------
HWND hwnd = nullptr;
ICoreWebView2Controller* controller = nullptr;
ICoreWebView2* webview = nullptr;

PROCESS_INFORMATION nodeProcess = {};

// -------------------- wait for port --------------------
bool WaitForPort(int port, int timeoutMs = 15000) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    DWORD start = GetTickCount();

    while (GetTickCount() - start < (DWORD)timeoutMs) {

        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == 0) {
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

/*
LPVOID BuildEnv() {
    LPWCH base = GetEnvironmentStringsW();

    std::vector<std::wstring> vars;

    // копируем текущее окружение
    for (LPWCH p = base; *p; ) {
        std::wstring var = p;
        vars.push_back(var);
        p += var.size() + 1;
    }
    FreeEnvironmentStringsW(base);

    // добавляем/переопределяем свои
    vars.push_back(L"TRILIUM_DATA_DIR=C:\\dev_node\\trilium_dev\\data_sandbox");
    vars.push_back(L"TRILIUM_PORT=9090");
    vars.push_back(L"HOST=127.0.0.1");
    vars.push_back(L"USERPROFILE=C:\\dev_node\\trilium_dev\\profile");
    vars.push_back(L"APPDATA=C:\\dev_node\\trilium_dev\\profile\\AppData\\Roaming");
    vars.push_back(L"LOCALAPPDATA=C:\\dev_node\\trilium_dev\\profile\\AppData\\Local");
    vars.push_back(L"TEMP=C:\\dev_node\\trilium_dev\\profile\\.tmp");
    vars.push_back(L"TMP=C:\\dev_node\\trilium_dev\\profile\\.tmp");
    vars.push_back(L"HOME=C:\\dev_node\\trilium_dev");

    // сортировка ОБЯЗАТЕЛЬНА
    std::sort(vars.begin(), vars.end());

    // собираем блок
    std::wstring env;
    for (auto& v : vars) {
        env += v;
        env += L'\0';
    }
    env += L'\0';

    return _wcsdup(env.c_str());
}
*/
// -------------------- node --------------------
void StartNode() {
    STARTUPINFOW si = { sizeof(si) };
	LPVOID lpEnvironment;

    //wchar_t cmd[] = L"node.exe apps\\server\\dist\\main.cjs";
	//node --permission --allow-fs-read=./* --allow-fs-write=./* --allow-fs-read=C:\dev_node\trilium_dev --allow-fs-write=C:\dev_node\trilium_dev --allow-child-process --allow-addons -e "console.log(process.env); console.log(process.execArgv)"
    //wchar_t cmd[] = L"\"C:\\Program Files\\nodejs\\node.exe\" --permission --allow-fs-read=./* --allow-fs-write=./* --allow-fs-read=C:\\dev_node\\trilium_dev --allow-fs-write=C:\\dev_node\\trilium_dev --allow-child-process --allow-addons -e \"import { fileURLToPath } from 'url';console.log(fileURLToPath(import.meta.url)); console.log(process.env); console.log(process.execArgv);process.stdin.resume(); process.stdin.on('data', ()=>process.exit(0));\"";
    wchar_t cmd[] = L"\"C:\\Program Files\\nodejs\\node.exe\" --max-old-space-size=192 --max-semi-space-size=16 --expose-gc --permission --allow-fs-read=./* --allow-fs-write=./* --allow-fs-read=C:\\dev_node\\trilium_dev --allow-fs-write=C:\\dev_node\\trilium_dev --allow-child-process --allow-addons .\\apps\\server\\dist\\main.cjs";
	//wchar_t cmd[] =	L"\"C:\\Program Files\\nodejs\\node.exe\" -e \"console.log('OK');process.stdin.resume(); process.stdin.on('data', ()=>process.exit(0));\"";
	LPWCH base = GetEnvironmentStringsW();
	FreeEnvironmentStringsW(base);
	//SetEnvironmentVariableW(L"TRILIUM_DATA_DIR", L"C:\\dev_node\\trilium_dev\\data_sandbox");
	//SetEnvironmentVariableW(L"TRILIUM_PORT", L"9090");
	//SetEnvironmentVariableW(L"HOST", L"127.0.0.1");
SetEnvironmentVariableW(L"APPDATA", L"C:\\dev_node\\trilium_dev\\profile\\AppData\\Roaming");
SetEnvironmentVariableW(L"ComSpec", L"C:\\WINDOWS\\system32\\cmd.exe");
SetEnvironmentVariableW(L"HOST", L"127.0.0.1");
SetEnvironmentVariableW(L"LOCALAPPDATA", L"C:\\dev_node\\trilium_dev\\profile\\AppData\\Local");
SetEnvironmentVariableW(L"PATH", L"C:\\msys64\\home\\maxko\\dev\\dev_node\\node_bin;C:\\WINDOWS\\system32");
SetEnvironmentVariableW(L"SYSTEMROOT", L"C:\\WINDOWS");
SetEnvironmentVariableW(L"TEMP", L"C:\\dev_node\\trilium_dev\\profile\\.tmp");
SetEnvironmentVariableW(L"TMP", L"C:\\dev_node\\trilium_dev\\profile\\.tmp");
SetEnvironmentVariableW(L"TRILIUM_DATA_DIR", L"C:\\dev_node\\trilium_dev\\data_sandbox");
SetEnvironmentVariableW(L"TRILIUM_PORT", L"9090");
SetEnvironmentVariableW(L"USERPROFILE", L"C:\\dev_node\\trilium_dev\\profile");


	
	wchar_t sysroot[MAX_PATH];
	GetEnvironmentVariableW(L"SystemRoot", sysroot, MAX_PATH);
	wchar_t comspec[MAX_PATH];
	GetEnvironmentVariableW(L"ComSpec", comspec, MAX_PATH);
	wprintf(L"comspec=%ls\n", comspec);
	wprintf(L"SystemRoot=%ls\n", sysroot);
	//comspec=C:\WINDOWS\system32\cmd.exe
	//SystemRoot=C:\WINDOWS
	wchar_t env1[] =
		L"TRILIUM_DATA_DIR=C:\\dev_node\\trilium_dev\\data_sandbox\0"
		L"TRILIUM_PORT=9090\0"
		L"HOST=127.0.0.1\0"
		L"USERPROFILE=C:\\dev_node\\trilium_dev\\profile\0"
		L"APPDATA=C:\\dev_node\\trilium_dev\\profile\\AppData\\Roaming\0"
		L"LOCALAPPDATA=C:\\dev_node\\trilium_dev\\profile\\AppData\\Local\0"
		L"TEMP=C:\\dev_node\\trilium_dev\\profile\\.tmp\0"
		L"TMP=C:\\dev_node\\trilium_dev\\profile\\.tmp\0"
		L"HOME=C:\\dev_node\\trilium_dev\0"
		L"SystemRoot=C:\\WINDOWS\0"
		L"ComSpec=C:\\WINDOWS\\system32\\cmd.exe\0"
		L"PATH=C:\\msys64\\home\\maxko\\dev\\dev_node\\node_bin\0"
		L"\0";
//	LPVOID env = BuildEnv();
wchar_t env[] =
L"APPDATA=C:\\dev_node\\trilium_dev\\profile\\AppData\\Roaming\0"
L"ComSpec=C:\\WINDOWS\\system32\\cmd.exe\0"
L"HOST=127.0.0.1\0"
L"LOCALAPPDATA=C:\\dev_node\\trilium_dev\\profile\\AppData\\Local\0"
L"PATH=C:\\msys64\\home\\maxko\\dev\\dev_node\\node_bin;C:\\WINDOWS\\system32\0"
L"SYSTEMROOT=C:\\WINDOWS\0"
L"TEMP=C:\\dev_node\\trilium_dev\\profile\\.tmp\0"
L"TMP=C:\\dev_node\\trilium_dev\\profile\\.tmp\0"
L"TRILIUM_DATA_DIR=C:\\dev_node\\trilium_dev\\data_sandbox\0"
L"TRILIUM_PORT=9090\0"
L"USERPROFILE=C:\\dev_node\\trilium_dev\\profile\0"
L"\0";
//SystemRoot
//ComSpec
//PATH		
// ::изоляция окружения в папку profile
// %~dp0=>C:\dev_node\trilium_dev\\
// set "USERPROFILE=%~dp0profile"
// set "APPDATA=%~dp0profile\AppData\Roaming"
// set "LOCALAPPDATA=%~dp0profile\AppData\Local"
// set "TEMP=%~dp0profile\.tmp"
// set "TMP=%~dp0profile\.tmp"
// set "HOME=C:\dev_node\trilium_dev"


//	wchar_t cmd1[] =L"cmd.exe /c echo STATUS_OK & pause";
//	CreateProcessW(NULL, cmd1, NULL, NULL, FALSE,CREATE_NEW_CONSOLE, NULL, NULL, &si, &nodeProcess);
    if (!CreateProcessW(
//        L"C:\\Program Files\\nodejs",   // ← поправь путь при необходимости
//		L"C:\\msys64\\home\\maxko\\dev\\dev_node\\node_bin",
		NULL,
        cmd,
        NULL, NULL, FALSE,
        CREATE_NEW_CONSOLE,//CREATE_NO_WINDOW,
		NULL,//env,   // ← ВОТ ТУТ
        L"C:\\dev_node\\trilium_dev",             // ← рабочая папка
        &si,
        &nodeProcess
    )) {
			DWORD err = GetLastError();
			wchar_t buf[256];
			wsprintf(buf, L"Error: %lu", err);
			MessageBoxW(NULL, buf, L"CreateProcess", MB_OK);
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

// -------------------- COM helper --------------------
template <typename T>
class CallbackImpl : public T {
public:
    ULONG ref = 1;

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override {
		if (riid == IID_IUnknown) {
			*ppvObject = static_cast<T*>(this);
			AddRef();
			return S_OK;
		}
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}
	
	
    STDMETHODIMP_(ULONG) AddRef() override {
        return ++ref;
    }

    STDMETHODIMP_(ULONG) Release() override {
        ULONG r = --ref;
        if (r == 0) delete this;
        return r;
    }
};

// -------------------- handlers --------------------
class CtrlHandler :
    public CallbackImpl<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>
{
public:
    STDMETHODIMP Invoke(HRESULT, ICoreWebView2Controller* ctrl) override {
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

class EnvHandler :
    public CallbackImpl<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>
{
public:
    STDMETHODIMP Invoke(HRESULT, ICoreWebView2Environment* env) override {
        env->CreateCoreWebView2Controller(hwnd, new CtrlHandler());
        return S_OK;
    }
};
//memory trim
void TrimMemory(HANDLE hProcess) {
    // Выгружает все возможные страницы процесса в файл подкачки (Standby/Free list)
    SetProcessWorkingSetSize(hProcess, (SIZE_T)-1, (SIZE_T)-1);
    EmptyWorkingSet(hProcess);
}

void TrimTree(DWORD parentId) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);

    if (Process32FirstW(hSnap, &pe)) {
        do {
            if (pe.th32ParentProcessID == parentId) {
                HANDLE hChild = OpenProcess(PROCESS_SET_QUOTA | PROCESS_QUERY_INFORMATION, FALSE, pe.th32ProcessID);
                if (hChild) {
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_SIZE:
        if (controller) {
            RECT bounds;
            GetClientRect(hWnd, &bounds);
            controller->put_Bounds(bounds);
        }
		if (wParam == SIZE_MINIMIZED) {
			TrimTree(GetCurrentProcessId());
			TrimTree(nodeProcess.dwProcessId);
		}
        return 0;

    case WM_DESTROY:
        StopNode();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// -------------------- main --------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {

    const wchar_t CLASS_NAME[] = L"AppGui";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
	////////////////////////////
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	////////////////////////////
	// ParseFlags();
	int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(
        GetCommandLineW(),
        &argc
    );
    Config cfg = ParseFlags(argc, argv);
    //std::wcout << L"Root dir: "<< std::endl; - не ЮЗАТЬ std::wcout!!!
	wprintf(L"cfg.paths.root_dir=%ls\n", cfg.paths.root_dir.c_str());
	//	cfg.launcher - отладка
    cfg.logs.launcher = LogMode::FILE_MODE;
	InitLogging(cfg.logs);
	LogLauncherInfo(L"launcher started");
	//wprintf(L"This writed to logfile=%ls\n", cfg.paths.root_dir.c_str());
//////////////////Loging test for launcher
LogLauncherInfo(L"launcher started");
LogLauncherError(L"test launcher error");
///////////////////
	
	

    // ---- start server ----
    StartNode();
	// даём Node стартануть
	Sleep(1000);  // ← обязательно
	
    if (!WaitForPort(9090)) {
        MessageBoxW(NULL, L"Server did not start", L"Error", MB_OK);
    }



    RegisterClassW(&wc);
	SetEnvironmentVariableW(L"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", L"--disable-features=LayoutNGPrinting,Printing --js-flags=\"--max-semi-space-size=8\"");
    hwnd = CreateWindowExW(
        0, CLASS_NAME, L"AppGui",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,
        NULL, NULL, hInstance, NULL
    );
	Sleep(1000);  // ← обязательно

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
        new EnvHandler()
    );

    // ---- loop ----
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}