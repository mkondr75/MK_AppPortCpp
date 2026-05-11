#include <windows.h>
#include <string>
#include <vector>
#include "config.hpp"
#include "wizard_engine.hpp"

extern Config cfg;
// Структура страницы визарда
struct AA_WizardPage {
	const wchar_t *internal_id; // ID для логов
	const wchar_t *title;		// Заголовок окна (Caption)
	const wchar_t *question;	// Текст вопроса (Static text)
	// std::wstring *target_field; // Указатель на поле в Config
	const std::wstring *readonly_cfg_field; // cfg readonly
	std::wstring *wizard_field;				// cfg_wizard mutable
	const wchar_t *suggestion;				// Дефолтное значение
	// bool target_bool; // дефолт (лож) - выбор системы, (истина) - пользователь
	bool is_path; // Флаг необходимости PathStripQuotes/Validation
	int source_mode;
	// reflection/binding metadata
	// что показать
	// откуда читать
	// куда писать
	// как визуализировать
	// как интерпретировать выбор
};

// Контейнер для хранения всех сцен
struct AA_WizardRegistry {
	std::vector<AA_WizardPage> pages;
	size_t current_index = 0;
};
enum class AA_SourceMode { SYSTEM, USER };
static Config g_cfg_wizard;
static AA_SourceMode g_node_exe_source = AA_SourceMode::SYSTEM;
static AA_SourceMode g_node_args_source = AA_SourceMode::SYSTEM;
#define IDC_RADIO_SYSTEM 1001
#define IDC_RADIO_CUSTOM 1002
#define AA_ID_EDIT_PATH 2001
#define AA_ID_BTN_NEXT 3003
#define AA_ID_BTN_BACK 3004
#define AA_ID_QUESTION 3001
static size_t g_current_page = 0;
static std::vector<AA_WizardPage> g_pages;
static HFONT g_hFont = NULL;
static void AA_UpdateLayout(HWND hwnd) {
	BOOL use_custom = SendMessageW(GetDlgItem(hwnd, IDC_RADIO_CUSTOM), BM_GETCHECK, 0, 0) == BST_CHECKED;
	EnableWindow(GetDlgItem(hwnd, AA_ID_EDIT_PATH), use_custom);
}

static void AA_SyncUI(HWND hwnd) {
	if (g_current_page >= g_pages.size()) return;
	SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0);
	const auto &p = g_pages[g_current_page];
	SetWindowTextW(hwnd, p.title);
	// Улучшение: Инвалидация статики перед сменой текста для чистого рендеринга без "теней"
	HWND hQuestion = GetDlgItem(hwnd, AA_ID_QUESTION);
	InvalidateRect(hQuestion, NULL, TRUE);
	SetWindowTextW(hQuestion, p.question);
	ShowWindow(GetDlgItem(hwnd, IDC_RADIO_SYSTEM), SW_SHOW);
	ShowWindow(GetDlgItem(hwnd, IDC_RADIO_CUSTOM), SW_SHOW);
	// Смена текста кнопки на последнем шаге
	HWND hBtnNext = GetDlgItem(hwnd, AA_ID_BTN_NEXT);
	if (g_current_page == g_pages.size() - 1) {
		SetWindowTextW(hBtnNext, L"Finish");
	} else {
		SetWindowTextW(hBtnNext, L"Next >");
	}
	EnableWindow(GetDlgItem(hwnd, AA_ID_BTN_BACK), g_current_page > 0);
	CheckRadioButton(hwnd, IDC_RADIO_SYSTEM, IDC_RADIO_CUSTOM, p.source_mode == (int)AA_SourceMode::USER ? IDC_RADIO_CUSTOM : IDC_RADIO_SYSTEM);
	SetWindowTextW(GetDlgItem(hwnd, AA_ID_EDIT_PATH), p.wizard_field->empty() ? (p.readonly_cfg_field->empty() ? p.suggestion : p.readonly_cfg_field->c_str()) : p.wizard_field->c_str());
	AA_UpdateLayout(hwnd);
	SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0);
	// Улучшение: Добавлен RDW_UPDATENOW для мгновенного исключения артефактов
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
static void AA_SaveCurrentPage(HWND hwnd) {
	if (g_current_page >= g_pages.size()) return;
	wchar_t buf[MAX_PATH];
	GetDlgItemTextW(hwnd, AA_ID_EDIT_PATH, buf, MAX_PATH);
	*g_pages[g_current_page].wizard_field = buf;
	HWND hRadioCustom = GetDlgItem(hwnd, IDC_RADIO_CUSTOM);
	g_pages[g_current_page].source_mode = (SendMessageW(hRadioCustom, BM_GETCHECK, 0, 0) == BST_CHECKED) ? (int)AA_SourceMode::USER : (int)AA_SourceMode::SYSTEM;
	if (g_current_page == 0) { g_node_exe_source = (g_pages[g_current_page].source_mode == (int)AA_SourceMode::USER) ? AA_SourceMode::USER : AA_SourceMode::SYSTEM; }
	if (g_current_page == 1) { g_node_args_source = (g_pages[g_current_page].source_mode == (int)AA_SourceMode::USER) ? AA_SourceMode::USER : AA_SourceMode::SYSTEM; }
}

static bool AA_ValidateWizard() {
	// cfg_wizard -> validate -> cfg
	// TODO:
	// file exists
	// runtime probe
	// version check
	return true;
}
static void AA_CommitWizardToCfg() {
	// cfg readonly, wizard mutable
	// cfg_wizard -> validate -> cfg
	cfg = g_cfg_wizard;
}

