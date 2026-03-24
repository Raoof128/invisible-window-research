/*
 * invisible_window.c
 * The Invisible Window — Windows Proof of Concept
 * Author: Mohammad Raouf Abedini <mohammadraouf.abedini@students.mq.edu.au>
 *
 * Demonstrates that SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE) creates
 * a window visible on the physical display but invisible to ALL screen capture
 * APIs, including WebRTC getDisplayMedia().
 *
 * Compile: cl invisible_window.c /link user32.lib gdi32.lib
 *     or:  gcc invisible_window.c -o invisible_window.exe -lgdi32 -luser32 -mwindows
 *
 * This is a research tool for authorized security testing only.
 */

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>

/* WDA_EXCLUDEFROMCAPTURE — introduced in Windows 10 Version 2004 */
#ifndef WDA_EXCLUDEFROMCAPTURE
#define WDA_EXCLUDEFROMCAPTURE 0x00000011
#endif

/* Configuration */
#define WINDOW_WIDTH   640
#define WINDOW_HEIGHT  480
#define WINDOW_TITLE   L"Invisible Window (WDA_EXCLUDEFROMCAPTURE)"
#define CLASS_NAME     L"InvisibleWindowClass"

/* Colors */
#define BG_COLOR       RGB(26, 26, 38)      /* Dark blue-gray background */
#define TEXT_COLOR     RGB(0, 255, 102)      /* Green terminal text */
#define BORDER_COLOR   RGB(255, 51, 51)      /* Red border */
#define STATUS_BG      RGB(13, 13, 20)       /* Darker status bar */
#define STATUS_COLOR   RGB(255, 102, 102)    /* Red status text */

/* Global state */
static BOOL g_captureExcluded = TRUE;
static HFONT g_hFont = NULL;
static HFONT g_hSmallFont = NULL;

/* Content to display in the invisible window */
static const wchar_t* CONTENT_LINES[] = {
    L"╔══════════════════════════════════════════════════════╗",
    L"║           THE INVISIBLE WINDOW — PoC                ║",
    L"║                                                     ║",
    L"║  This window is VISIBLE on your physical display    ║",
    L"║  but INVISIBLE to screen capture APIs.              ║",
    L"║                                                     ║",
    L"║  If you take a screenshot or share your screen,     ║",
    L"║  this window will NOT appear in the output.         ║",
    L"║                                                     ║",
    L"║  Proof-of-Concept for:                              ║",
    L"║  \"The Invisible Window: Exploiting OS-Level         ║",
    L"║   Display Affinity to Bypass WebRTC Proctoring\"     ║",
    L"║                                                     ║",
    L"║  Author: Mohammad Raouf Abedini                     ║",
    L"║  Macquarie University, Sydney                       ║",
    L"║                                                     ║",
    L"║  WDA_EXCLUDEFROMCAPTURE applied ✓                   ║",
    L"║                                                     ║",
    L"║  Press T to toggle capture visibility               ║",
    L"║  Press S to take a verification screenshot          ║",
    L"║  Press Q or ESC to quit                             ║",
    L"╚══════════════════════════════════════════════════════╝",
};
static const int CONTENT_LINE_COUNT = sizeof(CONTENT_LINES) / sizeof(CONTENT_LINES[0]);

