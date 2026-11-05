#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
#include "statepoint.h"
#include "configurations.h"
#include "RFTC.h"
#include "initial_curve.h"
#include "ao.h"
#include "canvas.h"

namespace {
constexpr int kPanelWidth = 300;
constexpr int kFirstEdit = 100;
constexpr wchar_t kClassName[] = L"GRFTCWindow";
constexpr int kConfigurationId = 90;
struct App {
    std::array<HWND, kChamps.size()> labels{};
    std::array<HWND, kChamps.size()> edits{};
    std::array<std::wstring, kChamps.size()> lastValid{};
    HWND configurationLabel{};
    HWND configuration{};
    std::vector<std::string> configurations;
    HWND result{};
    HWND aoResult{};
    HWND canvasWindow{};
    Canvas canvas;
    HFONT font{};
    bool ready = false;
    bool showingError = false;
};
HWND AddControl(HWND parent, const wchar_t* type, const wchar_t* text,
                DWORD style, int id, HFONT font) {
    HWND control = CreateWindowExW(std::wcscmp(type, L"EDIT") == 0 ? WS_EX_CLIENTEDGE : 0,
        type, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 0, 0, parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);
    if (control) SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return control;
}
bool LoadConfigurations(HWND window, App& app) {
    std::vector<wchar_t> path(32768);
    const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
    if (length == 0 || length >= path.size()) return false;
    const auto file = std::filesystem::path(std::wstring(path.data(), length))
        .parent_path() / L"configurations.txt";
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        MessageBoxW(window, L"Le fichier configurations.txt est introuvable ou illisible. "
            L"Placez-le dans le m\u00eame dossier que GRFTC.exe.", L"Configurations", MB_OK | MB_ICONERROR);
        return false;
    }
    app.configurations = ReadConfigurations(input);
    if (input.bad() || app.configurations.empty()) {
        MessageBoxW(window, L"Le fichier configurations.txt doit contenir au moins un nom.",
            L"Configurations", MB_OK | MB_ICONERROR);
        return false;
    }
    for (const auto& name : app.configurations) {
        const int lengthUtf8 = static_cast<int>(name.size());
        const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
            name.data(), lengthUtf8, nullptr, 0);
        if (count == 0) {
            MessageBoxW(window, L"Enregistrez configurations.txt au format UTF-8.",
                L"Configurations", MB_OK | MB_ICONERROR);
            return false;
        }
        std::wstring label(static_cast<std::size_t>(count), L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
            name.data(), lengthUtf8, label.data(), count);
        const LRESULT result = SendMessageW(app.configuration, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(label.c_str()));
        if (result == CB_ERR || result == CB_ERRSPACE) return false;
    }
    SendMessageW(app.configuration, CB_SETCURSEL, 0, 0);
    return true;
}
std::wstring FieldText(HWND edit) {
    wchar_t buffer[128]{};
    GetWindowTextW(edit, buffer, 128);
    return buffer;
}
void ValidateOnExit(HWND window, App& app, std::size_t index) {
    if (!app.ready || app.showingError) return;
    double value = 0;
    const std::wstring text = FieldText(app.edits[index]);
    if (ValidateField(index, text, value)) {
        app.lastValid[index] = text;
        return;
    }
    std::wstring message = kChamps[index].nom;
    message += L" : saisissez un nombre r\u00e9el fini (double).";
    if (const auto* limits = kChamps[index].limites) {
        wchar_t range[128]{};
        std::swprintf(range, 128, L"\nValeur attendue entre %.12g et %.12g, bornes incluses.",
            limits->min, limits->max);
        message += range;
    }
    app.showingError = true;
    MessageBoxW(window, message.c_str(), L"Valeur invalide", MB_OK | MB_ICONERROR);
    // Restore only after the user dismisses the error; EN_CHANGE refreshes RFTC.
    SetWindowTextW(app.edits[index], app.lastValid[index].c_str());
    app.showingError = false;
}
void UpdateResult(App& app) {
    if (!app.ready) return;
    std::array<double, kChamps.size()> values{};
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!ValidateField(i, FieldText(app.edits[i]), values[i])) {
            SetWindowTextW(app.result, L"RFTC=\u2014 (saisie invalide)");
            return;
        }
    }
    const LRESULT selection = SendMessageW(app.configuration, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR || static_cast<std::size_t>(selection) >= app.configurations.size()) {
        SetWindowTextW(app.result, L"RFTC=\u2014 (configuration invalide)");
        return;
    }
    // Give the char* API a mutable UTF-8 copy, preserving the original list.
    std::string configuration = app.configurations[static_cast<std::size_t>(selection)];
    const double result = RFTC(configuration.data(), values[0], values[1], values[2],
        values[3], values[4], values[5]);
    wchar_t text[128]{};
    if (std::isfinite(result)) std::swprintf(text, 128, L"RFTC=%.12g", result);
    else std::swprintf(text, 128, L"RFTC=\u2014 (hors limites)");
    SetWindowTextW(app.result, text);
}
void UpdateAO(App& app) {
    if (!app.aoResult) return;
    const double percent = AO(app.canvas.curve) * 100.0;
    wchar_t text[64]{};
    // Suppress a negative zero after rounding to one decimal place.
    std::swprintf(text, 64, L"AO=%.1f %%", std::abs(percent) < 0.05 ? 0.0 : percent);
    for (auto& c : text) if (c == L'.') c = L',';
    SetWindowTextW(app.aoResult, text);
}
void Layout(HWND window, App& app) {
    RECT r{};
    GetClientRect(window, &r);
    MoveWindow(app.configurationLabel, 20, 24, 260, 22, TRUE);
    MoveWindow(app.configuration, 20, 48, 260, 200, TRUE);
    for (size_t i = 0; i < app.edits.size(); ++i) {
        const int y = 104 + static_cast<int>(i) * 60;
        MoveWindow(app.labels[i], 20, y, 260, 22, TRUE);
        MoveWindow(app.edits[i], 20, y + 24, 260, 28, TRUE);
    }
    MoveWindow(app.result, 20, std::max(480L, r.bottom - 84), 270, 30, TRUE);
    MoveWindow(app.aoResult, 20, std::max(510L, r.bottom - 54), 270, 30, TRUE);
    MoveWindow(app.canvasWindow, kPanelWidth + 1, 0,
        std::max(0L, r.right - kPanelWidth - 1), r.bottom, TRUE);
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
        app->configurationLabel = AddControl(window, L"STATIC", L"Configuration", 0, 0, app->font);
        app->configuration = AddControl(window, L"COMBOBOX", L"",
            WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, kConfigurationId, app->font);
        if (!app->configurationLabel || !app->configuration || !LoadConfigurations(window, *app)) return -1;
        for (size_t i = 0; i < app->edits.size(); ++i) {
            app->labels[i] = AddControl(window, L"STATIC", kChamps[i].nom, 0, 0, app->font);
            app->edits[i] = AddControl(window, L"EDIT", kChamps[i].initial, WS_TABSTOP | ES_AUTOHSCROLL,
                kFirstEdit + static_cast<int>(i), app->font);
            if (!app->labels[i] || !app->edits[i]) return -1;
            app->lastValid[i] = kChamps[i].initial;
            SendMessageW(app->edits[i], EM_SETLIMITTEXT, 100, 0);
        }
        app->result = AddControl(window, L"STATIC", L"RFTC=0", SS_LEFT, 0, app->font);
        if (!app->result) return -1;
        app->aoResult = AddControl(window, L"STATIC", L"", SS_LEFT, 0, app->font);
        if (!app->aoResult) return -1;
        InitializeCurve(app->canvas.curve);
        UpdateAO(*app);
        app->canvasWindow = CreateWindowExW(0, kCanvasClass, L"", WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, window, nullptr, GetModuleHandleW(nullptr), &app->canvas);
        if (!app->canvasWindow) return -1;
        app->ready = true;
        Layout(window, *app);
        UpdateResult(*app);
        return 0;
    case WM_COMMAND:
        if (app && app->ready) {
            const int id = LOWORD(wParam);
            const int notification = HIWORD(wParam);
            if (id >= kFirstEdit && id < kFirstEdit + static_cast<int>(kChamps.size())) {
                if (notification == EN_CHANGE) UpdateResult(*app);
                if (notification == EN_KILLFOCUS)
                    ValidateOnExit(window, *app, static_cast<std::size_t>(id - kFirstEdit));
            } else if (id == kConfigurationId && notification == CBN_SELCHANGE) {
                UpdateResult(*app);
            }
        }
        return 0;
    case kCurveChanged:
        if (app) UpdateAO(*app);
        return 0;
    case WM_SIZE:
        if (app && app->ready) Layout(window, *app);
        return 0;
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize = {700, 630};
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT client{};
        GetClientRect(window, &client);
        MoveToEx(dc, kPanelWidth, 0, nullptr);
        LineTo(dc, kPanelWidth, client.bottom);
        EndPaint(window, &paint);
        return 0;
    }
    case WM_CLOSE:
        if (app) app->ready = false;
        DestroyWindow(window);
        return 0;
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
    if (!RegisterClassW(&wc) || !RegisterCanvas(instance)) return 1;
    App app;
    HWND window = CreateWindowExW(WS_EX_CONTROLPARENT, kClassName, L"GRFTC",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
        900, 680, nullptr, nullptr, instance, &app);
    if (!window) {
        MessageBoxW(nullptr, L"Impossible de cr\u00e9er la fen\u00eatre.", L"GRFTC", MB_ICONERROR);
        return 1;
    }
    ShowWindow(window, show);
    SetFocus(app.configuration);
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
