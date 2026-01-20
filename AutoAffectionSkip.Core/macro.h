#pragma once

#include "imageProcessing.h"

struct ButtonInfo {
    int x;
    int y;
    bool isFound;
    double score;
};

enum class MacroStep
{
    // TODO: 플로우차트 복구해서 진행도 구현
};

extern "C" __declspec(dllexport) ButtonInfo FindButtonAndClick(
    const char* templatePath, double threshold
);