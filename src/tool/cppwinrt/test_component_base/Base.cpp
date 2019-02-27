#include "pch.h"
#include "Base.h"

namespace winrt::test_component_base::implementation
{
    Base::Base(hstring const& name)
    {
        throw hresult_not_implemented();
    }
    void Base::BaseMethod()
    {
        throw hresult_not_implemented();
    }
}
