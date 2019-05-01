#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "winrt/base.h"
#include "winrt/test_component_fast.Composition.h"

using namespace winrt::test_component_fast::Composition;

using namespace winrt;

int main(int const argc, char** argv)
{
    //init_apartment();
    //return Catch::Session().run(argc, argv);

    Compositor compositor;

    SpriteVisual v1 = compositor.CreateSpriteVisual();
    SpriteVisual v2 = compositor.CreateSpriteVisual();

    // Under the slowabi, this needs a QI for ICompositionObject.
    assert(v1.Compositor() == compositor);

    [](CompositionObject const& object) { object.StartAnimationGroup(); }(v1);

    static_cast<CompositionObject const&>(v1).StartAnimationGroup();

    // Under the slowabi, this needs a QI for ICompositionObject2.
    v1.StartAnimationGroup();

    // Under the slowabi, this needs a QI for IVisual for each call.
    v1.Offset(123);
    assert(v1.Offset() == 123);

    // Under the slowabi, this needs a QI for IVisual2 to call ParentForTransform
    // *and* the v2 parameter needs a QI for IVisual.
    v1.ParentForTransform(v2);

    // Since Brush is on the default interface of SpriteVisual there's no QI here
    // for both slowabi and fastabi.
    v1.Brush();

    // Under the slowabi, this needs a QI for ISpriteVisual2.
    v1.Shadow();

    // Non exclusive interface requires QI as usual.
    v1.Close();
}

CATCH_TRANSLATE_EXCEPTION(hresult_error const& e)
{
    return to_string(e.message());
}

extern "C"
{
    int32_t WINRT_CALL OS_RoGetActivationFactory(void* classId, guid const& iid, void** factory) noexcept;
}

#ifdef _M_IX86
#pragma comment(linker, "/alternatename:_OS_RoGetActivationFactory@12=_RoGetActivationFactory@12")
#else
#pragma comment(linker, "/alternatename:OS_RoGetActivationFactory=RoGetActivationFactory")
#endif

bool starts_with(std::wstring_view value, std::wstring_view match) noexcept
{
    return 0 == value.compare(0, match.size(), match);
}

int32_t WINRT_CALL WINRT_RoGetActivationFactory(void* classId, guid const& iid, void** factory) noexcept
{
    *factory = nullptr;
    std::wstring_view name{ WINRT_WindowsGetStringRawBuffer(classId, nullptr) };
    HMODULE library{ nullptr };

    if (starts_with(name, L"test_component_fast."))
    {
        library = LoadLibraryW(L"test_component_fast.dll");
    }
    else
    {
        return OS_RoGetActivationFactory(classId, iid, factory);
    }

    if (!library)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    using DllGetActivationFactory = HRESULT __stdcall(void* classId, void** factory);
    auto call = reinterpret_cast<DllGetActivationFactory*>(GetProcAddress(library, "DllGetActivationFactory"));

    if (!call)
    {
        HRESULT const hr = HRESULT_FROM_WIN32(GetLastError());
        WINRT_VERIFY(FreeLibrary(library));
        return hr;
    }

    winrt::com_ptr<winrt::Windows::Foundation::IActivationFactory> activation_factory;
    HRESULT const hr = call(classId, activation_factory.put_void());

    if (FAILED(hr))
    {
        WINRT_VERIFY(FreeLibrary(library));
        return hr;
    }

    if (iid != winrt::guid_of<winrt::Windows::Foundation::IActivationFactory>())
    {
        return activation_factory->QueryInterface(iid, factory);
    }

    *factory = activation_factory.detach();
    return S_OK;
}
