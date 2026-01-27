using System.Runtime.InteropServices;

namespace UI
{
    internal static class NativeMethods
    {
        [DllImport("AasCore.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern ButtonInfo FindImage(string templatePath, double threshold);

        [DllImport("AasCore.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void MouseClick(int x, int y);

        [DllImport("AasCore.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void KeyPressScan(ushort scan);

        [DllImport("AasCore.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public static extern int FindMultiImage(string templatePath, double threshold, [In, Out] ButtonInfo[] results, int maxCount);
    }
}