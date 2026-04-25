#include "pch.h"
#include "imageProcessing.h"

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

using namespace cv;

WindowCapture::WindowCapture(HWND hwnd) {
    this->targetHwnd = hwnd;

    // 창 전체 크기와 실제 게임 화면 크기를 모두 구함
    RECT windowRect, clientRect;
    GetWindowRect(hwnd, &windowRect);
    GetClientRect(hwnd, &clientRect);

    width = windowRect.right - windowRect.left;
    height = windowRect.bottom - windowRect.top;

    // 잘라낼 실제 게임 영역 크기 저장
    int c_width = clientRect.right;
    int c_height = clientRect.bottom;

    // GDI 자원 생성 (창 전체 크기 기준)
    hScreenDC = GetDC(hwnd);
    hMemoryDC = CreateCompatibleDC(hScreenDC);
    hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // 마우스 커서를 제외하고 창의 내용물을 렌더링해서 복사함
    PrintWindow(hwnd, hMemoryDC, 2);
}

WindowCapture::~WindowCapture() {
    if (hOldBitmap) SelectObject(hMemoryDC, hOldBitmap);
    if (hBitmap) DeleteObject(hBitmap);
    if (hMemoryDC) DeleteDC(hMemoryDC);
    if (hScreenDC) ReleaseDC(this->targetHwnd, hScreenDC);
}

Mat CaptureGameWindow(const char* windowName) {
    HWND hwnd = FindWindowA("UnityWndClass", windowName);
    if (!hwnd) return Mat();

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
    int cWidth = clientRect.right;
    int cHeight = clientRect.bottom;

    // 테두리와 타이틀바 두께 계산
    int border_width = (cap.width - cWidth) / 2;
    int title_bar_height = (cap.height - cHeight) - border_width;

    Rect roi(border_width, title_bar_height, cWidth, cHeight);

    // 잘라낸 이미지 반환
    if (roi.x + roi.width <= bgr.cols && roi.y + roi.height <= bgr.rows && roi.x >= 0 && roi.y >= 0) {
        return bgr(roi).clone();
    }

    return bgr;
}