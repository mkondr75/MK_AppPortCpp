// edge_rt.hpp
#pragma once

#include <windows.h>
// #include <string>
#include "config.hpp"

// Инициализация и очистка "стелс" COM-окружения
bool InitStealthOLE();
void CleanupStealthOLE();

// Сигнатура коллбэка для приема сообщений из JS (мост JS -> C++)
using JSMessageHandler = void (*)(const std::wstring &message);

// Класс-обертка для окна WebView2
class EdgeRT {
  public:
	EdgeRT(HINSTANCE hInstance);
	~EdgeRT();
	// Добавляем в секцию public
	using ExitCallback = void (*)();
	void SetExitHandler(ExitCallback handler);

	// Регистрация класса окна, создание HWND и старт WebView2
	bool Start(const Config &cfg, int nCmdShow);

	// Доступ к хэндлу окна
	HWND GetHWND() const;

	// --- Мост C++ <-> JS ---
	// Отправка сообщения/скрипта в DOM (C++ -> JS)
	bool SendToJS(const std::wstring &message);
	bool ExecuteJS(const std::wstring &script);

	// Регистрация обработчика входящих сообщений (JS -> C++)
	void SetMessageHandler(JSMessageHandler handler);

	// Инкапсулированный обработчик сообщений окна
	static LRESULT CALLBACK AA_WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  private:
	HWND m_hwnd;
	HINSTANCE m_hInstance;
	JSMessageHandler m_jsHandler = nullptr;
	// В секцию private
	ExitCallback m_exitHandler = nullptr;

	// Внутренняя инициализация WebView2
	void InitializeWebView(const Config &cfg);
	void ApplyDarkMode();
};