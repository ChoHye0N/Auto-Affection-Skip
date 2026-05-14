#include "pch.h"
#include "imageProcessing.h"

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

using namespace cv;

WindowCapture::WindowCapture(HWND hwnd, int captureOption) {
    GetWindowRect(hwnd, &windowRect);
    GetClientRect(hwnd, &clientRect);

    // 창 전체 크기와 실제 게임 화면 크기를 모두 구함
    width = windowRect.right - windowRect.left;
    height = windowRect.bottom - windowRect.top;

    // GDI 자원 생성 (창 전체 크기 기준)
    if (captureOption == static_cast<int>(CaptureOption::PrintWindow)) {
        this->targetHwnd = hwnd;
        hScreenDC = GetDC(hwnd);
    }
    else {
        hScreenDC = GetDC(NULL);
    }
    
    hMemoryDC = CreateCompatibleDC(hScreenDC);
    hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    if (captureOption == static_cast<int>(CaptureOption::PrintWindow)) {
        // 마우스 커서를 제외하고 창의 내용물을 렌더링해서 복사함
        PrintWindow(hwnd, hMemoryDC, 2);
    }
    else {
        // 창의 내용물을 렌더링해서 복사함
        BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, windowRect.left, windowRect.top, SRCCOPY);
    }
}

WindowCapture::~WindowCapture() {
    if (hOldBitmap) SelectObject(hMemoryDC, hOldBitmap);
    if (hBitmap) DeleteObject(hBitmap);
    if (hMemoryDC) DeleteDC(hMemoryDC);
    if (hScreenDC) {
        if (this->targetHwnd == nullptr) ReleaseDC(NULL, hScreenDC);
        else ReleaseDC(this->targetHwnd, hScreenDC);
    }
}

Mat CaptureGameWindow(HWND hwnd, int captureOption) {
    WindowCapture cap(hwnd, captureOption);

    // 전체 창을 Mat으로 가져오기 (4채널 BGRA 방식)
    Mat src(cap.height, cap.width, CV_8UC4);
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), cap.width, -cap.height, 1, 32, BI_RGB };
    GetDIBits(cap.hMemoryDC, cap.hBitmap, 0, cap.height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    Mat bgr;
    cvtColor(src, bgr, COLOR_BGRA2BGR);

    // 게임 화면만 잘라내기
    int borderWidth = (cap.width - cap.clientRect.right) / 2;
    int titleBarHeight = (cap.height - cap.clientRect.bottom) - borderWidth;

    Rect roi(borderWidth, titleBarHeight, cap.clientRect.right, cap.clientRect.bottom);

    // 잘라낸 이미지 반환
    if (roi.x + roi.width <= bgr.cols && roi.y + roi.height <= bgr.rows && roi.x >= 0 && roi.y >= 0) {
        return bgr(roi).clone();
    }

    return bgr;
}