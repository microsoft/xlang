#include "pch.h"
#include "Class.h"

namespace winrt::Component::Result::implementation
{
    int32_t Class::GetInt32();
    {
        throw hresult_not_implemented();
    }
    hstring Class::GetString();
    {
        throw hresult_not_implemented();
    }
    Component::Fast::FastClass Class::GetFastClass();
    {
        throw hresult_not_implemented();
    }
    Component::Fast::SlowClass Class::GetSlowClass();
    {
        throw hresult_not_implemented();
    }
    Component::INotExclusive Class::GetInterface();
    {
        throw hresult_not_implemented();
    }
}
