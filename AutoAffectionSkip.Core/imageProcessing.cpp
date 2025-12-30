#include "pch.h"
#include "imageProcessing.h"

#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")

using namespace cv;

Mat CaptureScreen() {
    int screenX = GetSystemMetrics(SM_CXSCREEN);
    int screenY = GetSystemMetrics(SM_CYSCREEN);

    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenX, screenY);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, screenX, screenY, hScreenDC, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screenX;
    bi.biHeight = -screenY;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    Mat mat(screenY, screenX, CV_8UC3);
    GetDIBits(hMemoryDC, hBitmap, 0, screenY, mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    return mat;
}