/* Forward declarations */
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void PaintWindow(HWND hWnd, HDC hdc);
void ToggleCaptureExclusion(HWND hWnd);
void TakeVerificationScreenshot(void);
void PrintStatus(HWND hWnd);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow)
{
    /* Allocate console for status output */
    AllocConsole();
    FILE* fout;
    freopen_s(&fout, "CONOUT$", "w", stdout);

    /* Register window class */
    WNDCLASSEX wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = hInstance;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = CreateSolidBrush(BG_COLOR);
    wc.lpszClassName  = CLASS_NAME;
    wc.hIcon          = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"Window class registration failed.", L"Error", MB_ICONERROR);
        return 1;
    }

    /* Calculate centered position */
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - WINDOW_WIDTH) / 2;
    int posY = (screenH - WINDOW_HEIGHT) / 2;

    /* Create the window — WS_EX_TOPMOST keeps it above other windows */
    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST,                          /* Always on top */
        CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        posX, posY,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        MessageBox(NULL, L"Window creation failed.", L"Error", MB_ICONERROR);
        return 1;
    }

    /* ═══════════════════════════════════════════════════════════
     * THE KEY LINE: Exclude this window from ALL screen capture
     * ═══════════════════════════════════════════════════════════ */
    BOOL result = SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);

    if (!result) {
        DWORD err = GetLastError();
        wprintf(L"[WARN] SetWindowDisplayAffinity failed (error %lu).\n", err);
        wprintf(L"[WARN] This may mean Windows version < 2004. Trying WDA_MONITOR...\n");

        /* Fallback to WDA_MONITOR for older Windows 10 */
        result = SetWindowDisplayAffinity(hWnd, 0x00000001); /* WDA_MONITOR */
        if (result) {
            wprintf(L"[INFO] WDA_MONITOR applied (window shows as black in capture).\n");
            g_captureExcluded = TRUE;
        } else {
            wprintf(L"[ERROR] Both WDA_EXCLUDEFROMCAPTURE and WDA_MONITOR failed.\n");
            g_captureExcluded = FALSE;
        }
    } else {
        wprintf(L"[OK] WDA_EXCLUDEFROMCAPTURE applied successfully.\n");
        g_captureExcluded = TRUE;
    }

    /* Create fonts */
    g_hFont = CreateFont(
        16, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas"
    );
    g_hSmallFont = CreateFont(
        13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas"
    );

    /* Show window */
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    PrintStatus(hWnd);

    /* Message loop */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /* Cleanup */
    if (g_hFont) DeleteObject(g_hFont);
    if (g_hSmallFont) DeleteObject(g_hSmallFont);
    FreeConsole();

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        PaintWindow(hWnd, hdc);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_KEYDOWN:
        switch (wParam) {
        case 'T':
        case 't':
            ToggleCaptureExclusion(hWnd);
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        case 'S':
        case 's':
            TakeVerificationScreenshot();
            break;
        case 'Q':
        case 'q':
        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

void PaintWindow(HWND hWnd, HDC hdc)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);

    /* Fill background */
    HBRUSH bgBrush = CreateSolidBrush(BG_COLOR);
    FillRect(hdc, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    /* Draw red border */
    HPEN borderPen = CreatePen(PS_SOLID, 3, BORDER_COLOR);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 2, 2, clientRect.right - 2, clientRect.bottom - 2);
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(borderPen);

    /* Draw content text */
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, TEXT_COLOR);
    HFONT oldFont = (HFONT)SelectObject(hdc, g_hFont);

    int y = 20;
    for (int i = 0; i < CONTENT_LINE_COUNT; i++) {
        TextOut(hdc, 20, y, CONTENT_LINES[i], (int)wcslen(CONTENT_LINES[i]));
        y += 18;
    }

    /* Draw status bar */
    RECT statusRect = {3, clientRect.bottom - 28, clientRect.right - 3, clientRect.bottom - 3};
    HBRUSH statusBrush = CreateSolidBrush(STATUS_BG);
    FillRect(hdc, &statusRect, statusBrush);
    DeleteObject(statusBrush);

    SelectObject(hdc, g_hSmallFont);
    SetTextColor(hdc, STATUS_COLOR);

    const wchar_t* statusText = g_captureExcluded
        ? L"● CAPTURE-INVISIBLE  |  WDA_EXCLUDEFROMCAPTURE  |  T=toggle  S=screenshot  Q=quit"
        : L"○ CAPTURE-VISIBLE    |  WDA_NONE                |  T=toggle  S=screenshot  Q=quit";

    TextOut(hdc, 10, clientRect.bottom - 24, statusText, (int)wcslen(statusText));

    SelectObject(hdc, oldFont);
}

