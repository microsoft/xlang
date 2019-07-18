using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Microsoft.VisualStudio.TestPlatform.TestExecutor;

namespace UnitTestUWP
{
    sealed partial class App : Application
    {
        public App()
        {
        }

        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {
            UnitTestClient.Run(e.Arguments);
        }
    }
}
