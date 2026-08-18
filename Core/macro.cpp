#include "pch.h"
#include "macro.h"

using namespace std;
using namespace cv;

HWND g_hwnd = nullptr;
int g_captureOption = NULL;
ROI g_roi = {};

// ROI 설정 (1920x1080 기준 좌표/크기)
extern "C" __declspec(dllexport)
void SetROI(int x, int y, int width, int height) {
    g_roi.x = x;
    g_roi.y = y;
    g_roi.width = width;
    g_roi.height = height;
    g_roi.enabled = true;
}

// ROI 해제 (다시 전체 화면 검색)
extern "C" __declspec(dllexport)
void ClearROI() {
    g_roi.enabled = false;
}

// ROI 활성화 여부
extern "C" __declspec(dllexport)
bool IsROIEnabled() {
    return g_roi.enabled;
}

extern "C" __declspec(dllexport)
int Initialize(int captureOption) {
    // 추후 에뮬레이터 도입 예정
    g_captureOption = captureOption;
    wstring winName[] = { L"Blue Archive"};

    for (int i = 0; i < 1; i++) {
        g_hwnd = FindWindowW(L"UnityWndClass", winName[i].c_str());
        if (g_hwnd) return i;
    }

    return -1;
}

// 캡처 후 1920x1080으로 리사이즈 및 스케일 정보 반환
// ROI가 설정되어 있으면 해당 영역만 잘라서 반환하고, 오프셋을 ctx에 저장
SearchContext PrepareSearch() {
    SearchContext ctx = { Mat(), 1.0, 1.0, 0, 0 };

    RECT clientRect;
    GetClientRect(g_hwnd, &clientRect);

    ctx.screen = CaptureGameWindow(g_hwnd, g_captureOption);
    if (ctx.screen.empty()) return ctx;

    ctx.scaleX = clientRect.right / 1920.0;
    ctx.scaleY = clientRect.bottom / 1080.0;

    resize(ctx.screen, ctx.screen, Size(1920, 1080), 0, 0, INTER_CUBIC);

    // ROI 적용 (1920x1080 좌표 기준)
    if (g_roi.enabled) {
        Rect roiRect(g_roi.x, g_roi.y, g_roi.width, g_roi.height);
        // 화면 범위로 클램프
        roiRect &= Rect(0, 0, ctx.screen.cols, ctx.screen.rows); 

        if (roiRect.width > 0 && roiRect.height > 0) {
            ctx.screen = ctx.screen(roiRect).clone();
            ctx.roiOffsetX = roiRect.x;
            ctx.roiOffsetY = roiRect.y;
        }
        // ROI가 화면 밖으로 완전히 벗어난 경우엔 그냥 전체 화면으로 검색
    }

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

    // ROI가 템플릿보다 작으면 매칭 불가
    if (ctx.screen.cols < button.cols || ctx.screen.rows < button.rows) return info;

    if (!mask.empty()) {
        matchTemplate(ctx.screen, button, result, TM_CCORR_NORMED, mask);
    }
    else {
        matchTemplate(ctx.screen, button, result, TM_CCOEFF_NORMED);
    }

    double maxVal;
    Point maxLoc;

    minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

    info.score = maxVal;
    if (maxVal >= threshold) {
        info.isFound = true;
        // ROI 오프셋을 더해서 전체 화면(1920x1080) 기준 좌표로 복원 후 실제 창 크기로 스케일
        info.x = (int)((maxLoc.x + ctx.roiOffsetX + button.cols / 2.0) * ctx.scaleX);
        info.y = (int)((maxLoc.y + ctx.roiOffsetY + button.rows / 2.0) * ctx.scaleY);
    }

    return info;
}

// 복수 객체 검출 (객체 개수 반환)
extern "C" __declspec(dllexport)
int FindMultiImage(const char* templatePath, double threshold, ButtonInfo* outResults, int maxCount) {
    Mat button, mask, result;
    SearchContext ctx = PrepareSearch();

    if (ctx.screen.empty() || !LoadTemplate(templatePath, button, mask)) return 0;
    if (ctx.screen.cols < button.cols || ctx.screen.rows < button.rows) return 0;

    matchTemplate(ctx.screen, button, result, TM_CCORR_NORMED, mask);

    int count = 0;
    while (count < maxCount) {
        double maxVal;
        Point maxLoc;

        minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

        if (maxVal < threshold) break;

        outResults[count] = {
            (int)((maxLoc.x + ctx.roiOffsetX + button.cols / 2.0) * ctx.scaleX),
            (int)((maxLoc.y + ctx.roiOffsetY + button.rows / 2.0) * ctx.scaleY),
            true, maxVal
        };

        count++;

        // 검출 영역 제외
        Rect ignoreRect(maxLoc.x - 5, maxLoc.y - 5, button.cols + 10, button.rows + 10);
        rectangle(result, ignoreRect & Rect(0, 0, result.cols, result.rows), Scalar(0), -1);
    }

    return count;
}

// 마우스 클릭 (Win32)
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

// 키 입력 (Win32)
extern "C" __declspec(dllexport)
void KeyPressScan(WORD scan) {
    INPUT in[2] = {};

    in[0].type = in[1].type = INPUT_KEYBOARD;
    in[0].ki.wScan = in[1].ki.wScan = scan;
    in[0].ki.dwFlags = KEYEVENTF_SCANCODE;

    in[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;

    SendInput(2, in, sizeof(INPUT));
}