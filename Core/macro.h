#pragma once

#include "imageProcessing.h"

extern HWND g_hwnd;
extern int g_captureOption;

struct ButtonInfo {
    int x;
    int y;
    bool isFound;
    double score;
};

struct SearchContext {
    cv::Mat screen;
    double scaleX, scaleY;
};

SearchContext PrepareSearch();
bool LoadTemplate(const char* path, cv::Mat& btn, cv::Mat& mask);

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) int Initialize(int captureOption);
    __declspec(dllexport) ButtonInfo FindImage(const char* templatePath, double threshold);
    __declspec(dllexport) int FindMultiImage(const char* templatePath, double threshold, ButtonInfo* outResults, int maxCount);
    __declspec(dllexport) void MouseClick(int x, int y);
    __declspec(dllexport) void KeyPressScan(WORD scan);

#ifdef __cplusplus
}
#endif