static LRESULT CALLBACK WizardWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	// Улучшение: Добавлен WM_CTLCOLORBTN для корректной прозрачности Radio Buttons
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN: {
		HDC hdcStatic = (HDC)wParam;
		SetBkMode(hdcStatic, TRANSPARENT);
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
	}
	case WM_CREATE: {
		NONCLIENTMETRICSW ncm{sizeof(NONCLIENTMETRICSW)};
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);
		// CreateFontIndirectW(&ncm.lfMessageFont);
		wcscpy_s(ncm.lfMessageFont.lfFaceName, L"Segoe UI");
		// Улучшение: Высота -18 для Segoe UI на Win11
		ncm.lfMessageFont.lfHeight = -18;
		ncm.lfMessageFont.lfQuality = CLEARTYPE_QUALITY;
		g_hFont = CreateFontIndirectW(&ncm.lfMessageFont);
		// Статика вопроса
		HWND hQuestion = CreateWindowW(L"STATIC", L"", WS_VISIBLE | WS_CHILD, 20, 20, 640,
									   30, // Улучшение: h=30 вместо 24 для воздуха
									   hwnd, (HMENU)AA_ID_QUESTION, NULL, NULL);
		SendMessageW(hQuestion, WM_SETFONT, (WPARAM)g_hFont, TRUE);
		// Твои бутоны (Radio)
		HWND hRadioSystem = CreateWindowW(L"BUTTON", L"System",
										  // WS_CHILD | BS_AUTORADIOBUTTON,
										  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 20, 60, 500,
										  30, // Улучшение: h=30 вместо 24
										  hwnd, (HMENU)IDC_RADIO_SYSTEM, NULL, NULL);
		SendMessageW(hRadioSystem, WM_SETFONT, (WPARAM)g_hFont, TRUE);
		HWND hRadioCustom = CreateWindowW(L"BUTTON", L"Custom",
										  // WS_CHILD | BS_AUTORADIOBUTTON,
										  WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 20,
										  100, // Скорректирован Y-offset под новые высоты
										  500,
										  30, // Улучшение: h=30 вместо 24
										  hwnd, (HMENU)IDC_RADIO_CUSTOM, NULL, NULL);
		SendMessageW(hRadioCustom, WM_SETFONT, (WPARAM)g_hFont, TRUE);
		// Поле ввода
		HWND hEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 20,
									 145, // Скорректирован Y-offset
									 640,
									 32, // Улучшение: h=32 для комфортного ввода
									 hwnd, (HMENU)AA_ID_EDIT_PATH, NULL, NULL);
		SendMessageW(hEdit, WM_SETFONT, (WPARAM)g_hFont, TRUE);
		// Навигация
		HWND hBtnBack = CreateWindowW(L"BUTTON", L"< Back", WS_VISIBLE | WS_CHILD, 330,
									  225, // Скорректирован Y-offset
									  140,
									  38, // Улучшение: h=38 для Win11 Button Style
									  hwnd, (HMENU)AA_ID_BTN_BACK, NULL, NULL);
		SendMessageW(hBtnBack, WM_SETFONT, (WPARAM)g_hFont, TRUE);
		HWND hBtnNext = CreateWindowW(L"BUTTON", L"Next >", WS_VISIBLE | WS_CHILD, 490,
									  225, // Скорректирован Y-offset
									  140,
									  38, // Улучшение: h=38
									  hwnd, (HMENU)AA_ID_BTN_NEXT, NULL, NULL);
		SendMessageW(hBtnNext, WM_SETFONT, (WPARAM)g_hFont, TRUE);
		CheckRadioButton(hwnd, IDC_RADIO_SYSTEM, IDC_RADIO_CUSTOM, IDC_RADIO_SYSTEM);
		AA_SyncUI(hwnd);
		AA_UpdateLayout(hwnd);
		return 0;
	}
	case WM_COMMAND: {
		int id = LOWORD(wParam);
		if (id == IDC_RADIO_SYSTEM || id == IDC_RADIO_CUSTOM) AA_UpdateLayout(hwnd);
		if (id == AA_ID_BTN_NEXT) {
			AA_SaveCurrentPage(hwnd);
			if (g_current_page == g_pages.size() - 1) {
				if (AA_ValidateWizard()) {
					AA_CommitWizardToCfg();
					DestroyWindow(hwnd);
				}
			} else {
				g_current_page++;
				AA_SyncUI(hwnd);
			}
		}
		if (id == AA_ID_BTN_BACK) {
			if (g_current_page > 0) {
				AA_SaveCurrentPage(hwnd);
				g_current_page--;
				AA_SyncUI(hwnd);
			}
		}
		return 0;
	}
	case WM_DESTROY:
		if (g_hFont) DeleteObject(g_hFont);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool RunWizard(HINSTANCE hInst) {
	g_cfg_wizard = cfg;
	g_pages = {{L"node_exe", L"Node Setup", L"Путь к node.exe:", &cfg.node.exe, &g_cfg_wizard.node.exe, L"node.exe", true, (int)AA_SourceMode::SYSTEM},
			   {L"node_args", L"Arguments", L"Аргументы запуска:", &cfg.node.args, &g_cfg_wizard.node.args, L"index.js", false, (int)AA_SourceMode::SYSTEM}};
	g_current_page = 0;
	WNDCLASSW wc{0};
	wc.lpfnWndProc = WizardWndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = L"AA_WizardClass";
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassW(&wc);
	HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Wizard",
								// Улучшение: Добавлен WS_CLIPCHILDREN для устранения мерцания при перерисовке
								WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 700,
								350, // Немного увеличена высота окна под новые метрики
								NULL, NULL, hInst, NULL);

	if (!hwnd) return false;
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return true;
}