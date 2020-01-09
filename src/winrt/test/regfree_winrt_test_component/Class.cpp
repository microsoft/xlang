#include "pch.h"
#include "Class.h"
#include "Class.g.cpp"

namespace winrt::regfree_winrt_test_component::implementation
{
    int32_t Class::MyProperty()
    {
		return property;
    }

    void Class::MyProperty(int32_t value)
    {
		property = value;
    }

	int32_t Class::GetApartmentType()
	{
		APTTYPE aptType;
		APTTYPEQUALIFIER aptQualifier;
		if (CoGetApartmentType(&aptType, &aptQualifier) == S_OK)
		{
			return static_cast<int32_t>(aptType);
		}
		throw hresult_error();
	}
}
