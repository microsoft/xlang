#include "Class.h"

namespace winrt::Component::implementation
{
    hstring Class::GetValue() const noexcept
    {
        return L"Class.Value";
    }
}
