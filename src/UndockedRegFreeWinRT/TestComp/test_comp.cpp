#include <wrl\module.h>
#include <wrl\implements.h>
#include <combaseapi.h>
#include <windows.foundation.h>
#include <winstring.h>
#include "test_comp.h"

IFACEMETHODIMP
RegFreeWinRtTest::TestComp::ToString(__out HSTRING* result)
{
    APTTYPE aptType;
    APTTYPEQUALIFIER aptQualifier;
    // Returning apartment type via ToString
    if (CoGetApartmentType(&aptType, &aptQualifier) == S_OK)
    {
        if (aptType == APTTYPE_STA)
        {
            WindowsCreateString(STA,
                static_cast<UINT32>(::wcslen(STA)),
                result);
        }
        else if (aptType == APTTYPE_MTA)
        {
            WindowsCreateString(MTA,
                static_cast<UINT32>(::wcslen(MTA)),
                result);
        }
        else if (aptType == APTTYPE_CURRENT)
        {
            WindowsCreateString(CURRENT,
                static_cast<UINT32>(::wcslen(CURRENT)),
                result);
        }
        else if (aptType == APTTYPE_NA)
        {
            WindowsCreateString(NA,
                static_cast<UINT32>(::wcslen(NA)),
                result);
        }
        else if (aptType == APTTYPE_MAINSTA)
        {
            WindowsCreateString(MAIN_STA,
                static_cast<UINT32>(::wcslen(MAIN_STA)),
                result);
        }
        else
        {
            return E_FAIL;
        }
    }
    return S_OK;
}