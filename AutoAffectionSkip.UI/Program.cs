using System.Runtime.InteropServices;

namespace AutoAffectionSkip.UI
{
    internal static class Program
    {
        [STAThread]
        static void Main()
        {
            ApplicationConfiguration.Initialize();

            // DLL 호출 테스트
            try
            {
                ButtonInfo info = FindButtonAndClick("button_template.png", 0.9);
                if (info.found)
                    Console.WriteLine($"Button clicked at ({info.x},{info.y})");
                else
                    Console.WriteLine("Button not found");
            }
            catch (Exception ex)
            {
                Console.WriteLine("DLL 호출 중 오류: " + ex.Message);
            }

            // UI 시작
            Application.Run(new Form1());
        }

        [DllImport("AutoAffectionSkip.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ButtonInfo FindButtonAndClick(string templatePath, double threshold);
    }

    struct ButtonInfo
    {
        public int x;
        public int y;
        [MarshalAs(UnmanagedType.I1)]
        public bool found;
    }
}
