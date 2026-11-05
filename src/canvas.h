#pragma once
#include <windowsx.h>
#include "curve.h"

namespace {
constexpr wchar_t kCanvasClass[] = L"GRFTCCanvas";
constexpr UINT kCurveChanged = WM_APP + 1;
struct Canvas {
    Curve curve;
    bool drawing = false;
    int previousRow = 0;
    double previousX = 0;
};
RECT PlotRect(HWND window) {
    RECT r{};
    GetClientRect(window, &r);
    r.left += 36;
    r.top += 24;
    r.right -= 24;
    r.bottom -= 36;
    return r;
}
void StopDrawing(HWND window, Canvas& canvas) {
    canvas.drawing = false;
    if (GetCapture() == window) ReleaseCapture();
}
void DrawAt(HWND window, Canvas& canvas, LPARAM position, bool start) {
    const RECT r = PlotRect(window);
    if (r.right <= r.left || r.bottom <= r.top) return;
    const double x = GraphXFromFraction(static_cast<double>(GET_X_LPARAM(position) - r.left)
        / (r.right - r.left));
    const double y = std::clamp(static_cast<double>(GET_Y_LPARAM(position) - r.top)
        / (r.bottom - r.top), 0.0, 1.0);
    const int row = static_cast<int>(y * Curve::kLast + 0.5);
    canvas.curve.DrawSegment(start ? row : canvas.previousRow,
        start ? x : canvas.previousX, row, x);
    canvas.previousRow = row;
    canvas.previousX = x;
    InvalidateRect(window, nullptr, FALSE);
    SendMessageW(GetParent(window), kCurveChanged, 0, 0);
}
void PaintCanvas(HWND window, HDC dc, const Canvas& canvas) {
    RECT client{};
    GetClientRect(window, &client);
    FillRect(dc, &client, static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    const RECT r = PlotRect(window);
    if (r.right <= r.left || r.bottom <= r.top) return;
    std::array<POINT, Curve::kLast + 3> points{};
    points[0] = {r.left, r.top};
    for (int i = 0; i <= Curve::kLast; ++i) {
        points[static_cast<std::size_t>(i) + 1] = {
            r.left + static_cast<LONG>(GraphXToFraction(canvas.curve.x[static_cast<std::size_t>(i)])
                * (r.right - r.left) + 0.5),
            r.top + MulDiv(i, r.bottom - r.top, Curve::kLast)};
    }
    points.back() = {r.left, r.bottom};
    const auto oldBrush = SelectObject(dc, GetStockObject(DC_BRUSH));
    const COLORREF oldColor = SetDCBrushColor(dc, RGB(255, 0, 0));
    const auto oldPen = SelectObject(dc, GetStockObject(NULL_PEN));
    Polygon(dc, points.data(), static_cast<int>(points.size()));
    SelectObject(dc, GetStockObject(BLACK_PEN));
    Polyline(dc, points.data() + 1, Curve::kLast + 1);
    MoveToEx(dc, r.left, r.top, nullptr);
    LineTo(dc, r.left, r.bottom);
    LineTo(dc, r.right, r.bottom);
    const auto oldFont = SelectObject(dc, GetStockObject(DEFAULT_GUI_FONT));
    const int oldMode = SetBkMode(dc, TRANSPARENT);
    wchar_t label[64]{};
    auto tick = [&](double value, int left, int top) {
        std::swprintf(label, 64, L"%.3g", value);
        TextOutW(dc, left, top, label, static_cast<int>(std::wcslen(label)));
    };
    tick(kGraphYMax, r.left - 26, r.top);
    tick(kGraphYMin, r.left - 26, r.bottom - 14);
    tick(kGraphXMin, r.left - 4, r.bottom + 4);
    tick(kGraphXMax, r.right - 18, r.bottom + 4);
    TextOutW(dc, r.right + 5, r.bottom - 14, L"x", 1);
    TextOutW(dc, r.left - 12, r.top - 18, L"y", 1);
    SetBkMode(dc, oldMode);
    SelectObject(dc, oldFont);
    SelectObject(dc, oldPen);
    SetDCBrushColor(dc, oldColor);
    SelectObject(dc, oldBrush);
}
LRESULT CALLBACK CanvasProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* canvas = reinterpret_cast<Canvas*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        canvas = static_cast<Canvas*>(reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(canvas));
    }
    switch (message) {
    case WM_LBUTTONDOWN: {
        const POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        const RECT plot = PlotRect(window);
        if (canvas && PtInRect(&plot, point)) {
            // Leaving an edit for the canvas must also run its focus-loss validation.
            SetFocus(window);
            // A validation dialog may have consumed the button release.
            if ((GetKeyState(VK_LBUTTON) & 0x8000) == 0) return 0;
            SetCapture(window);
            canvas->drawing = GetCapture() == window;
            if (canvas->drawing) DrawAt(window, *canvas, lParam, true);
        }
        return 0;
    }
    case WM_MOUSEMOVE:
        if (canvas && canvas->drawing) {
            if ((wParam & MK_LBUTTON) && GetCapture() == window)
                DrawAt(window, *canvas, lParam, false);
            else StopDrawing(window, *canvas);
        }
        return 0;
    case WM_LBUTTONUP:
        if (canvas && canvas->drawing) {
            DrawAt(window, *canvas, lParam, false);
            StopDrawing(window, *canvas);
        }
        return 0;
    case WM_CAPTURECHANGED:
        if (canvas) canvas->drawing = false;
        return 0;
    case WM_CANCELMODE:
    case WM_SIZE:
        if (canvas) StopDrawing(window, *canvas);
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_ERASEBKGND:
        return 1; // WM_PAINT paints the entire child in one buffered operation.
    case WM_PAINT: {
        PAINTSTRUCT paint{};
        HDC dc = BeginPaint(window, &paint);
        RECT client{};
        GetClientRect(window, &client);
        HDC memory = CreateCompatibleDC(dc);
        HBITMAP bitmap = CreateCompatibleBitmap(dc, std::max(1L, client.right),
            std::max(1L, client.bottom));
        if (canvas && memory && bitmap) {
            const auto previous = SelectObject(memory, bitmap);
            PaintCanvas(window, memory, *canvas);
            BitBlt(dc, 0, 0, client.right, client.bottom, memory, 0, 0, SRCCOPY);
            SelectObject(memory, previous);
        } else if (canvas) PaintCanvas(window, dc, *canvas);
        if (bitmap) DeleteObject(bitmap);
        if (memory) DeleteDC(memory);
        EndPaint(window, &paint);
        return 0;
    }
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
bool RegisterCanvas(HINSTANCE instance) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = CanvasProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_CROSS);
    wc.lpszClassName = kCanvasClass;
    return RegisterClassW(&wc) != 0;
}
}
