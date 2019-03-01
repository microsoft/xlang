#include "pch.h"
#include <windows.ui.xaml.hosting.referencetracker.h>

using namespace winrt;

struct scratch : implements<scratch, IReferenceTrackerExtension>
{

};

TEST_CASE("scratch")
{
}
