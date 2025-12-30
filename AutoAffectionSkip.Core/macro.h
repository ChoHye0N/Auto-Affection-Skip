#pragma once

#include "imageProcessing.h"

struct ButtonInfo {
    int x;
    int y;
    bool isFound;
};

extern "C" __declspec(dllexport) ButtonInfo FindButtonAndClick(const char* templatePath, double threshold);