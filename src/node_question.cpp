#include "../include/node_question.hpp"

#define IDC_RADIO_SYSTEM 1001
#define IDC_RADIO_CUSTOM 1002
#define IDC_BUTTON_OK 1005
#define AA_ID_EDIT_PATH 2001

static NodeQuestion *g_model = nullptr;
static HWND g_radio_system = NULL;
static HWND g_radio_custom = NULL;
static HWND g_edit_path = NULL; // AA_ добавил статик для быстрого доступа

static void AA_UpdateControls() {
	BOOL use_custom = SendMessageW(g_radio_custom, BM_GETCHECK, 0, 0) == BST_CHECKED;
	if (g_edit_path) EnableWindow(g_edit_path, use_custom);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HFONT hFont = NULL; // AA_ статик для хранения шрифта

	switch (msg) {
	case WM_CTLCOLORSTATIC: {
		HDC hdcStatic = (HDC)wParam;
		SetBkMode(hdcStatic, TRANSPARENT);
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	}
	case WM_CREATE: {
		NONCLIENTMETRICSW ncm{sizeof(NONCLIENTMETRICSW)};
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);
		hFont = CreateFontIndirectW(&ncm.lfMessageFont);

		CreateWindowW(L"STATIC", L"Определите Node.js", WS_VISIBLE | WS_CHILD, 20, 20, 400, 20, hwnd, NULL, NULL, NULL);

		std::wstring system_text = g_model->system_node_path + L"   version " + g_model->system_node_version;

		g_radio_system = CreateWindowW(L"BUTTON", system_text.c_str(), WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 20, 60, 700, 20, hwnd, (HMENU)IDC_RADIO_SYSTEM, NULL, NULL);
		SendMessageW(g_radio_system, WM_SETFONT, (WPARAM)hFont, TRUE);

		g_radio_custom = CreateWindowW(L"BUTTON", L"Использовать пользовательскую Node.js", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON, 20, 100, 400, 20, hwnd, (HMENU)IDC_RADIO_CUSTOM, NULL, NULL);
		SendMessageW(g_radio_custom, WM_SETFONT, (WPARAM)hFont, TRUE);

		HWND hBtnOk = CreateWindowW(L"BUTTON", L"OK", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 260, 210, 100, 30, hwnd, (HMENU)IDC_BUTTON_OK, NULL, NULL);
		SendMessageW(hBtnOk, WM_SETFONT, (WPARAM)hFont, TRUE);

		if (g_model->system_variant_available) {
			SendMessageW(g_radio_system, BM_SETCHECK, BST_CHECKED, 0);
		} else {
			EnableWindow(g_radio_system, FALSE);
			SendMessageW(g_radio_custom, BM_SETCHECK, BST_CHECKED, 0);
		}
		return 0;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_RADIO_SYSTEM:
		case IDC_RADIO_CUSTOM:
			AA_UpdateControls();
			break;
		case IDC_BUTTON_OK: {
			g_model->use_system_variant = (SendMessageW(g_radio_system, BM_GETCHECK, 0, 0) == BST_CHECKED);
			wchar_t buffer[MAX_PATH];
			GetDlgItemTextW(hwnd, AA_ID_EDIT_PATH, buffer, MAX_PATH);
			g_model->custom_node_path = buffer;
			g_model->accepted = true;
			DestroyWindow(hwnd);
			break;
		}
		}
		return 0;
	}
	case WM_DESTROY:
		if (hFont) DeleteObject(hFont); // AA_ чистка GDI
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool ShowNodeQuestion(HINSTANCE hInstance, NodeQuestion &model) {
	g_model = &model;
	const wchar_t CLASS_NAME[] = L"NodeQuestionWindow";
	WNDCLASSW wc{0};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassW(&wc);

	HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Node.js setup", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 700, 300, NULL, NULL, hInstance, NULL);
	if (!hwnd) return false;

	// AA_NodePathEdit
	g_edit_path = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", model.custom_node_path.c_str(), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 20, 150, 640, 25, hwnd, (HMENU)AA_ID_EDIT_PATH, hInstance, NULL);

	// Подтягиваем шрифт к Edit (был системный Classic)
	NONCLIENTMETRICSW ncm{sizeof(NONCLIENTMETRICSW)};
	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);
	HFONT hTempFont = CreateFontIndirectW(&ncm.lfMessageFont);
	SendMessageW(g_edit_path, WM_SETFONT, (WPARAM)hTempFont, TRUE);

	AA_UpdateControls();

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	DeleteObject(hTempFont);
	return model.accepted;
}
