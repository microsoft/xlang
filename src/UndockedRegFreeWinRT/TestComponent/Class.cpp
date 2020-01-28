#include "pch.h"
#include "Class.h"
#include "Class.g.cpp"

namespace winrt::TestComponent::implementation
{
    int32_t Class::Apartment()
    {
        APTTYPE aptType;
        APTTYPEQUALIFIER aptQualifier;
        check_hresult(CoGetApartmentType(&aptType, &aptQualifier));
        return aptType;
    }

    void Class::Apartment(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
