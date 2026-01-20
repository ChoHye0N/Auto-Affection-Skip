using System.Runtime.InteropServices;

namespace AutoAffectionSkip.UI
{
    internal static class Program
    {
        // 디버그 콘솔 출력용
        [DllImport("kernel32.dll")]
        static extern bool AllocConsole();

        [DllImport("AutoAffectionSkip.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ButtonInfo FindButtonAndClick(
            string templatePath, double threshold
        );

        [STAThread]
        static void Main()
        {
            AllocConsole();
            ApplicationConfiguration.Initialize();

            string imgPath = Path.Combine(
                AppContext.BaseDirectory,
                "assets",
                "images",
                "menu_btn.png"
            );

            // DLL 호출 테스트
            try
            {
                ButtonInfo info = FindButtonAndClick(imgPath, 0.9);

                if (info.found)
                {
                    Console.WriteLine($"Button clicked at " +
                        $"({info.x},{info.y}) [Score: {info.score}]");
                }

                else
                {
                    Console.WriteLine("Button not found");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("DLL 호출 중 오류: " + ex.Message);
            }

            // UI 시작
            Application.Run(new Form1());
        }
    }

    struct ButtonInfo
    {
        public int x, y;
        [MarshalAs(UnmanagedType.I1)]
        public bool found;
        public double score;
    }
}
