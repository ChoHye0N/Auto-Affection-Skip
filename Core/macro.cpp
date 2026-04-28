#include "pch.h"
#include "macro.h"

using namespace std;
using namespace cv;

HWND g_hwnd = nullptr;

extern "C" __declspec(dllexport)
bool Initialize() {
    g_hwnd = FindWindowA("UnityWndClass", "Blue Archive");
    if (!g_hwnd) return false;

    return true;
}

// 캡처 후 1920x1080으로 리사이즈 및 스케일 정보 반환
SearchContext PrepareSearch() {
    SearchContext ctx = { Mat(), 1.0, 1.0 };

    RECT clientRect;
    GetClientRect(g_hwnd, &clientRect);

    ctx.screen = CaptureGameWindow(g_hwnd);
    if (ctx.screen.empty()) return ctx;

    ctx.scaleX = clientRect.right / 1920.0;
    ctx.scaleY = clientRect.bottom / 1080.0;

    resize(ctx.screen, ctx.screen, Size(1920, 1080), 0, 0, INTER_CUBIC);

    return ctx;
}

// 템플릿 이미지 로드 및 알파 채널 처리
bool LoadTemplate(const char* path, Mat& btn, Mat& mask) {
    Mat raw = imread(path, IMREAD_UNCHANGED);
    if (raw.empty()) return false;

    if (raw.channels() == 4) {
        vector<Mat> channels;
        split(raw, channels);
        mask = channels[3];
        cvtColor(raw, btn, COLOR_BGRA2BGR);
        cv::threshold(mask, mask, 1, 255, THRESH_BINARY);
    }
    else {
        btn = raw;
        mask = Mat();
    }

    return true;
}

// 단일 객체 검출 (객체 정보 반환)
extern "C" __declspec(dllexport)
ButtonInfo FindImage(const char* templatePath, double threshold) {
    ButtonInfo info = { 0, 0, false, 0.0 };
    Mat button, mask, result;

    SearchContext ctx = PrepareSearch();
    if (ctx.screen.empty() || !LoadTemplate(templatePath, button, mask)) return info;

    if (!mask.empty()) {
        matchTemplate(ctx.screen, button, result, TM_CCORR_NORMED, mask);
    }
    else {
        matchTemplate(ctx.screen, button, result, TM_CCOEFF_NORMED);
    }

    double maxVal; Point maxLoc;
    minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    info.score = maxVal;
    if (maxVal >= threshold) {
        info.isFound = true;
        info.x = (int)((maxLoc.x + button.cols / 2.0) * ctx.scaleX);
        info.y = (int)((maxLoc.y + button.rows / 2.0) * ctx.scaleY);
    }

    return info;
}

// 복수 객체 검출 (객체 개수 반환)
extern "C" __declspec(dllexport)
int FindMultiImage(const char* templatePath, double threshold, ButtonInfo* outResults, int maxCount) {
    Mat button, mask, result;
    SearchContext ctx = PrepareSearch();

    if (ctx.screen.empty() || !LoadTemplate(templatePath, button, mask)) return 0;

    matchTemplate(ctx.screen, button, result, TM_CCORR_NORMED, mask);

    int count = 0;
    while (count < maxCount) {
        double maxVal; Point maxLoc;
        minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

        if (maxVal < threshold) break;

        outResults[count] = {
            (int)((maxLoc.x + button.cols / 2.0) * ctx.scaleX),
            (int)((maxLoc.y + button.rows / 2.0) * ctx.scaleY),
            true, maxVal
        };

        count++;

        // 검출 영역 제외
        Rect ignoreRect(maxLoc.x - 5, maxLoc.y - 5, button.cols + 10, button.rows + 10);
        rectangle(result, ignoreRect & Rect(0, 0, result.cols, result.rows), Scalar(0), -1);
    }

    return count;
}

extern "C" __declspec(dllexport)
void MouseClick(int x, int y) {
    if (!g_hwnd) return;

    POINT pt = { x, y };
    ClientToScreen(g_hwnd, &pt);

    double sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
    INPUT in[3] = {};

    in[0].type = INPUT_MOUSE;
    in[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    in[0].mi.dx = (long)(pt.x * 65535.0 / (sw - 1));
    in[0].mi.dy = (long)(pt.y * 65535.0 / (sh - 1));

    in[1].type = in[2].type = INPUT_MOUSE;
    in[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    in[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(3, in, sizeof(INPUT));
}

extern "C" __declspec(dllexport)
void KeyPressScan(WORD scan) {
    INPUT in[2] = {};

    in[0].type = in[1].type = INPUT_KEYBOARD;
    in[0].ki.wScan = in[1].ki.wScan = scan;
    in[0].ki.dwFlags = KEYEVENTF_SCANCODE;

    in[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    SendInput(2, in, sizeof(INPUT));
}