#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    if (msg == WM_KEYDOWN && wp == VK_ESCAPE) { PostQuitMessage(0); return 0; }
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT r;
        GetClientRect(hWnd, &r);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0,255,0));
        DrawTextA(hdc, "INVISIBLE WINDOW POC - PRESS ESC TO QUIT", -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        EndPaint(hWnd, &ps);
        return 0;
    }
    return DefWindowProcA(hWnd, msg, wp, lp);
}

int main() {
    HINSTANCE hInst = GetModuleHandle(NULL);

    WNDCLASSEXA wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(26,26,38));
    wc.lpszClassName = "TestInvisibleWindow";

    if (!RegisterClassExA(&wc)) {
        printf("RegisterClassEx failed: %lu\n", GetLastError());
        return 1;
    }
    printf("RegisterClassEx OK\n");

    HWND hWnd = CreateWindowExA(
        WS_EX_TOPMOST,
        "TestInvisibleWindow",
        "Invisible Window Test",
        WS_OVERLAPPEDWINDOW,
        100, 100, 640, 480,
        NULL, NULL, hInst, NULL);

    if (!hWnd) {
        printf("CreateWindowEx failed: %lu\n", GetLastError());
        return 1;
    }
    printf("CreateWindowEx OK, HWND=%p\n", (void*)hWnd);

    /* THE KEY API CALL */
    typedef BOOL (WINAPI *PFN)(HWND, DWORD);
    PFN pSet = (PFN)GetProcAddress(GetModuleHandleA("user32.dll"), "SetWindowDisplayAffinity");
    if (pSet) {
        BOOL ok = pSet(hWnd, 0x00000011); /* WDA_EXCLUDEFROMCAPTURE */
        printf("SetWindowDisplayAffinity(WDA_EXCLUDEFROMCAPTURE) = %d (err=%lu)\n", ok, GetLastError());
    } else {
        printf("SetWindowDisplayAffinity NOT FOUND\n");
    }

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);
    printf("Window shown. Press ESC in the window to quit.\n");
    printf("Try Win+Shift+S to screenshot - this window should be MISSING.\n");

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
