#include "../include/node_question.hpp"

#include <commdlg.h>
#include <dwmapi.h>
// #include <commctrl.h>
// #include <uxtheme.h>

#define IDC_RADIO_SYSTEM 1001
#define IDC_RADIO_CUSTOM 1002

#define IDC_EDIT_PATH 1003
#define IDC_BUTTON_BROWSE 1004

#define IDC_BUTTON_OK 1005

static NodeQuestion *g_model = nullptr;

static HWND g_radio_system = NULL;
static HWND g_radio_custom = NULL;
static HWND g_edit_path = NULL;

// static HFONT g_font = NULL;
// static void ApplyFont(HWND hwnd, HFONT g_font)
// static void ApplyFont(HWND hwnd)
// {    SendMessageW(hwnd, WM_SETFONT, (WPARAM)g_font, TRUE);}
static void UpdateControls()
{
    BOOL use_system = SendMessageW(g_radio_system, BM_GETCHECK, 0, 0) == BST_CHECKED;
    EnableWindow(g_edit_path, !use_system);
}

static void BrowseForNode(HWND hwnd)
{
    wchar_t file_name[MAX_PATH] = L"";
    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0";
    ofn.lpstrFile = file_name;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST |
                OFN_PATHMUSTEXIST;

    if (!GetOpenFileNameW(&ofn))
        return;

    SetWindowTextW(g_edit_path, file_name);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;

        // Делаем фон текста прозрачным, чтобы он не перекрывал заливку кистью
        SetBkMode(hdcStatic, TRANSPARENT);

        // Возвращаем кисть, цвет которой совпадает с фоном твоего окна (белый)
        return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
    }
    case WM_CREATE:
    {
        // bool themed = IsThemeActive();
        // wchar_t dbg[128];
        // swprintf_s(dbg, L"themes=%d", themed); // #include <uxtheme.h> + в смаке флаг uxtheme в target_link_libraries

        // g_font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        NONCLIENTMETRICSW ncm{};
        ncm.cbSize = sizeof(NONCLIENTMETRICSW);
        SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0);
        HFONT g_font = CreateFontIndirectW(&ncm.lfMessageFont);
        CreateWindowW(
            L"STATIC",
            L"Определите Node.js",
            WS_VISIBLE | WS_CHILD,
            20,
            20,
            400,
            20,
            hwnd,
            NULL,
            NULL,
            NULL);

        std::wstring system_text =
            g_model->system_node_path +
            L"  version " +
            g_model->system_node_version;

        g_radio_system =
            CreateWindowW(
                L"BUTTON",
                system_text.c_str(),
                WS_VISIBLE |
                    WS_CHILD |
                    BS_AUTORADIOBUTTON,
                20,
                60,
                700,
                20,
                hwnd,
                (HMENU)IDC_RADIO_SYSTEM,
                NULL,
                NULL);
        // ApplyFont(g_edit_path);
        SendMessageW(g_radio_system, WM_SETFONT, (WPARAM)g_font, TRUE);
        g_radio_custom =
            CreateWindowW(
                L"BUTTON",
                L"Использовать пользовательскую Node.js",
                WS_VISIBLE |
                    WS_CHILD |
                    BS_AUTORADIOBUTTON,
                20,
                100,
                300,
                20,
                hwnd,
                (HMENU)IDC_RADIO_CUSTOM,
                NULL,
                NULL);
        // ApplyFont(g_radio_custom);
        SendMessageW(g_radio_custom, WM_SETFONT, (WPARAM)g_font, TRUE);
        g_edit_path =
            CreateWindowW(
                L"EDIT",
                g_model->custom_node_path.c_str(),
                WS_VISIBLE |
                    WS_CHILD |
                    WS_BORDER |
                    ES_AUTOHSCROLL,
                40,
                130,
                500,
                24,
                hwnd,
                (HMENU)IDC_EDIT_PATH,
                NULL,
                NULL);
        // ApplyFont(g_edit_path);
        SendMessageW(g_edit_path, WM_SETFONT, (WPARAM)g_font, TRUE);
        CreateWindowW(
            L"BUTTON",
            L"...",
            WS_VISIBLE | WS_CHILD,
            550,
            130,
            40,
            24,
            hwnd,
            (HMENU)IDC_BUTTON_BROWSE,
            NULL,
            NULL);

        CreateWindowW(
            L"BUTTON",
            L"OK",
            WS_VISIBLE | WS_CHILD,
            260,
            190,
            100,
            30,
            hwnd,
            (HMENU)IDC_BUTTON_OK,
            NULL,
            NULL);

        if (g_model->system_variant_available)
        {
            SendMessageW(
                g_radio_system,
                BM_SETCHECK,
                BST_CHECKED,
                0);
        }
        else
        {
            EnableWindow(
                g_radio_system,
                FALSE);

            SendMessageW(
                g_radio_custom,
                BM_SETCHECK,
                BST_CHECKED,
                0);
        }

        UpdateControls();

        return 0;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_RADIO_SYSTEM:
        case IDC_RADIO_CUSTOM:
        {
            UpdateControls();

            return 0;
        }

        case IDC_BUTTON_BROWSE:
        {
            BrowseForNode(hwnd);

            return 0;
        }

        case IDC_BUTTON_OK:
        {
            g_model->use_system_variant =
                SendMessageW(
                    g_radio_system,
                    BM_GETCHECK,
                    0,
                    0) == BST_CHECKED;

            wchar_t buffer[1024];

            GetWindowTextW(
                g_edit_path,
                buffer,
                1024);

            g_model->custom_node_path =
                buffer;

            g_model->accepted = true;

            DestroyWindow(hwnd);

            return 0;
        }
        }

        return 0;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);

        return 0;
    }
    }

    return DefWindowProcW(
        hwnd,
        msg,
        wParam,
        lParam);
}

bool ShowNodeQuestion(HINSTANCE hInstance, NodeQuestion &model)
{
    g_model = &model;
    const wchar_t CLASS_NAME[] = L"NodeQuestionWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    /////
    // INITCOMMONCONTROLSEX icex;
    // icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    // icex.dwICC = ICC_STANDARD_CLASSES;
    // InitCommonControlsEx(&icex);
    /////

    RegisterClassW(&wc);
    HWND hwnd =
        CreateWindowExW(
            0,
            CLASS_NAME,
            L"Node.js setup",
            WS_OVERLAPPED |
                WS_CAPTION |
                WS_SYSMENU,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            700,
            300,
            NULL,
            NULL,
            hInstance,
            NULL);

    if (!hwnd)
        return false;
    // ДВМапи
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &useDarkMode, sizeof(useDarkMode));
    //
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return model.accepted;
}