#include <wrl\module.h>
#include <wrl\implements.h>
#include <combaseapi.h>
#include <windows.foundation.h>

const wchar_t* STA = L"STA";
const wchar_t* MTA = L"MTA";
const wchar_t* CURRENT = L"CURRENT";
const wchar_t* NA = L"NA";
const wchar_t* MAIN_STA = L"MAIN_STA";

namespace RegFreeWinRtTest
{
	class TestComp :
		public Microsoft::WRL::RuntimeClass<ABI::Windows::Foundation::IStringable>
	{
		InspectableClass(L"RegFreeWinRtTest.TestComp", BaseTrust);

	public:

		IFACEMETHOD(ToString)(__out HSTRING* result);

	};
	ActivatableClass(TestComp);
}