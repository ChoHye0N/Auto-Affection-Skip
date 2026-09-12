#include "pch.h"
#include "imageProcessing.h"

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

using namespace cv;

// 캡처마다 메모리 DC/비트맵을 새로 만들지 않도록 창 크기/옵션이 같으면 재사용
struct CaptureResources {
    HWND hwnd = nullptr;
    int captureOption = -1;
    int width = 0;
    int height = 0;
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
        *this = CaptureResources{};
    }

    // hScreenDC는 호환 DC/비트맵 생성에만 쓰고 보관하지 않음
    bool Create(HDC hScreenDC, HWND targetWnd, int option, int w, int h) {
        Release();

        hMemoryDC = CreateCompatibleDC(hScreenDC);
        hBitmap = CreateCompatibleBitmap(hScreenDC, w, h);
        if (!hMemoryDC || !hBitmap) {
            Release();
            return false;
        }
        hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

        hwnd = targetWnd;
        captureOption = option;
        width = w;
        height = h;
        return true;
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

    bool usePrintWindow = captureOption == static_cast<int>(CaptureOption::PrintWindow);

    // 화면/창 DC는 매 캡처마다 얻고 바로 돌려줌
    HWND dcOwner = usePrintWindow ? hwnd : NULL;
    HDC hScreenDC = GetDC(dcOwner);
    if (!hScreenDC) return Mat();

    // 창 크기/캡처 옵션이 바뀌지 않았으면 기존 메모리 DC/비트맵 재사용
    if (!g_captureRes.Matches(hwnd, captureOption, width, height) &&
        !g_captureRes.Create(hScreenDC, hwnd, captureOption, width, height)) {
        ReleaseDC(dcOwner, hScreenDC);
        return Mat();
    }

    BOOL captured;
    if (usePrintWindow) {
        // 마우스 커서를 제외하고 창의 내용물을 렌더링해서 복사함
        captured = PrintWindow(hwnd, g_captureRes.hMemoryDC, 2);
    }
    else {
        // 창의 내용물을 렌더링해서 복사함
        captured = BitBlt(g_captureRes.hMemoryDC, 0, 0, width, height, hScreenDC, windowRect.left, windowRect.top, SRCCOPY);
    }

    ReleaseDC(dcOwner, hScreenDC);

    // 복사에 실패하면 비트맵에는 이전 프레임이 남아 있으므로 그걸 돌려주지 말고 실패 처리
    if (!captured) {
        g_captureRes.Release();
        return Mat();
    }

    // 전체 창을 Mat으로 가져오기 (4채널 BGRA 방식)
    Mat src(height, width, CV_8UC4);
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), width, -height, 1, 32, BI_RGB };
    SelectObject(g_captureRes.hMemoryDC, g_captureRes.hOldBitmap);
    int lines = GetDIBits(g_captureRes.hMemoryDC, g_captureRes.hBitmap, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    SelectObject(g_captureRes.hMemoryDC, g_captureRes.hBitmap);
    if (lines == 0) {
        g_captureRes.Release();
        return Mat();
    }

    Mat bgr;
    cvtColor(src, bgr, COLOR_BGRA2BGR);

    // 게임 화면만 잘라냄
    int borderWidth = (width - clientRect.right) / 2;
    int titleBarHeight = (height - clientRect.bottom) - borderWidth;

    Rect roi(borderWidth, titleBarHeight, clientRect.right, clientRect.bottom);

    // 잘라낸 이미지 반환
    if (roi.x + roi.width <= bgr.cols && roi.y + roi.height <= bgr.rows && roi.x >= 0 && roi.y >= 0) {
        return bgr(roi).clone();
    }

    return bgr;
}
