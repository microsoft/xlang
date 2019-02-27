#include "pch.h"
#include "Base.h"

namespace winrt::test_component_base::implementation
{
    void Base::Method()
    {
        throw hresult_not_implemented();
    }
}
