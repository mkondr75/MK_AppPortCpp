/// \file wk_flags.cpp
/// \brief Парсим флаги командной строки заполняем структуры значениями.
/// \see header_file.hpp
#include "..\include\wk_flags.hpp"
#include "config.hpp"
extern Config cfg;
/// \brief Что делает эта функция.
/// \param  int argc, wchar_t *argv[] - под винду возможно нужен врапер для мультисис.
/// \return Значение при успехе/ошибке.
Config ParseFlags(int argc, wchar_t *argv[]) {
	// Config cfg;
	cfg.logs.launcher = LogMode::OFF; //
	cfg.logs.node = LogMode::OFF;	  //
	cfg.logs.webview = LogMode::OFF;  //
	cfg.paths.setup = SetupMode::OFF; //

	for (int i = 1; i < argc; ++i) {
		if (wcscmp(argv[i], L"--log-launcher=cons") == 0) {
			cfg.logs.launcher = LogMode::CONS;
		} else if (wcscmp(argv[i], L"--log-launcher=file") == 0) {
			cfg.logs.launcher = LogMode::FILE_MODE;
		} else if (wcscmp(argv[i], L"--log-node=cons") == 0) {
			cfg.logs.node = LogMode::CONS;
		} else if (wcscmp(argv[i], L"--log-node=file") == 0) {
			cfg.logs.node = LogMode::FILE_MODE;
		} else if (wcscmp(argv[i], L"--setup") == 0) {
			cfg.paths.setup = SetupMode::FLAGS;
		}
	}
	return cfg;
}