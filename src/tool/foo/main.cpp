#include "pch.h"
#include "helpers.h"

#include "ffi.h"


// TODO: local declarations of winrt APIs to avoid dependencies on Windows SDK
#include <wrl\client.h>
#include <wrl\wrappers\corewrappers.h>
#include <windows.data.json.h>

namespace wrl 
{
    using namespace Microsoft::WRL;
    using namespace Microsoft::WRL::Wrappers;
} 

namespace xmd = xlang::meta::reader;

using namespace fooxlang;

struct winrt_ns
{
    std::map<std::string_view, winrt_ns> sub_namespaces{};
    xmd::cache::namespace_members members{};
};

xmd::cache::namespace_members const* find_ns(std::map<std::string_view, winrt_ns> const& namespaces, std::string_view const& ns) noexcept
{
    auto dot_pos = ns.find('.', 0);
    if (dot_pos == std::string_view::npos)
    {
        auto it = namespaces.find(ns);
        if (it == namespaces.end())
        {
            return nullptr;
        }
        else
        {
            return &(it->second.members);
        }
    }
    else
    {
        auto it = namespaces.find(ns.substr(0, dot_pos));
        if (it == namespaces.end())
        {
            return nullptr;
        }
        else
        {
            return find_ns(it->second.sub_namespaces, ns.substr(dot_pos + 1));
        }
    }
}

xmd::cache::namespace_members const& get_ns(std::map<std::string_view, winrt_ns> const& namespaces, std::string_view const& ns)
{
    auto _ns = find_ns(namespaces, ns);
    if (_ns == nullptr)
    {
        throw std::out_of_range{ std::string{ns} };
    }

    return *_ns;
}

auto get_system_metadata()
{
    namespace fs = std::experimental::filesystem;

#ifdef _WIN64
    auto sys32 = "c:\\Windows\\System32";
#else
    auto sys32 = "c:\\Windows\\Sysnative";
#endif

    auto winmd = fs::path{ sys32 } / "WinMetadata";

    std::vector<std::string> files;
    for (auto& p : fs::directory_iterator(winmd))
    {
        files.push_back(p.path().string());
    }

    return std::move(files);
}

auto get_namespace_map(xmd::cache const& c)
{
    std::map<std::string_view, winrt_ns> root_namespaces;

    for (auto&&[ns, members] : c.namespaces())
    {
        winrt_ns* curns{ nullptr };
        std::map<std::string_view, winrt_ns>* nsmap = &root_namespaces;

        for (auto&& s : get_dotted_name_segments(ns))
        {
            winrt_ns& q = (*nsmap)[s];
            curns = &q;
            nsmap = &(q.sub_namespaces);
        }

        curns->members = members;
    }

    return std::move(root_namespaces);
}

auto get_guid(xmd::CustomAttribute const& attrib)
{
    std::vector<xmd::FixedArgSig> z = attrib.Value().FixedArgs();

    auto getarg = [&z](std::size_t index) { return std::get<xmd::ElemSig>(z[index].value).value; };
    return GUID{
        std::get<uint32_t>(getarg(0)),
        std::get<uint16_t>(getarg(1)),
        std::get<uint16_t>(getarg(2)),
        {
            std::get<uint8_t>(getarg(3)),
            std::get<uint8_t>(getarg(4)),
            std::get<uint8_t>(getarg(5)),
            std::get<uint8_t>(getarg(6)),
            std::get<uint8_t>(getarg(7)),
            std::get<uint8_t>(getarg(8)),
            std::get<uint8_t>(getarg(9)),
            std::get<uint8_t>(getarg(10))
        }
    };
}

