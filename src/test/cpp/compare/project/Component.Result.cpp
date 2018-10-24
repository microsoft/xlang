#include "pch.h"
#include "Result.h"

namespace winrt::Component::implementation
{
    int32_t Result::GetInt32()
    {
        throw hresult_not_implemented();
    }
    hstring Result::GetString()
    {
        throw hresult_not_implemented();
    }
    Component::Fast::FastClass Result::GetFastClass()
    {
        throw hresult_not_implemented();
    }
    Component::Fast::SlowClass Result::GetSlowClass()
    {
        throw hresult_not_implemented();
    }
    Component::INotExclusive Result::GetInterface()
    {
        throw hresult_not_implemented();
    }
}
