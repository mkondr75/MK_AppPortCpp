// #include <windows.h>
// #include <string>
// #include <vector>
// #include "config.hpp"

// extern Config cfg;

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
	std::wstring *target_field; // Указатель на поле в Config
	const wchar_t *suggestion;	// Дефолтное значение
	bool target_bool;			// дефолт (лож) - выбор системы, (истина) - пользователь
	bool is_path;				// Флаг необходимости PathStripQuotes/Validation
};

// Контейнер для хранения всех сцен
struct AA_WizardRegistry {
	std::vector<AA_WizardPage> pages;
	size_t current_index = 0;
};

#define IDC_RADIO_SYSTEM 1001
#define IDC_RADIO_CUSTOM 1002
#define AA_ID_EDIT_PATH 2001
#define AA_ID_BTN_NEXT 3003
#define AA_ID_BTN_BACK 3004
#define AA_ID_QUESTION 3001

static size_t g_current_page = 0;
static std::vector<AA_WizardPage> g_pages;

static void AA_UpdateLayout(HWND hwnd) {
	// Логика из твоего прототипа: если выбран кастом, включаем Edit
	BOOL use_custom = SendMessageW(GetDlgItem(hwnd, IDC_RADIO_CUSTOM), BM_GETCHECK, 0, 0) == BST_CHECKED;
	EnableWindow(GetDlgItem(hwnd, AA_ID_EDIT_PATH), use_custom);
}

static void AA_SyncUI(HWND hwnd) {
	if (g_current_page >= g_pages.size()) return;
	const auto &p = g_pages[g_current_page];
	SetWindowTextW(hwnd, p.title);
	SetWindowTextW(GetDlgItem(hwnd, AA_ID_QUESTION), p.question);
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
	// Внутри void AA_SyncUI(HWND hwnd) перед EnableWindow(GetDlgItem(hwnd, AA_ID_BTN_BACK)...
	// target_bool == true (Пользователь), target_bool == false (Система)
	CheckRadioButton(hwnd, IDC_RADIO_SYSTEM, IDC_RADIO_CUSTOM, p.target_bool ? IDC_RADIO_CUSTOM : IDC_RADIO_SYSTEM);
	SetWindowTextW(GetDlgItem(hwnd, AA_ID_EDIT_PATH), p.target_field->empty() ? p.suggestion : p.target_field->c_str());
	AA_UpdateLayout(hwnd); // Мгновенно дизейблим/энейблим EDIT
}
static LRESULT CALLBACK WizardWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HFONT hFont = NULL;
	switch (msg) {
	case WM_CREATE: {
		NONCLIENTMETRICSW ncm{sizeof(NONCLIENTMETRICSW)};
		SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);
		hFont = CreateFontIndirectW(&ncm.lfMessageFont);
		// Статика вопроса
		CreateWindowW(L"STATIC", L"", WS_VISIBLE | WS_CHILD, 20, 20, 600, 20, hwnd, (HMENU)AA_ID_QUESTION, NULL, NULL);
		// Твои бутоны (Radio)
		CreateWindowW(L"BUTTON", L"System Node.js", WS_CHILD | BS_AUTORADIOBUTTON, 20, 50, 400, 20, hwnd, (HMENU)IDC_RADIO_SYSTEM, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Custom Path", WS_CHILD | BS_AUTORADIOBUTTON, 20, 80, 400, 20, hwnd, (HMENU)IDC_RADIO_CUSTOM, NULL, NULL);
		// Поле ввода
		CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 20, 120, 640, 25, hwnd, (HMENU)AA_ID_EDIT_PATH, NULL, NULL);
		// Навигация
		CreateWindowW(L"BUTTON", L"< Back", WS_VISIBLE | WS_CHILD, 350, 210, 100, 30, hwnd, (HMENU)AA_ID_BTN_BACK, NULL, NULL);
		CreateWindowW(L"BUTTON", L"Next >", WS_VISIBLE | WS_CHILD, 460, 210, 100, 30, hwnd, (HMENU)AA_ID_BTN_NEXT, NULL, NULL);
		EnumChildWindows(
			hwnd,
			[](HWND child, LPARAM font) -> BOOL {
				SendMessageW(child, WM_SETFONT, (WPARAM)font, TRUE);
				return TRUE;
			},
			(LPARAM)hFont);
		SendMessageW(GetDlgItem(hwnd, IDC_RADIO_SYSTEM), BM_SETCHECK, BST_CHECKED, 0);
		AA_SyncUI(hwnd);
		AA_UpdateLayout(hwnd);
		return 0;
	}
	case WM_COMMAND: {
		int id = LOWORD(wParam);
		if (id == IDC_RADIO_SYSTEM || id == IDC_RADIO_CUSTOM) AA_UpdateLayout(hwnd);
		if (id == AA_ID_BTN_NEXT) {
			wchar_t buf[MAX_PATH];
			GetDlgItemTextW(hwnd, AA_ID_EDIT_PATH, buf, MAX_PATH);
			*g_pages[g_current_page].target_field = buf;
			HWND hRadioCustom = GetDlgItem(hwnd, IDC_RADIO_CUSTOM);
			g_pages[g_current_page].target_bool = (SendMessageW(hRadioCustom, BM_GETCHECK, 0, 0) == BST_CHECKED);

			if (g_current_page == g_pages.size() - 1) {
				// Если это была последняя страница - уничтожаем окно
				DestroyWindow(hwnd);
			} else {
				g_current_page++;
				AA_SyncUI(hwnd);
			}
		}
		if (id == AA_ID_BTN_BACK) {
			if (g_current_page > 0) {
				// Сохраняем текущий ввод, чтобы он не пропал при возврате "вперед"
				wchar_t buf[MAX_PATH];
				GetDlgItemTextW(hwnd, AA_ID_EDIT_PATH, buf, MAX_PATH);
				*g_pages[g_current_page].target_field = buf;
				g_pages[g_current_page].target_bool = (SendMessageW(GetDlgItem(hwnd, IDC_RADIO_CUSTOM), BM_GETCHECK, 0, 0) == BST_CHECKED);
				g_current_page--;
				AA_SyncUI(hwnd);
			}
		}
		return 0;
	}
	case WM_DESTROY:
		if (hFont) DeleteObject(hFont);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool RunWizard(HINSTANCE hInst) {
	// Инициализация сценария
	// const wchar_t *internal_id; // ID для логов
	// const wchar_t *title;		// Заголовок окна (Caption)
	// const wchar_t *question;	// Текст вопроса (Static text)
	// std::wstring *target_field; // Указатель на поле в Config
	// const wchar_t *suggestion;	// Дефолтное значение
	// bool target_bool;			// дефолт (лож) - выбор системы, (истина) - пользователь
	// bool is_path;				// Флаг необходимости PathStripQuotes/Validation

	g_pages = {{L"node_exe", L"Node Setup", L"Путь к node.exe:", &cfg.node.exe, L"node.exe", false, true}, //
			   {L"node_args", L"Arguments", L"Аргументы запуска:", &cfg.node.args, L"index.js", false, false}};
	g_current_page = 0;

	WNDCLASSW wc{0};
	wc.lpfnWndProc = WizardWndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = L"AA_WizardClass";
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClassW(&wc);

	HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Wizard", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 700, 300, NULL, NULL, hInst, NULL);
	if (!hwnd) return false;

	ShowWindow(hwnd, SW_SHOW);
	MSG msg;
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return true;
}
