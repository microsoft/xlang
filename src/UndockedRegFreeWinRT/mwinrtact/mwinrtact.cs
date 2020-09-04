using System.Runtime.InteropServices;

namespace Microsoft.Windows
{
    public static class UndockedRegFreeWinrt
    {
        [DllImport("winrtact.dll")]
        static extern void winrtact_Initialize();

        public static void Initialize()
        {
            winrtact_Initialize();
        }
    }
}