#include "pch.h"
#include "imageProcessing.h"

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

using namespace cv;

// 캡처마다 DC/비트맵을 새로 만들지 않도록 창 크기/옵션이 같으면 재사용
struct CaptureResources {
    HWND hwnd = nullptr;
    HWND targetHwnd = nullptr; // ReleaseDC 대상 (PrintWindow 방식일 때만 hwnd, 아니면 NULL 화면 DC)
    int captureOption = -1;
    int width = 0;
    int height = 0;
    HDC hScreenDC = nullptr;
    HDC hMemoryDC = nullptr;
    HBITMAP hBitmap = nullptr;
    HBITMAP hOldBitmap = nullptr;

    bool Matches(HWND targetWnd, int option, int w, int h) const {
        return hMemoryDC != nullptr && hwnd == targetWnd && captureOption == option && width == w && height == h;
    }

    void Release() {
        if (hMemoryDC && hOldBitmap) SelectObject(hMemoryDC, hOldBitmap);
        if (hBitmap) DeleteObject(hBitmap);
        if (hMemoryDC) DeleteDC(hMemoryDC);
        if (hScreenDC) {
            if (targetHwnd == nullptr) ReleaseDC(NULL, hScreenDC);
            else ReleaseDC(targetHwnd, hScreenDC);
        }
        *this = CaptureResources{};
    }

    void Create(HWND targetWnd, int option, int w, int h) {
        Release();

        hwnd = targetWnd;
        captureOption = option;
        width = w;
        height = h;

        if (option == static_cast<int>(CaptureOption::PrintWindow)) {
            targetHwnd = targetWnd;
            hScreenDC = GetDC(targetWnd);
        }
        else {
            hScreenDC = GetDC(NULL);
        }

        hMemoryDC = CreateCompatibleDC(hScreenDC);
        hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
        hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
    }
};

static CaptureResources g_captureRes;

void ReleaseCaptureResources() {
    g_captureRes.Release();
}

Mat CaptureGameWindow(HWND hwnd, int captureOption) {
    RECT windowRect, clientRect;
    GetWindowRect(hwnd, &windowRect);
    GetClientRect(hwnd, &clientRect);

    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;
    if (width <= 0 || height <= 0) return Mat();

    // 창 크기/캡처 옵션이 바뀌지 않았으면 기존 DC/비트맵 재사용 (매 프레임 GDI 자원 생성/해제 방지)
    if (!g_captureRes.Matches(hwnd, captureOption, width, height)) {
        g_captureRes.Create(hwnd, captureOption, width, height);
    }

    if (captureOption == static_cast<int>(CaptureOption::PrintWindow)) {
        // 마우스 커서를 제외하고 창의 내용물을 렌더링해서 복사함
        PrintWindow(hwnd, g_captureRes.hMemoryDC, 2);
    }
    else {
        // 창의 내용물을 렌더링해서 복사함
        BitBlt(g_captureRes.hMemoryDC, 0, 0, width, height, g_captureRes.hScreenDC, windowRect.left, windowRect.top, SRCCOPY);
    }

    // 전체 창을 Mat으로 가져오기 (4채널 BGRA 방식)
    Mat src(height, width, CV_8UC4);
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB };
    GetDIBits(g_captureRes.hMemoryDC, g_captureRes.hBitmap, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    Mat bgr;
    cvtColor(src, bgr, COLOR_BGRA2BGR);

    // 게임 화면만 잘라내기
    int borderWidth = (width - clientRect.right) / 2;
    int titleBarHeight = (height - clientRect.bottom) - borderWidth;

    Rect roi(borderWidth, titleBarHeight, clientRect.right, clientRect.bottom);

    // 잘라낸 이미지 반환
    if (roi.x + roi.width <= bgr.cols && roi.y + roi.height <= bgr.rows && roi.x >= 0 && roi.y >= 0) {
        return bgr(roi).clone();
    }

    return bgr;
}
