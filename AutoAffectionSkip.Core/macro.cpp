#include "pch.h"
#include "macro.h"

using namespace cv;

extern "C" __declspec(dllexport)
ButtonInfo FindButtonAndClick(const char* templatePath, double threshold = 0.9) {
    ButtonInfo info = { 0, 0, false };

    Mat screen = CaptureScreen();
    Mat button = cv::imread(templatePath);

    if (button.empty()) return info;

    Mat result;
    matchTemplate(screen, button, result, TM_CCOEFF_NORMED);

    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    if (maxVal > threshold) {
        info.isFound = true;
        info.x = maxLoc.x + button.cols / 2;
        info.y = maxLoc.y + button.rows / 2;

        // 마우스 클릭
        INPUT input[2] = {};
        input[0].type = INPUT_MOUSE;
        input[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
        input[0].mi.dx = (info.x * 65535) / GetSystemMetrics(SM_CXSCREEN);
        input[0].mi.dy = (info.y * 65535) / GetSystemMetrics(SM_CYSCREEN);

        input[1].type = INPUT_MOUSE;
        input[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;

        SendInput(2, input, sizeof(INPUT));
    }

    return info;
}