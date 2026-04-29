#include "pch.h"
#include "imageProcessing.h"

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

using namespace cv;

WindowCapture::WindowCapture(HWND hwnd) {
    // 창의 전체 영역(테두리 포함) 구함
    GetWindowRect(hwnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    // GDI 자원 생성 (창 전체 크기 기준)
    hScreenDC = GetDC(NULL);
    hMemoryDC = CreateCompatibleDC(hScreenDC);
    hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // 창의 내용물을 렌더링해서 복사함
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, rect.left, rect.top, SRCCOPY);
}

WindowCapture::~WindowCapture() {
    if (hOldBitmap) SelectObject(hMemoryDC, hOldBitmap);
    if (hBitmap) DeleteObject(hBitmap);
    if (hMemoryDC) DeleteDC(hMemoryDC);
    if (hScreenDC) ReleaseDC(NULL, hScreenDC);
}

Mat CaptureGameWindow(HWND hwnd) {
    WindowCapture cap(hwnd);

    // 전체 창을 Mat으로 가져오기 (4채널 BGRA 방식)
    Mat src(cap.height, cap.width, CV_8UC4);
    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), cap.width, -cap.height, 1, 32, BI_RGB };
    GetDIBits(cap.hMemoryDC, cap.hBitmap, 0, cap.height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    Mat bgr;
    cvtColor(src, bgr, COLOR_BGRA2BGR);

    // 게임 화면만 잘라내기
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int border_width = (cap.width - clientRect.right) / 2;
    int title_bar_height = (cap.height - clientRect.bottom) - border_width;

    Rect roi(border_width, title_bar_height, clientRect.right, clientRect.bottom);

    // 잘라낸 이미지 반환
    if (roi.x + roi.width <= bgr.cols && roi.y + roi.height <= bgr.rows && roi.x >= 0 && roi.y >= 0) {
        return bgr(roi).clone();
    }

    return bgr;
}