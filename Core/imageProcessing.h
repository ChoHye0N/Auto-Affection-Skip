#pragma once

enum class CaptureOption : int {
    BitBlt = 0,
    PrintWindow = 1
};

cv::Mat CaptureGameWindow(HWND hwnd, int captureOption);

// 캡처용 GDI 자원(DC/비트맵) 해제. 프로세스 종료(DLL_PROCESS_DETACH) 시 호출.
void ReleaseCaptureResources();
