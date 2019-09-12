using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Xunit;
//using TestComp.Manual;
using TestComp;

namespace UnitTest
{
    public class TestCompTests
    {
        public Class TestObject { get; private set; }

        public TestCompTests()
        {
            TestObject = new Class();
        }

        [Fact]
        public void TestPrimitives()
        {
            var test_int = 21;
            TestObject.IntPropertyChanged += (Object sender, Int32 value) => {
                var c = (Class)sender;
                Assert.Equal(value, test_int);
            };
            TestObject.IntProperty = test_int;
        }

        [Fact]
        public void TestStrings()
        {
            string test_string = "x";
            string test_string2 = "y";

            var href = new WinRT.HStringReference(test_string);

            // In hstring from managed->native implicitly creates hstring reference 
            TestObject.StringProperty = test_string;

            // Out hstring from native->managed only creates System.String on demand
            var sp = TestObject.StringProperty;
            Assert.Equal(sp, test_string);

            // Out hstring from managed->native always creates HString from System.String
            TestObject.SetString(() => test_string2);
            Assert.Equal(TestObject.StringProperty, test_string2);

            // In hstring from native->managed only creates System.String on demand
            TestObject.StringPropertyChanged += (Class sender, WinRT.HString value) => sender.StringProperty2 = value;
            TestObject.GetString();
            Assert.Equal(TestObject.StringProperty2, test_string2);
        }
    }
}
