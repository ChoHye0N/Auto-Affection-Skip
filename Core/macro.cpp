#include "pch.h"
#include "macro.h"
#include <string>
#include <unordered_map>

using namespace std;
using namespace cv;

HWND g_hwnd = nullptr;
int g_captureOption = NULL;
ROI g_roi = {};

// 같은 틱(짧은 시간) 안에서 여러 템플릿을 검사할 때 화면 캡처를 재사용하기 위한 캐시
namespace {
    constexpr DWORD FRAME_CACHE_TTL_MS = 100;

    struct FrameCache {
        Mat frame; // 리사이즈까지 끝난 1920x1080 기준 전체 화면 (ROI 적용 전)
        double scaleX = 1.0;
        double scaleY = 1.0;
        DWORD timestamp = 0;
    } g_frameCache;

    struct TemplateEntry {
        Mat button;
        Mat mask;
    };
    unordered_map<string, TemplateEntry> g_templateCache;
}

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

    // 매크로를 새로 시작할 때 이전 실행에서 남은 캡처 캐시를 사용하지 않도록 초기화
    g_frameCache.timestamp = 0;
    g_frameCache.frame.release();

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

    DWORD now = GetTickCount();

    // 직전 캡처가 아직 유효하면(같은 루프 틱 안에서의 반복 호출) 재캡처하지 않고 재사용
    if (!g_frameCache.frame.empty() && (now - g_frameCache.timestamp) < FRAME_CACHE_TTL_MS) {
        ctx.screen = g_frameCache.frame;
        ctx.scaleX = g_frameCache.scaleX;
        ctx.scaleY = g_frameCache.scaleY;
    }
    else {
        RECT clientRect;
        GetClientRect(g_hwnd, &clientRect);

        Mat captured = CaptureGameWindow(g_hwnd, g_captureOption);
        if (captured.empty()) return ctx;

        double scaleX = clientRect.right / 1920.0;
        double scaleY = clientRect.bottom / 1080.0;

        // 이미 1920x1080이면 리사이즈를 건너뜀. 리사이즈가 필요한 경우도 매칭 목적이면
        // INTER_CUBIC보다 훨씬 저렴한 INTER_LINEAR로 충분함
        if (captured.cols != 1920 || captured.rows != 1080) {
            resize(captured, captured, Size(1920, 1080), 0, 0, INTER_LINEAR);
        }

        g_frameCache.frame = captured;
        g_frameCache.scaleX = scaleX;
        g_frameCache.scaleY = scaleY;
        g_frameCache.timestamp = now;

        ctx.screen = captured;
        ctx.scaleX = scaleX;
        ctx.scaleY = scaleY;
    }

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

// 템플릿 이미지 로드 및 알파 채널 처리 (경로별로 한 번만 디코딩하고 이후엔 캐시에서 반환)
bool LoadTemplate(const char* path, Mat& btn, Mat& mask) {
    auto cached = g_templateCache.find(path);
    if (cached != g_templateCache.end()) {
        btn = cached->second.button;
        mask = cached->second.mask;
        return true;
    }

    Mat raw = imread(path, IMREAD_UNCHANGED);
    if (raw.empty()) return false;

    Mat button, alphaMask;
    if (raw.channels() == 4) {
        vector<Mat> channels;
        split(raw, channels);
        alphaMask = channels[3];
        cvtColor(raw, button, COLOR_BGRA2BGR);
        cv::threshold(alphaMask, alphaMask, 1, 255, THRESH_BINARY);
    }
    else {
        button = raw;
    }

    g_templateCache.emplace(string(path), TemplateEntry{ button, alphaMask });
    btn = button;
    mask = alphaMask;

    return true;
}

// FindImage/FindMultiImage 공통 준비 과정 (캡처 확보 + 템플릿 로드 + 크기 검증)
static bool PrepareMatch(const char* templatePath, SearchContext& ctx, Mat& button, Mat& mask) {
    ctx = PrepareSearch();
    if (ctx.screen.empty() || !LoadTemplate(templatePath, button, mask)) return false;

    // ROI가 템플릿보다 작으면 매칭 불가
    if (ctx.screen.cols < button.cols || ctx.screen.rows < button.rows) return false;

    return true;
}

// 단일 객체 검출 (객체 정보 반환)
extern "C" __declspec(dllexport)
ButtonInfo FindImage(const char* templatePath, double threshold) {
    ButtonInfo info = { 0, 0, false, 0.0 };
    SearchContext ctx;
    Mat button, mask, result;

    if (!PrepareMatch(templatePath, ctx, button, mask)) return info;

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
    SearchContext ctx;
    Mat button, mask, result;

    if (!PrepareMatch(templatePath, ctx, button, mask)) return 0;

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
