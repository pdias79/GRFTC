#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <array>
#include <cstdio>
#include "numeric.h"

namespace {
constexpr int kPanelWidth = 300;
constexpr int kFirstEdit = 100;
constexpr wchar_t kClassName[] = L"GRFTCWindow";
constexpr std::array<const wchar_t*, 5> kLabels = {
    L"Puissance", L"Temp\u00e9rature", L"FDH", L"D\u00e9bit relatif", L"FDHE1"
};
struct App {
    std::array<HWND, 5> labels{};
    std::array<HWND, 5> edits{};
    HWND result{};
    HFONT font{};
    bool ready = false;
};
HWND AddControl(HWND parent, const wchar_t* type, const wchar_t* text,
                DWORD style, int id, HFONT font) {
    HWND control = CreateWindowExW(std::wcscmp(type, L"EDIT") == 0 ? WS_EX_CLIENTEDGE : 0,
        type, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 0, 0, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
    if (control) SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return control;
}
void UpdateResult(App& app) {
    if (!app.ready) return;
    std::array<double, 5> values{};
    for (size_t i = 0; i < values.size(); ++i) {
        wchar_t buffer[128]{};
        GetWindowTextW(app.edits[i], buffer, 128);
        if (!ParseNumber(buffer, values[i])) {
            SetWindowTextW(app.result, L"RFTC=\u2014 (saisie invalide)");
            return;
        }
    }
    const double result = EvaluateRftc(values[0], values[1]);
    wchar_t text[128]{};
    if (std::isfinite(result)) std::swprintf(text, 128, L"RFTC=%.12g", result);
    else std::swprintf(text, 128, L"RFTC=\u2014 (hors limites)");
    SetWindowTextW(app.result, text);
}
void Layout(HWND window, App& app) {
    RECT r{};
    GetClientRect(window, &r);
    for (size_t i = 0; i < app.edits.size(); ++i) {
        const int y = 24 + static_cast<int>(i) * 66;
        MoveWindow(app.labels[i], 20, y, 260, 22, TRUE);
        MoveWindow(app.edits[i], 20, y + 24, 260, 28, TRUE);
    }
    MoveWindow(app.result, 20, std::max(370L, r.bottom - 54), 270, 30, TRUE);
    InvalidateRect(window, nullptr, TRUE);
}
LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* app = reinterpret_cast<App*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        app = static_cast<App*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    switch (message) {
    case WM_CREATE:
        app->font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        for (size_t i = 0; i < app->edits.size(); ++i) {
            app->labels[i] = AddControl(window, L"STATIC", kLabels[i], 0, 0, app->font);
            app->edits[i] = AddControl(window, L"EDIT", L"0", WS_TABSTOP | ES_AUTOHSCROLL,
                kFirstEdit + static_cast<int>(i), app->font);
            if (!app->labels[i] || !app->edits[i]) return -1;
            SendMessageW(app->edits[i], EM_SETLIMITTEXT, 100, 0);
        }
        app->result = AddControl(window, L"STATIC", L"RFTC=0", SS_LEFT, 0, app->font);
        if (!app->result) return -1;
        app->ready = true;
        Layout(window, *app);
        return 0;
    case WM_COMMAND:
        if (app && HIWORD(wParam) == EN_CHANGE) UpdateResult(*app);
        return 0;
    case WM_SIZE:
        if (app && app->ready) Layout(window, *app);
        return 0;
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize = {700, 480};
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT client{};
        GetClientRect(window, &client);
        RECT canvas{kPanelWidth + 1, 0, client.right, client.bottom};
        FillRect(dc, &canvas, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
        MoveToEx(dc, kPanelWidth, 0, nullptr);
        LineTo(dc, kPanelWidth, client.bottom);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = kClassName;
    if (!RegisterClassW(&wc)) return 1;
    App app;
    HWND window = CreateWindowExW(WS_EX_CONTROLPARENT, kClassName, L"GRFTC",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
        900, 600, nullptr, nullptr, instance, &app);
    if (!window) {
        MessageBoxW(nullptr, L"Impossible de cr\u00e9er la fen\u00eatre.", L"GRFTC", MB_ICONERROR);
        return 1;
    }
    ShowWindow(window, show);
    SetFocus(app.edits[0]);
    MSG message{};
    int status;
    while ((status = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        if (!IsDialogMessageW(window, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    return status == -1 ? 1 : static_cast<int>(message.wParam);
}
