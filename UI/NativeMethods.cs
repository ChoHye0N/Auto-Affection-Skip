using System.Runtime.InteropServices;

namespace UI
{
    // DLL 오류 및 이름 바꿔야 할 경우 생기면 수정
    internal static class NativeMethods
    {
        [DllImport("Core.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern ButtonInfo FindImage(string templatePath, double threshold);

        [DllImport("Core.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int FindMultiImage(string templatePath, double threshold, [In, Out] ButtonInfo[] results, int maxCount);

        [DllImport("Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void MouseClick(int x, int y);

        [DllImport("Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void KeyPressScan(ushort scan);
    }
}