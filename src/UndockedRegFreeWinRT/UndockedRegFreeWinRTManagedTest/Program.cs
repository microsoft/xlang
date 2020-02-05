using System;
using System.Runtime.InteropServices;
using TestComponent;

namespace UndockedRegFreeWinRTManagedTest
{
    class Program
    {
        [DllImport("winrtact.dll")]
        static extern void winrtact_Initialize();

        public static bool succeeded;

        static void TestClassBoth(int expected)
        {
            try
            {
                TestComponent.ClassBoth c = new ClassBoth();
                succeeded = (expected == c.Apartment);
            }
            catch
            {
                succeeded = false;
            }
        }

        static void TestClassMta(int expected)
        {
            try 
            {
                TestComponent.ClassMta c = new ClassMta();
                succeeded = (expected == c.Apartment);
            }
            catch
            {
                succeeded = false;
            }
        }

        static void TestClassSta(int expected)
        {
            try
            {
                TestComponent.ClassSta c = new ClassSta();
                succeeded = (expected == c.Apartment);
            }
            catch
            {
                succeeded = false;
            }
        }

        static int Main(string[] args)
        {
            winrtact_Initialize();
            System.Threading.Thread testThread;

            succeeded = false;
            testThread = new System.Threading.Thread(new System.Threading.ThreadStart(() => TestClassBoth(3)));
            testThread.SetApartmentState(System.Threading.ApartmentState.STA);
            testThread.Start();
            testThread.Join();
            if (!succeeded) return 1;

            succeeded = false;
            testThread = new System.Threading.Thread(new System.Threading.ThreadStart(() => TestClassBoth(1)));
            testThread.SetApartmentState(System.Threading.ApartmentState.MTA);
            testThread.Start();
            testThread.Join();
            if (!succeeded) return 1;

            succeeded = false;
            testThread = new System.Threading.Thread(new System.Threading.ThreadStart(() => TestClassMta(1)));
            testThread.SetApartmentState(System.Threading.ApartmentState.STA);
            testThread.Start();
            testThread.Join();
            if (!succeeded) return 1;

            succeeded = false;
            testThread = new System.Threading.Thread(new System.Threading.ThreadStart(() => TestClassSta(1)));
            testThread.SetApartmentState(System.Threading.ApartmentState.MTA);
            testThread.Start();
            testThread.Join();
            if (succeeded) return 1;

            return 0;
        }
    }
}
