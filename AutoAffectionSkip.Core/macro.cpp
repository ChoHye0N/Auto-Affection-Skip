#include "pch.h"
#include "macro.h"

using namespace cv;

extern "C" __declspec(dllexport)
ButtonInfo FindButtonAndClick(const char* templatePath, double threshold = 0.9) {
    ButtonInfo info = { 0, 0, false };

    Mat screen = CaptureScreen();

    // 알파 채널 포함 이미지 불러오기
    Mat buttonWithAlpha = cv::imread(templatePath, cv::IMREAD_UNCHANGED);
    if (buttonWithAlpha.empty())
        return info;

    // 알파 채널 분리
    Mat button, alphaMask;
    if (buttonWithAlpha.channels() == 4) {
        std::vector<Mat> channels;
        split(buttonWithAlpha, channels);

        alphaMask = channels[3];
        cvtColor(buttonWithAlpha, button, COLOR_BGRA2BGR);
    }
    else {
        button = buttonWithAlpha;
    }

    // 알파 마스크 이진화
    if (!alphaMask.empty()) {
        cv::threshold(alphaMask, alphaMask, 1, 255, THRESH_BINARY);
    }

    Mat result;

    // 마스크 지원 매칭 방식 사용
    if (!alphaMask.empty()) {
        matchTemplate(
            screen,
            button,
            result,
            TM_CCORR_NORMED,
            alphaMask
        );
    }
    else {
        matchTemplate(
            screen,
            button,
            result,
            TM_CCORR_NORMED
        );
    }

    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
    info.score = maxVal;

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