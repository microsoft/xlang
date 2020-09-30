using System;
using System.Runtime.InteropServices;
using TestComponent;
using Microsoft.Windows;

namespace UndockedRegFreeWinRTManagedTest
{
    class Program
    {
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
            UndockedRegFreeWinrt.Initialize();
            Console.WriteLine("Undocked RegFree WinRT Managed Test - Starting");
            System.Threading.Thread testThread;

            succeeded = false;
            testThread = new System.Threading.Thread(new System.Threading.ThreadStart(() => TestClassBoth(3)));
            testThread.SetApartmentState(System.Threading.ApartmentState.STA);
            testThread.Start();
            testThread.Join();
            if (!succeeded)
            {
                Console.WriteLine("Both to STA test failed");
                return 1;
            }

            succeeded = false;
            testThread = new System.Threading.Thread(new System.Threading.ThreadStart(() => TestClassBoth(1)));
            testThread.SetApartmentState(System.Threading.ApartmentState.MTA);
            testThread.Start();
            testThread.Join();
            if (!succeeded)
            {
                Console.WriteLine("Both to MTA test failed");
                return 1;
            }

            succeeded = false;
            testThread = new System.Threading.Thread(new System.Threading.ThreadStart(() => TestClassMta(1)));
            testThread.SetApartmentState(System.Threading.ApartmentState.STA);
            testThread.Start();
            testThread.Join();
            if (!succeeded)
            {
                Console.WriteLine("MTA to STA test failed");
                return 1;
            }

            succeeded = false;
            testThread = new System.Threading.Thread(new System.Threading.ThreadStart(() => TestClassSta(1)));
            testThread.SetApartmentState(System.Threading.ApartmentState.MTA);
            testThread.Start();
            testThread.Join();
            if (succeeded)
            {
                Console.WriteLine("STA to MTA should failed");
                return 1;
            }

            Console.WriteLine("Undocked RegFree WinRT Managed Test - All tests passed");
            return 0;
        }
    }
}
