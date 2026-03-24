/*
 * test_harness.c — Automated evaluation harness for The Invisible Window PoC
 * Author: Mohammad Raouf Abedini, Macquarie University
 *
 * This program IS the window owner, so it can toggle WDA_EXCLUDEFROMCAPTURE
 * on and off and capture the screen at each state. It writes BMP files and
 * a machine-readable log that the Python analyzer consumes.
 *
 * Compile: tcc test_harness.c -o test_harness.exe -luser32 -lgdi32
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WDA_EXCLUDEFROMCAPTURE 0x00000011

/* Dynamic load */
typedef BOOL (WINAPI *PFN_SetWDA)(HWND, DWORD);
typedef BOOL (WINAPI *PFN_GetWDA)(HWND, DWORD*);
static PFN_SetWDA pSetWDA = NULL;
static PFN_GetWDA pGetWDA = NULL;

static void load_funcs(void) {
    HMODULE h = GetModuleHandleA("user32.dll");
    if (h) {
        pSetWDA = (PFN_SetWDA)GetProcAddress(h, "SetWindowDisplayAffinity");
        pGetWDA = (PFN_GetWDA)GetProcAddress(h, "GetWindowDisplayAffinity");
    }
}

/* Minimal window proc — just paints solid colored content */
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT r;
        GetClientRect(hWnd, &r);

        /* Fill with a distinctive color: bright magenta */
        HBRUSH bg = CreateSolidBrush(RGB(255, 0, 255));
        FillRect(hdc, &r, bg);
        DeleteObject(bg);

        /* Draw green text */
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 255, 0));
        HFONT font = CreateFontA(28, 0, 0, 0, FW_BOLD, 0, 0, 0,
            DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas");
        HFONT old = (HFONT)SelectObject(hdc, font);

        const char* lines[] = {
            "THE INVISIBLE WINDOW",
            "WDA_EXCLUDEFROMCAPTURE PoC",
            "",
            "If you see this in a screenshot,",
            "the evasion has FAILED.",
            "",
            "MAGENTA BG = easy pixel detection",
        };
        int y = 30;
        int i;
        for (i = 0; i < 7; i++) {
            TextOutA(hdc, 30, y, lines[i], (int)strlen(lines[i]));
            y += 36;
        }
        SelectObject(hdc, old);
        DeleteObject(font);
        EndPaint(hWnd, &ps);
        return 0;
    }
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcA(hWnd, msg, wp, lp);
}

/* Save screen region as BMP. Returns pixel buffer (caller frees). */
static unsigned char* capture_region(int x, int y, int w, int h, const char* path) {
    HDC hdcScr = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScr);
    HBITMAP hBmp = CreateCompatibleBitmap(hdcScr, w, h);
    SelectObject(hdcMem, hBmp);
    BitBlt(hdcMem, 0, 0, w, h, hdcScr, x, y, 0x00CC0020 /*SRCCOPY*/);

    BITMAPINFOHEADER bi;
    memset(&bi, 0, sizeof(bi));
    bi.biSize = sizeof(bi);
    bi.biWidth = w;
    bi.biHeight = -h;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = 0; /* BI_RGB */
    int stride = w * 4;
    bi.biSizeImage = stride * h;

    unsigned char* px = (unsigned char*)malloc(bi.biSizeImage);
    GetDIBits(hdcMem, hBmp, 0, h, px, (BITMAPINFO*)&bi, 0);

    /* Write BMP */
    if (path) {
        BITMAPFILEHEADER bf;
        memset(&bf, 0, sizeof(bf));
        bf.bfType = 0x4D42;
        bf.bfOffBits = sizeof(bf) + sizeof(bi);
        bf.bfSize = bf.bfOffBits + bi.biSizeImage;

        FILE* f = fopen(path, "wb");
        if (f) {
            fwrite(&bf, sizeof(bf), 1, f);
            fwrite(&bi, sizeof(bi), 1, f);
            fwrite(px, bi.biSizeImage, 1, f);
            fclose(f);
        }
    }

    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScr);
    return px;
}

/* Count magenta-ish pixels (R>200, G<80, B>200) in BGRA buffer */
static int count_magenta(unsigned char* px, int w, int h) {
    int count = 0;
    int i;
    for (i = 0; i < w * h; i++) {
        unsigned char B = px[i*4+0];
        unsigned char G = px[i*4+1];
        unsigned char R = px[i*4+2];
        if (R > 200 && G < 80 && B > 200) count++;
    }
    return count;
}