auto get_guid(xmd::TypeDef const& type)
{
    return get_guid(xmd::get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute"));
}

//ffi_type* foobarffi(method_signature::param_t const& type)
//{
//    //struct fiibarbaz: signature_handler_base<fiibarbaz>
//    //{
//    //    using signature_handler_base<fiibarbaz>::handle;
//    //};
//
//    //fiibarbaz fbb{};
//    //fbb.handle(type);
//}

wrl::ComPtr<IUnknown> query_interface(IUnknown* unk, GUID const& iid)
{
    wrl::ComPtr<IUnknown> new_interface;
    HRESULT hr = unk->QueryInterface(iid, (void**)new_interface.GetAddressOf());
    if (FAILED(hr)) { throw std::exception{}; }

    return std::move(new_interface);
}

auto get_arg_types(method_signature const& signature)
{
    std::vector<ffi_type*> arg_types{};
    
    // explicit this pointer 
    arg_types.push_back(&ffi_type_pointer); 

    for (auto&& p : signature.params())
    {
        xlang::throw_invalid("not implemented");
    }

    if (signature.return_signature())
    {
        if (signature.return_signature().Type().is_szarray())
        {
            xlang::throw_invalid("not implemented");
        }

        // return values are always pointers
        arg_types.push_back(&ffi_type_pointer);
    }

    return std::move(arg_types);
}

HSTRING invoke(IUnknown* unk, int index, xmd::MethodDef const& method)
{
    ffi_cif cif;
    method_signature signature{ method };
    std::vector<ffi_type*> arg_types = get_arg_types(signature);
    auto ffi_return = ffi_prep_cif(&cif, FFI_STDCALL, arg_types.size(), &ffi_type_sint32, arg_types.data());
    if (ffi_return != FFI_OK) { throw std::exception{}; }

    HRESULT hr;
    std::vector<void*> arg_values{};
    arg_values.push_back(&unk); // explicit this

    // TODO: read return value and params from metadata
    HSTRING returnvalue{};
    auto foo = &returnvalue;
    arg_values.push_back(&foo);

    auto vtbl = *((void***)unk);
    ffi_call(&cif, FFI_FN(vtbl[6]), &hr, arg_values.data());
    if (FAILED(hr)) { throw std::exception{}; }

    return returnvalue;
}

int main(int const /*argc*/, char** /*argv*/)
{
    auto start = get_start_time();

    xmd::cache c{ get_system_metadata() };
    auto namespaces = get_namespace_map(c);

    auto elapsed = get_elapsed_time(start);

    printf("%lldms\n", elapsed);

    auto istringable_typedef = get_ns(namespaces, "Windows.Foundation").types.at("IStringable");
    auto tostring_methoddef = istringable_typedef.MethodList().first;


    wrl::RoInitializeWrapper ro_init(RO_INIT_MULTITHREADED);
    if (FAILED(ro_init)) { throw std::exception{ "roinit failed" }; }

    wrl::ComPtr<IActivationFactory> factory;
    if (FAILED(Windows::Foundation::GetActivationFactory(wrl::HStringReference{ L"Windows.Data.Json.JsonObject" }.Get(), &factory))) { throw std::exception{}; }

    auto call_to_string = [&istringable_typedef, &tostring_methoddef](IUnknown* insp)
    {
        wrl::ComPtr<IUnknown> istringable_ptr = query_interface(insp, get_guid(istringable_typedef));

        return invoke(istringable_ptr.Get(), 6, tostring_methoddef);
        //ffi_cif cif;
        //method_signature signature{ tostring_methoddef };
        //std::vector<ffi_type*> arg_types = get_arg_types(signature);
        //auto ffi_return = ffi_prep_cif(&cif, FFI_STDCALL, arg_types.size(), &ffi_type_sint32, arg_types.data());
        //if (ffi_return != FFI_OK) { throw std::exception{}; }

        //HRESULT hr;
        //wrl::HString result;

        //auto arg0 = istringable_ptr.Get();
        //auto arg1 = result.GetAddressOf();

        //std::vector<void*> arg_values{};
        //arg_values.push_back(&arg0);
        //arg_values.push_back(&arg1);

        //auto vtbl = *((void***)istringable_ptr.Get());
        //ffi_call(&cif, FFI_FN(vtbl[6]), &hr, arg_values.data());
        //if (FAILED(hr)) { throw std::exception{}; }

        //return result;
    };

    {
        wrl::ComPtr<IInspectable> insp;
        HRESULT hr = factory->ActivateInstance(insp.GetAddressOf());
        if (FAILED(hr)) { throw std::exception{}; }

        auto str = call_to_string(insp.Get());
    }

    {
        auto factory_vtbl = *((void***)factory.Get());
        auto activate_intance_method = (HRESULT(__stdcall *)(void*, IInspectable**))factory_vtbl[6];

        wrl::ComPtr<IInspectable> insp;
        HRESULT hr = (*activate_intance_method)(factory.Get(), insp.GetAddressOf());
        if (FAILED(hr)) { throw std::exception{}; }

        auto str = call_to_string(insp.Get());
    }

    {
        ffi_cif cif;
        ffi_type* arg_types[]{ &ffi_type_pointer, &ffi_type_pointer };
        auto ffi_return = ffi_prep_cif(&cif, FFI_STDCALL, 2, &ffi_type_sint32, arg_types);
        if (ffi_return != FFI_OK) { throw std::exception{}; }

        HRESULT hr;
        wrl::ComPtr<IInspectable> insp;

        auto arg0 = factory.Get();
        auto arg1 = insp.GetAddressOf();
        void* arg_values[]{ &arg0, &arg1 };

        auto factory_vtbl = *((void***)factory.Get());
        ffi_call(&cif, FFI_FN(factory_vtbl[6]), &hr, arg_values);
        if (FAILED(hr)) { throw std::exception{}; }

        auto str = call_to_string(insp.Get());
    }

    return 0;
}