#include "pch.h"
#include <windows.ui.xaml.hosting.referencetracker.h>

using namespace winrt;
using namespace Windows::Foundation;

struct scratch : implements<scratch, IStringable, IReferenceTrackerExtension, IClosable>
{
    hstring ToString()
    {
        return L"scratch";
    }

    void Close()
    {
        IStringable a = *this;
        IClosable b = *this;

        auto c = static_cast<IStringable>(*this);
        auto d = static_cast<IClosable>(*this);
    }
};

TEST_CASE("scratch")
{
}
