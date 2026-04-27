g++ main.cpp -o Sharpgui.exe -DUNICODE -D_UNICODE -Iinclude -Llib WebView2Loader.dll -lole32 -luuid -luser32 -lgdi32 -lws2_32
сборка без клнсольного окна в бэке -mwindows
g++ main.cpp -o Sharpgui.exe -DUNICODE -D_UNICODE -mwindows -Iinclude -Llib WebView2Loader.dll -lole32 -luuid -luser32 -lgdi32 -lws2_32
и если надо открываеш прогамно:

	wprintf(L"comspec=%ls\n", comspec); - этого в консоле не будет
AllocConsole();
freopen("CONOUT$", "w", stdout);
freopen("CONOUT$", "w", stderr);
	wprintf(L"SystemRoot=%ls\n", sysroot); - это уже видно
