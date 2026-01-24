using System.Runtime.InteropServices;
using AutoAffectionSkip.UI;

namespace AutoAffectionSkip.UI
{
    internal static class NativeMethods
    {
        [DllImport("AutoAffectionSkip.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern ButtonInfo FindImage(string templatePath, double threshold);

        [DllImport("AutoAffectionSkip.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void MouseClick(int x, int y);

        [DllImport("AutoAffectionSkip.Core.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void KeyPressScan(ushort scan);
    }
}