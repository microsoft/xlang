using System.Runtime.InteropServices;

namespace Microsoft.Windows
{
    static class UndockedRegFreeWinrt
    {
        [DllImport("winrtact.dll")]
        static extern void winrtact_Initialize();

        public static void Initialize()
        {
            winrtact_Initialize();
        }
    }
}