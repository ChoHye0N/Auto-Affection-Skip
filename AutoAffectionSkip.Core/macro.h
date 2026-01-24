#pragma once

#include "imageProcessing.h"

struct ButtonInfo {
    int x;
    int y;
    bool isFound;
    double score;
};

enum class MacroStep {
    IDLE,                   // 시작 및 대기
    CHECK_MAIN_NOTI,        // [조건] 메인화면: 읽을 메시지가 있는가?
    ENTER_MOMOTALK,         // 모모톡 선택 및 메시지 창 열기
    SCAN_MESSAGES,          // [조건] 화면에 보이는 메시지가 있는가?
    SELECT_MESSAGE,         // 읽을 메시지 선택
    OPEN_CHAT,              // 채팅창 누르기
    REPLY_MESSAGE,          // 답장하기
    ENTER_STORY,            // 인연 스토리 입장
    SELECT_MENU,            // 메뉴 버튼 누르기
    SKIP_STORY,             // 스킵 버튼 누르기
    CLAIM_REWARD,           // 확인 버튼 누르기 및 보상 획득
    RETURN_TO_MAIN,         // 다시 메인화면으로 복귀 (루프)
    FINISH                  // 종료
};

extern "C" __declspec(dllexport)
ButtonInfo FindImage(const char* templatePath, double threshold);
extern "C" __declspec(dllexport) void MouseClick(int x, int y);
extern "C" __declspec(dllexport) void KeyPressScan(WORD scan);