void ToggleCaptureExclusion(HWND hWnd)
{
    if (g_captureExcluded) {
        SetWindowDisplayAffinity(hWnd, 0x00000000); /* WDA_NONE */
        g_captureExcluded = FALSE;
        wprintf(L"[TOGGLE] WDA_NONE — window NOW VISIBLE to capture\n");
    } else {
        BOOL ok = SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);
        if (ok) {
            g_captureExcluded = TRUE;
            wprintf(L"[TOGGLE] WDA_EXCLUDEFROMCAPTURE — window INVISIBLE to capture\n");
        } else {
            wprintf(L"[ERROR] Failed to re-apply WDA_EXCLUDEFROMCAPTURE\n");
        }
    }
}

void TakeVerificationScreenshot(void)
{
    /* Use BitBlt to capture the screen — this respects display affinity */
    HDC hdcScreen = GetDC(NULL);
    int w = GetSystemMetrics(SM_CXSCREEN);
    int h = GetSystemMetrics(SM_CYSCREEN);

    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, w, h);
    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, w, h, hdcScreen, 0, 0, SRCCOPY);

    /* Save as BMP */
    wchar_t path[MAX_PATH];
    SYSTEMTIME st;
    GetLocalTime(&st);
    swprintf_s(path, MAX_PATH,
        L"invisible_window_verify_%04d%02d%02d_%02d%02d%02d.bmp",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    BITMAPINFOHEADER bi = {0};
    bi.biSize        = sizeof(BITMAPINFOHEADER);
    bi.biWidth       = w;
    bi.biHeight      = -h; /* top-down */
    bi.biPlanes      = 1;
    bi.biBitCount    = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage   = ((w * 3 + 3) & ~3) * h;

    BYTE* pixels = (BYTE*)malloc(bi.biSizeImage);
    GetDIBits(hdcMem, hBitmap, 0, h, pixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    BITMAPFILEHEADER bf = {0};
    bf.bfType    = 0x4D42; /* "BM" */
    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bf.bfSize    = bf.bfOffBits + bi.biSizeImage;

    HANDLE hFile = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD written;
        WriteFile(hFile, &bf, sizeof(bf), &written, NULL);
        WriteFile(hFile, &bi, sizeof(bi), &written, NULL);
        WriteFile(hFile, pixels, bi.biSizeImage, &written, NULL);
        CloseHandle(hFile);
        wprintf(L"[VERIFY] Screenshot saved: %s\n", path);
        wprintf(L"[VERIFY] If the invisible window is NOT in the screenshot,\n");
        wprintf(L"[VERIFY] WDA_EXCLUDEFROMCAPTURE is working.\n");
    } else {
        wprintf(L"[ERROR] Failed to save screenshot\n");
    }

    free(pixels);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

void PrintStatus(HWND hWnd)
{
    DWORD affinity = 0;
    GetWindowDisplayAffinity(hWnd, &affinity);

    OSVERSIONINFOEX osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(osvi);

    wprintf(L"\n");
    wprintf(L"╔══════════════════════════════════════════════════════════╗\n");
    wprintf(L"║         THE INVISIBLE WINDOW — Windows PoC Running     ║\n");
    wprintf(L"╠══════════════════════════════════════════════════════════╣\n");
    wprintf(L"║                                                        ║\n");
    wprintf(L"║  Display Affinity: 0x%08X                           ║\n", affinity);
    wprintf(L"║  Status: %s                    ║\n",
        g_captureExcluded ? L"CAPTURE-INVISIBLE" : L"CAPTURE-VISIBLE  ");
    wprintf(L"║                                                        ║\n");
    wprintf(L"║  Controls:                                             ║\n");
    wprintf(L"║    T — Toggle capture visibility                       ║\n");
    wprintf(L"║    S — Take verification screenshot                    ║\n");
    wprintf(L"║    Q / ESC — Quit                                      ║\n");
    wprintf(L"║                                                        ║\n");
    wprintf(L"║  TEST: Press Win+Shift+S (Snipping Tool) or           ║\n");
    wprintf(L"║  PrintScreen and check if this window appears.         ║\n");
    wprintf(L"║  If WDA_EXCLUDEFROMCAPTURE works, the window           ║\n");
    wprintf(L"║  will be COMPLETELY MISSING from the capture.          ║\n");
    wprintf(L"║                                                        ║\n");
    wprintf(L"╚══════════════════════════════════════════════════════════╝\n");
    wprintf(L"\n");
}
