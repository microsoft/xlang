#include "pch.h"
#include "base.h"
#include "helpers.h"

#include "ffi.h"

namespace meta = xlang::meta::reader;

using namespace fooxlang;

struct winrt_ns
{
    std::map<std::string_view, winrt_ns> sub_namespaces{};
    meta::cache::namespace_members members{};
};

meta::cache::namespace_members const* find_ns(std::map<std::string_view, winrt_ns> const& namespaces, std::string_view const& ns) noexcept
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

meta::cache::namespace_members const& get_ns(std::map<std::string_view, winrt_ns> const& namespaces, std::string_view const& ns)
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

auto get_namespace_map(meta::cache const& c)
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

auto get_guid(meta::CustomAttribute const& attrib)
{
    std::vector<meta::FixedArgSig> z = attrib.Value().FixedArgs();

    auto getarg = [&z](std::size_t index) { return std::get<meta::ElemSig>(z[index].value).value; };
    return winrt::guid{
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

auto get_guid(meta::TypeDef const& type)
{
    return get_guid(meta::get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute"));
}

int main(int const /*argc*/, char** /*argv*/)
{
    //auto start = get_start_time();

    meta::cache c{ get_system_metadata() };
    auto namespaces = get_namespace_map(c);

    //auto elapsed = get_elapsed_time(start);

    //printf("%lldms\n", elapsed);

    auto istringable_typedef = get_ns(namespaces, "Windows.Foundation").types.at("IStringable");
    auto tostring_methoddef = istringable_typedef.MethodList().first;

    winrt::init_apartment();
    auto factory = winrt::get_activation_factory(L"Windows.Data.Json.JsonObject");

    ffi_cif cif;
    ffi_type* arg_types[]{ &ffi_type_pointer, &ffi_type_pointer };
    auto ffi_return = ffi_prep_cif(&cif, FFI_STDCALL, 2, &ffi_type_sint32, arg_types);
    if (ffi_return != FFI_OK) { throw std::exception{}; }

    auto invoke_activate_instance = [&cif](winrt::Windows::Foundation::IUnknown const& factory)
    {
        winrt::hresult hr;
        winrt::com_ptr<winrt::Windows::Foundation::IInspectable> instance;

        auto arg0 = winrt::get_abi(factory);
        auto arg1 = winrt::put_abi(instance);
        void* arg_values[]{ &arg0, &arg1 };

        ffi_call(&cif, FFI_FN((*((void***)arg0))[6]), &hr, arg_values);
        winrt::check_hresult(hr);

        return std::move(instance);
    };

    auto invoke_to_string = [&cif, &istringable_typedef](winrt::com_ptr<winrt::Windows::Foundation::IInspectable> const& instance)
    {
        winrt::com_ptr<winrt::Windows::Foundation::IInspectable> istringable;
        winrt::check_hresult(instance.as(get_guid(istringable_typedef), winrt::put_abi(istringable)));

        winrt::hresult hr;
        winrt::hstring str;

        auto arg0 = winrt::get_abi(istringable);
        auto arg1 = winrt::put_abi(str);
        void* arg_values[]{ &arg0, &arg1 };

        ffi_call(&cif, FFI_FN((*((void***)arg0))[6]), &hr, arg_values);
        winrt::check_hresult(hr);

        return std::move(str);
    };

    auto instance = invoke_activate_instance(factory);
    auto str = invoke_to_string(instance);

    return 0;
}