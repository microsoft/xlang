# Python/WinRT

The Windows Runtime Python Projection (Python/WinRT) enables Python developers to access
[Windows Runtime APIs](https://docs.microsoft.com/en-us/uwp/api/) directly from Python in a natural
and familiar way.

## Getting Started

### Prerequisites

* [Windows 10](https://www.microsoft.com/en-us/windows), October 2018 Update or later.
* [Python for Windows](https://docs.python.org/3.7/using/windows.html), version 3.7 or later
* [pip](https://pypi.org/project/pip/), version 19 or later

### Installing

Python/WinRT can be installed from the [Python Package Index](https://pypi.org/) via pip. Assuming
pip is on the path, Python/WinRT can be installed from the command line with the following command:

``` shell
> pip install winrt
```

You can test that Python/WinRT is installed correctly by launching Python and running the following
snippet of Python code. It should print "https://github.com/Microsoft/xlang/tree/master/src/tool/python"
to the console.

``` python
import winrt.windows.foundation as wf
u = wf.Uri("https://github.com/")
u2 = u.combine_uri("Microsoft/xlang/tree/master/src/tool/python")
print(str(u2))
```

## Using the Windows Runtime from Python

> For a full end-to-end sample of using Python/WinRT, please see the
> [WinML Tutorial](https://github.com/Microsoft/xlang/tree/master/samples/python/winml_tutorial)
> in the samples folder of the xlang GitHub repo.

The WinRT APIs are documented on [docs.microsoft.com](https://docs.microsoft.com/en-us/uwp/api/).
At this time, there is no official documentation for using WinRT from Python. However, this section
will describe how WinRT APIs are projected in Python.

### Namespace Modules

WinRT namespaces are projected in Python as modules. WinRT namespaces are projected in Python as
lower case to conform to standard Python naming conventions. WinRT namespaces are also prefixed
with the `winrt` package name. For example, the Windows.Devices.Geolocation namespace is projected
in Python as `winrt.windows.devices.geolocation`.

Importing a WinRT namespace module will automatically import namespace modules containing dependent
types, but will not automatically import child namespace modules. For example `winrt.windows.devices.geolocation`
will automatically import `winrt.windows.foundation` and `winrt.windows.foundation.collections` but
will not automatically import ``winrt.windows.devices.geolocation.geofencing`.

### Class Members

WinRT type members are projected in Python using snake_case, following standard Python naming conventions.

### Async Coroutines

WinRT methods that return one of the IAsync* interfaces are projected as Python coroutines. This means
they can be called using the await keyword from inside a Python
[coroutine function](https://docs.python.org/3/reference/compound_stmts.html#async-def). For example:

``` python
import winrt.windows.devices.geolocation as wdg

async def get_current_latitude():
    locator = wdg.Geolocator()
    pos = await locator.get_geoposition_async()
    return pos.coordinate.latitude
```

### Known Issues

* This release of the Python/WinRT does not support WinRT composable types. This includes most of
the classes in the Windows.UI.Composition and Windows.UI.Xaml namespaces. These namespaces are excluded
from Python/WinRT.
  * Note, a few methods in other namespaces reference types in these namespaces. For example,
  [MediaPlayerSurface](https://docs.microsoft.com/en-us/uwp/api/windows.media.playback.mediaplayersurface)
  is in the Windows.Media.Playback namespace but has properties that return types in the
  Windows.UI.Composition namespace. While these properties are available to Python, accessing them
  is not supported at this time.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE) file for details.

## Contributing

This project welcomes contributions and suggestions. Please visit our [GitHub repo](https://github.com/Microsoft/xlang/)
to file issues, make suggestions or submit pull requests.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
