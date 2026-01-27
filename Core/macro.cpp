#include "pch.h"
#include "macro.h"

using namespace std;
using namespace cv;

// 1. 이미지 탐색 (단일 객체 검출)
extern "C" __declspec(dllexport)
ButtonInfo FindImage(const char* templatePath, double threshold) {
    ButtonInfo info = { 0, 0, false, 0.0 };
    Mat button, alphaMask, result;

    // 게임 화면 캡처
    Mat screen = CaptureGameWindow("Blue Archive");
    if (screen.empty()) return info;

    // 이미지 로드
    Mat buttonWithAlpha = imread(templatePath, IMREAD_UNCHANGED);
    if (buttonWithAlpha.empty()) return info;

    // 알파 채널 처리 (마스크 생성)
    if (buttonWithAlpha.channels() == 4) {
        std::vector<Mat> channels;
        split(buttonWithAlpha, channels);
        alphaMask = channels[3];
        cvtColor(buttonWithAlpha, button, COLOR_BGRA2BGR);
        cv::threshold(alphaMask, alphaMask, 1, 255, THRESH_BINARY);
    }
    else {
        button = buttonWithAlpha;
    }

    // 템플릿 매칭
    if (!alphaMask.empty()) {
        matchTemplate(screen, button, result, TM_CCORR_NORMED, alphaMask);
    }
    else {
        matchTemplate(screen, button, result, TM_CCOEFF_NORMED);
    }

    double maxVal;
    Point maxLoc;
    minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    info.score = maxVal;
    if (maxVal >= threshold) {
        info.isFound = true;

        // 중앙 좌표 계산
        info.x = maxLoc.x + button.cols / 2;
        info.y = maxLoc.y + button.rows / 2;
    }

    return info;
}

// 2. 이미지 탐색 (여러 객체 검출)
extern "C" __declspec(dllexport)
int FindMultiImage(const char* templatePath, double threshold, ButtonInfo* outResults, int maxCount) {
    Mat button, alphaMask, result;
    Mat screen = CaptureGameWindow("Blue Archive");

    Mat buttonWithAlpha = imread(templatePath, IMREAD_UNCHANGED);
    if (screen.empty() || buttonWithAlpha.empty()) return 0;

    // 알파 채널 처리 (마스크 생성)
    if (buttonWithAlpha.channels() == 4) {
        std::vector<Mat> channels;
        split(buttonWithAlpha, channels);
        alphaMask = channels[3];
        cvtColor(buttonWithAlpha, button, COLOR_BGRA2BGR);
        cv::threshold(alphaMask, alphaMask, 1, 255, THRESH_BINARY);
    }
    else {
        button = buttonWithAlpha;
    }

    matchTemplate(screen, button, result, TM_CCORR_NORMED, alphaMask);

    int foundCount = 0;
    while (foundCount < maxCount) {
        double maxVal;
        Point maxLoc;
        minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

        if (maxVal < threshold) break;

        // 결과 채우기
        outResults[foundCount].isFound = true;
        outResults[foundCount].score = maxVal;
        outResults[foundCount].x = maxLoc.x + button.cols / 2;
        outResults[foundCount].y = maxLoc.y + button.rows / 2;

        foundCount++;

        // 중복 검출 방지 범위를 이미지 크기보다 약간 더 넓게 설정
        int padding = 10;
        Rect foundRect(
            maxLoc.x - padding,
            maxLoc.y - padding,
            button.cols + (padding * 2),
            button.rows + (padding * 2)
        );

        // 결과 맵의 실제 유효 범위를 벗어나지 않도록 교집합 계산
        Rect scanBoundary(0, 0, result.cols, result.rows);
        rectangle(result, foundRect & scanBoundary, Scalar(0), -1);
    }

    return foundCount;
}

// 3. 마우스 클릭
extern "C" __declspec(dllexport)
void MouseClick(int x, int y) {
    // 좌표를 화면 절대 좌표로 변환 (0~65535)
    double screenW = GetSystemMetrics(SM_CXSCREEN);
    double screenH = GetSystemMetrics(SM_CYSCREEN);

    INPUT input[3] = {};

    // 마우스 이동
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    input[0].mi.dx = (long)((x * 65535) / screenW);
    input[0].mi.dy = (long)((y * 65535) / screenH);

    // 마우스 누르기
    input[1].type = INPUT_MOUSE;
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    // 마우스 떼기
    input[2].type = INPUT_MOUSE;
    input[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(3, input, sizeof(INPUT));
}

// 4. 버튼 입력
extern "C" __declspec(dllexport)
void KeyPressScan(WORD scan) {
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wScan = scan;
    inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wScan = scan;
    inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}