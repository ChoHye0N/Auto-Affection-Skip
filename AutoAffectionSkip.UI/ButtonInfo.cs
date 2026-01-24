using System.Runtime.InteropServices;

namespace AutoAffectionSkip.UI
{
    public struct ButtonInfo
    {
        public int x, y;               // 이미지의 좌표
        [MarshalAs(UnmanagedType.I1)]
        public bool found;             // 이미지 찾음 여부
        public double score;           // 이미지 정확도
    }

    public enum MacroStep
    {
        Idle,               // 시작 대기
        CheckMain,          // 메인화면
        EnterMomotalk,      // 모모톡 선택 / 메시지 창 열기
        ScanMessages,       // 화면에 보이는 메시지가 있는가?
        ProcessMessage,     // 읽을 메시지 선택 -> 채팅창 -> 답장 -> 스토리 입장
        SkipStory,          // 스킵 버튼 누르기
        ClickMenu,          // 메뉴 버튼 누르기
        ClaimReward,        // 확인 버튼 누르기 -> 보상 획득
        Finish              // 종료
    }
}
