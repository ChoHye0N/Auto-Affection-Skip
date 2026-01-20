#include "pch.h"
#include "imageProcessing.h"

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

using namespace cv;

// 창 제목을 인자로 받아 해당 영역만 캡처
Mat CaptureGameWindow(const char* windowName) {
    HWND hwnd = FindWindowA("UnityWndClass", windowName);
    if (!hwnd) return Mat();

    RECT rect;
    // 모니터 기준 창 좌표
    GetWindowRect(hwnd, &rect); 

    int width = rect.right - rect.left, height = rect.bottom - rect.top;

    HDC hScreenDC = GetDC(NULL), hMemoryDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    SelectObject(hMemoryDC, hBitmap);

    // 창의 시작점부터 캡처
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, rect.left, rect.top, SRCCOPY);

    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), width, -height, 1, 24, BI_RGB };
    Mat mat(height, width, CV_8UC3);
    GetDIBits(hMemoryDC, hBitmap, 0, height, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return mat;
}