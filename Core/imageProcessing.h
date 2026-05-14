#pragma once

enum class CaptureOption : int {
    BitBlt = 0,
    PrintWindow = 1
};

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
    RECT windowRect = { 0, };
    RECT clientRect = { 0, };

    WindowCapture(HWND hwnd, int captureOption);
    ~WindowCapture();
};

cv::Mat CaptureGameWindow(HWND hwnd, int captureOption);