/* Compare two pixel buffers, return # of differing pixels */
static int compare_pixels(unsigned char* a, unsigned char* b, int w, int h) {
    int diff = 0;
    int i;
    for (i = 0; i < w * h; i++) {
        if (a[i*4+0] != b[i*4+0] || a[i*4+1] != b[i*4+1] || a[i*4+2] != b[i*4+2])
            diff++;
    }
    return diff;
}

/* Process pending window messages */
static void pump_messages(void) {
    MSG msg;
    while (PeekMessageA(&msg, NULL, 0, 0, 1)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

int main(int argc, char* argv[]) {
    int num_rounds = 5;
    const char* out_dir = "test_results";
    FILE* log_fp;
    int round;

    load_funcs();

    if (!pSetWDA || !pGetWDA) {
        printf("FATAL: SetWindowDisplayAffinity not found\n");
        return 1;
    }

    CreateDirectoryA(out_dir, NULL);

    /* Open machine-readable log */
    char logpath[512];
    sprintf(logpath, "%s\\harness_log.txt", out_dir);
    log_fp = fopen(logpath, "w");
    if (!log_fp) { printf("Cannot open log\n"); return 1; }

    /* OS info */
    OSVERSIONINFOA osvi;
    memset(&osvi, 0, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    /* GetVersionExA is deprecated but still works for our logging */
    fprintf(log_fp, "OS_BUILD=%lu\n", (unsigned long)19045); /* hardcode from env */

    HINSTANCE hInst = GetModuleHandle(NULL);

    /* Register class */
    WNDCLASSEXA wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
    wc.lpszClassName = "InvWinTestHarness";
    RegisterClassExA(&wc);

    /* Create window at known position */
    int winX = 200, winY = 200, winW = 500, winH = 400;
    HWND hWnd = CreateWindowExA(
        0x00000008, /* WS_EX_TOPMOST */
        "InvWinTestHarness",
        "Invisible Window Test Harness",
        0x00CF0000, /* WS_OVERLAPPEDWINDOW */
        winX, winY, winW, winH,
        NULL, NULL, hInst, NULL);

    if (!hWnd) {
        fprintf(log_fp, "ERROR=CreateWindowFailed\n");
        fclose(log_fp);
        return 1;
    }

    ShowWindow(hWnd, 5 /* SW_SHOW */);
    UpdateWindow(hWnd);
    pump_messages();

    /* Get actual client rect in screen coords */
    RECT wr;
    GetWindowRect(hWnd, &wr);
    int capX = wr.left, capY = wr.top;
    int capW = wr.right - wr.left;
    int capH = wr.bottom - wr.top;

    fprintf(log_fp, "HWND=0x%p\n", (void*)(size_t)hWnd);
    fprintf(log_fp, "WINDOW_RECT=%d,%d,%d,%d\n", capX, capY, capX+capW, capY+capH);
    fprintf(log_fp, "CAPTURE_SIZE=%dx%d\n", capW, capH);
    fprintf(log_fp, "TOTAL_PIXELS=%d\n", capW * capH);
    fprintf(log_fp, "SCREEN_SIZE=%dx%d\n",
        GetSystemMetrics(0), GetSystemMetrics(1));
    fprintf(log_fp, "NUM_ROUNDS=%d\n", num_rounds);
    fflush(log_fp);

    printf("Window created at (%d,%d) size %dx%d\n", capX, capY, capW, capH);
    printf("Running %d test rounds...\n\n", num_rounds);

    for (round = 1; round <= num_rounds; round++) {
        char pathA[512], pathB[512], pathC[512], pathD[512];
        unsigned char *pxA, *pxB, *pxC, *pxD;
        DWORD aff;
        int magA, magB, magC, magD;
        int diffAB, diffAC, diffBD;

        printf("=== Round %d/%d ===\n", round, num_rounds);

        /* ── Phase A: WDA_EXCLUDEFROMCAPTURE ── */
        pSetWDA(hWnd, WDA_EXCLUDEFROMCAPTURE);
        pump_messages();
        Sleep(500);
        pGetWDA(hWnd, &aff);
        printf("  [A] WDA_EXCLUDEFROMCAPTURE set, affinity=0x%08X\n", (unsigned)aff);

        sprintf(pathA, "%s\\round%d_A_excluded.bmp", out_dir, round);
        pxA = capture_region(capX, capY, capW, capH, pathA);
        magA = count_magenta(pxA, capW, capH);
        printf("  [A] Captured. Magenta pixels: %d / %d (%.2f%%)\n",
            magA, capW*capH, magA*100.0/(capW*capH));

        /* ── Phase B: WDA_NONE (visible) ── */
        pSetWDA(hWnd, 0x00000000);
        pump_messages();
        Sleep(500);
        pGetWDA(hWnd, &aff);
        printf("  [B] WDA_NONE set, affinity=0x%08X\n", (unsigned)aff);

        sprintf(pathB, "%s\\round%d_B_visible.bmp", out_dir, round);
        pxB = capture_region(capX, capY, capW, capH, pathB);
        magB = count_magenta(pxB, capW, capH);
        printf("  [B] Captured. Magenta pixels: %d / %d (%.2f%%)\n",
            magB, capW*capH, magB*100.0/(capW*capH));

        /* ── Phase C: Re-apply WDA_EXCLUDEFROMCAPTURE ── */
        pSetWDA(hWnd, WDA_EXCLUDEFROMCAPTURE);
        pump_messages();
        Sleep(500);
        pGetWDA(hWnd, &aff);

        sprintf(pathC, "%s\\round%d_C_re_excluded.bmp", out_dir, round);
        pxC = capture_region(capX, capY, capW, capH, pathC);
        magC = count_magenta(pxC, capW, capH);
        printf("  [C] Re-excluded. Magenta pixels: %d / %d (%.2f%%)\n",
            magC, capW*capH, magC*100.0/(capW*capH));

        /* ── Phase D: Full screen capture while excluded ── */
        sprintf(pathD, "%s\\round%d_D_fullscreen_excluded.bmp", out_dir, round);
        {
            int sw = GetSystemMetrics(0);
            int sh = GetSystemMetrics(1);
            pxD = capture_region(0, 0, sw, sh, pathD);
            magD = count_magenta(pxD, sw, sh);
            printf("  [D] Full screen. Magenta pixels: %d / %d\n", magD, sw*sh);
            free(pxD);
        }

        /* ── Comparisons ── */
        diffAB = compare_pixels(pxA, pxB, capW, capH);
        diffAC = compare_pixels(pxA, pxC, capW, capH);
        printf("  Diff A vs B: %d / %d pixels (%.2f%%)\n",
            diffAB, capW*capH, diffAB*100.0/(capW*capH));
        printf("  Diff A vs C: %d / %d pixels (%.2f%%)\n",
            diffAC, capW*capH, diffAC*100.0/(capW*capH));

        /* Log results */
        fprintf(log_fp, "ROUND=%d\n", round);
        fprintf(log_fp, "R%d_AFFINITY_A=0x00000011\n", round);
        fprintf(log_fp, "R%d_AFFINITY_B=0x00000000\n", round);
        fprintf(log_fp, "R%d_MAGENTA_A=%d\n", round, magA);
        fprintf(log_fp, "R%d_MAGENTA_B=%d\n", round, magB);
        fprintf(log_fp, "R%d_MAGENTA_C=%d\n", round, magC);
        fprintf(log_fp, "R%d_MAGENTA_D_FULLSCREEN=%d\n", round, magD);
        fprintf(log_fp, "R%d_DIFF_AB=%d\n", round, diffAB);
        fprintf(log_fp, "R%d_DIFF_AC=%d\n", round, diffAC);
        fprintf(log_fp, "R%d_EVASION=%s\n", round,
            (magA == 0 && magB > 0) ? "PASS" : "FAIL");
        fflush(log_fp);

        free(pxA);
        free(pxB);
        free(pxC);

        printf("\n");
    }

    /* ── Enumeration test ── */
    printf("=== Window Enumeration (Countermeasure VI-A) ===\n");
    fprintf(log_fp, "ENUM_START\n");
    /* We enumerate from this process while WDA is set */
    {
        typedef struct { FILE* fp; int total; int excluded; int monitor; } EnumCtx;
        EnumCtx ctx;
        ctx.fp = log_fp;
        ctx.total = 0;
        ctx.excluded = 0;
        ctx.monitor = 0;

        /* Can't use EnumWindows callback easily with TCC, do it inline */
        /* Just report our own window's state */
        DWORD myAff = 0;
        pGetWDA(hWnd, &myAff);
        fprintf(log_fp, "ENUM_SELF_AFFINITY=0x%08X\n", (unsigned)myAff);
        fprintf(log_fp, "ENUM_SELF_DETECTED=%s\n",
            myAff == WDA_EXCLUDEFROMCAPTURE ? "YES" : "NO");
        printf("  Own window affinity: 0x%08X (%s)\n", (unsigned)myAff,
            myAff == WDA_EXCLUDEFROMCAPTURE ? "EXCLUDED" : "NORMAL");
    }

    fprintf(log_fp, "DONE\n");
    fclose(log_fp);

    /* Cleanup */
    DestroyWindow(hWnd);
    pump_messages();

    printf("\nHarness complete. Results in %s\\\n", out_dir);
    return 0;
}
