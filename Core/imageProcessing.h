#pragma once

#include <opencv2/opencv.hpp>
#include <windows.h>

// GDI 자원 자동 관리를 위한 RAII 클래스
class WindowCapture {
public:
    HWND targetHwnd = nullptr;
    HDC hScreenDC = nullptr;
    HDC hMemoryDC = nullptr;
    HBITMAP hBitmap = nullptr;
    HBITMAP hOldBitmap = nullptr;
    int width = 0;
    int height = 0;
    RECT rect = { 0, };

    WindowCapture(HWND hwnd);
    ~WindowCapture();
};

cv::Mat CaptureGameWindow(const char* windowName);