using System;
using System.Runtime.InteropServices;
using TestComponent;
using Microsoft.Windows;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace UndockedRegFreeWinRTManagedTest
{
    [TestClass]
    public class Program
    {
        public static bool succeeded;
        private static int testsRan;
        //private static int testsFailed;

        public Program()
        {
            UndockedRegFreeWinrt.Initialize();
        }

        static void TestClassBoth(int expected)
        {
            try
            {
                TestComponent.ClassBoth c = new ClassBoth();
                succeeded = (expected == c.Apartment);
            }
            catch(Exception e)
            {
                Console.WriteLine(e.Message);
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

        static bool RunTest(System.Threading.ThreadStart testThreadStart, System.Threading.ApartmentState apartmentState)
        {
            testsRan++;
            System.Threading.Thread testThread = new System.Threading.Thread(testThreadStart);
            succeeded = false;
            testThread.SetApartmentState(apartmentState);
            testThread.Start();
            testThread.Join();
            return succeeded;
        }

        [TestMethod]
        public void TestBothFromSTA()
        {
            Assert.IsTrue(RunTest(new System.Threading.ThreadStart(() => TestClassBoth(3)), System.Threading.ApartmentState.STA));
        }

        [TestMethod]
        public void TestBothFromMTA()
        {
            Assert.IsTrue(RunTest(new System.Threading.ThreadStart(() => TestClassBoth(1)), System.Threading.ApartmentState.MTA));
        }

        [TestMethod]
        public void TestMTAFromSTA()
        {
            Assert.IsTrue(RunTest(new System.Threading.ThreadStart(() => TestClassMta(1)), System.Threading.ApartmentState.STA));
        }

        [TestMethod]
        public void TestSTAFromMTA()
        {
            Assert.IsTrue(RunTest(new System.Threading.ThreadStart(() => TestClassSta(1)), System.Threading.ApartmentState.MTA));
        }

        public static int Main()
        {
            Program p = new Program();
            p.TestBothFromMTA();
            p.TestBothFromSTA();
            p.TestMTAFromSTA();
            p.TestSTAFromMTA();

            Console.WriteLine("Done!");
            
            return 0;
        }
    }

}


