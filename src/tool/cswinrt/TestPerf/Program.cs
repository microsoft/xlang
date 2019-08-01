using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace TestPerf
{
    struct StringStats
    {
        public long in_native;
        public long out_native;
        public long in_managed;
        public long out_managed;
    };

    class Program
    {
        const long COUNT = 1;
        const long MILLION = 1000000;

        static void TestStrings(string test_string)
        {
            StringStats xl_stats;
            StringStats cs_stats;

            {
                var obj = new TestComp.Projected.Class();

                var sw = Stopwatch.StartNew();

                sw.Restart();
                var href = new WinRT.HStringReference(test_string);
                for (long i = 0; i < (COUNT * MILLION); i++)
                {
                    // In hstring from managed->native implicitly creates hstring reference 
                    obj.StringProperty = test_string; 
                }
                xl_stats.in_native = sw.ElapsedMilliseconds / COUNT;
                
                // Out hstring from native->managed only creates System.String on demand
                sw.Restart();
                for (long i = 0; i < (COUNT * MILLION); i++)
                {
                    var sp = obj.StringProperty;
                }
                xl_stats.out_managed = sw.ElapsedMilliseconds / COUNT;
                
                // Out hstring from managed->native always creates HString
                sw.Restart();
                for (long i = 0; i < (COUNT * MILLION); i++)
                {
                    obj.SetString(() => test_string);
                }
                xl_stats.out_native = sw.ElapsedMilliseconds / COUNT;
                
                // In hstring from native->managed only creates System.String on demand
                obj.StringPropertyChanged += (TestComp.Projected.Class sender, WinRT.HString value) =>
                {
                    sender.StringProperty2 = value; 
                };
                sw.Restart();
                for (long i = 0; i < (COUNT * MILLION); i++)
                {
                    obj.GetString();
                }
                xl_stats.in_managed = sw.ElapsedMilliseconds / COUNT;
            }

            {
                var obj = new TestComp.Class();

                var sw = Stopwatch.StartNew();

                sw.Restart();
                for (long i = 0; i < (COUNT * MILLION); i++)
                {
                    // Can't beat the marshaler here, which passes a direct pointer to the managed string
                    obj.StringProperty = test_string;
                }
                cs_stats.in_native = sw.ElapsedMilliseconds / COUNT;

                sw.Restart();
                for (long i = 0; i < (COUNT * MILLION); i++)
                {
                    var sp = obj.StringProperty;
                }
                cs_stats.out_managed = sw.ElapsedMilliseconds / COUNT;

                sw.Restart();
                for (long i = 0; i < (COUNT * MILLION); i++)
                {
                    obj.SetString(() => test_string);
                }
                cs_stats.out_native = sw.ElapsedMilliseconds / COUNT;

                obj.StringPropertyChanged += (TestComp.Class sender, String value) =>
                {
                    sender.StringProperty2 = value;
                };
                sw.Restart();
                for (long i = 0; i < (COUNT * MILLION); i++)
                {
                    obj.GetString();
                }
                cs_stats.in_managed = sw.ElapsedMilliseconds / COUNT;
            }

            Console.WriteLine("Testing string ops with: \"{0:S}\"", test_string);
            Console.WriteLine("in -> native:   C#: {0:D}nsec, xlang: {1:D}nsec ({2:P})", cs_stats.in_native, xl_stats.in_native, (float)(xl_stats.in_native - cs_stats.in_native) / cs_stats.in_native);
            Console.WriteLine("out -> managed: C#: {0:D}nsec, xlang: {1:D}nsec ({2:P})", cs_stats.out_managed, xl_stats.out_managed, (float)(xl_stats.out_managed - cs_stats.out_managed) / cs_stats.out_managed);
            Console.WriteLine("out -> native:  C#: {0:D}nsec, xlang: {1:D}nsec ({2:P})", cs_stats.out_native, xl_stats.out_native, (float)(xl_stats.out_native - cs_stats.out_native) / cs_stats.out_native);
            Console.WriteLine("in -> managed:  C#: {0:D}nsec, xlang: {1:D}nsec ({2:P})", cs_stats.in_managed, xl_stats.in_managed, (float)(xl_stats.in_managed - cs_stats.in_managed) / cs_stats.in_managed);
        }

        static void Main(string[] args)
        {
            // test short string
            TestStrings("foo");

            // test long string
            TestStrings(
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit, " +
                "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. " +
                "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi " +
                "ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in " +
                "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint " +
                "occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." +
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit, " +
                "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. " +
                "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi " +
                "ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in " +
                "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint " +
                "occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." +
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit, " +
                "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. " +
                "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi " +
                "ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in " +
                "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint " +
                "occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." +
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit, " +
                "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. " +
                "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi " +
                "ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in " +
                "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint " +
                "occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum." +
                "Lorem ipsum dolor sit amet, consectetur adipiscing elit, " +
                "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. " +
                "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi " +
                "ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in " +
                "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint " +
                "occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
            );
        }
    }
}
