#include "pch.h"
#include "base.h"
#include "helpers.h"

#include "ffi.h"

#include <memory>

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

auto get_arg_types(meta::MethodDef const& method)
{
    method_signature signature{ method };

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

std::vector<ffi_type const*> get_method_ffi_types(meta::MethodDef const& method)
{
    std::vector<ffi_type const*> arg_types{ &ffi_type_pointer };

    method_signature signature{ method };
    if (signature.has_params())
    {
        xlang::throw_invalid("not implemented");
    }

    if (signature.return_signature())
    {
        if (signature.return_signature().Type().is_szarray())
        {
            xlang::throw_invalid("not implemented");
        }

        arg_types.push_back(&ffi_type_pointer);
    }

    return std::move(arg_types);
}

namespace std
{
    template<> struct hash<std::vector<ffi_type const*>>
    {
        std::size_t operator()(std::vector<ffi_type const*> const& vec) const noexcept
        {
            std::hash<ffi_type const*> hasher;
            
            std::size_t seed = vec.size();
            for (auto&& t : vec)
            {
                seed ^= hasher(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            return seed;
        }
    };
}

std::unordered_map<std::vector<ffi_type const*>, std::shared_ptr<ffi_cif>> cif_cache;

ffi_cif* get_cif(std::vector<ffi_type const*> const& arg_types)
{
    if (cif_cache.find(arg_types) == cif_cache.end())
    {
        std::shared_ptr<ffi_cif> new_cif(new ffi_cif);
        auto ffi_return = ffi_prep_cif(new_cif.get(), FFI_STDCALL, arg_types.size(), &ffi_type_sint32, const_cast<ffi_type**>(arg_types.data()));
        if (ffi_return != FFI_OK) { xlang::throw_invalid("ffi_prep_cif failure"); }

        cif_cache.emplace(arg_types, std::move(new_cif));
    }

    return cif_cache.at(arg_types).get();
}

void invoke(ffi_cif* cif, winrt::Windows::Foundation::IInspectable const& instance, int offset, std::vector<void*> const& parameters)
{
    winrt::hresult hr;
    std::vector<void*> args{};

    auto vtable = winrt::get_abi(instance);

    args.push_back(&vtable);
    for (auto&& p : parameters)
    {
        args.push_back(const_cast<void**>(&p));
    }

    ffi_call(cif, FFI_FN((*((void***)vtable))[offset]), &hr, args.data());
    winrt::check_hresult(hr);
}

int main(int const /*argc*/, char** /*argv*/)
{
    //auto start = get_start_time();

    meta::cache c{ get_system_metadata() };
    auto namespaces = get_namespace_map(c);

    //auto elapsed = get_elapsed_time(start);

    //printf("%lldms\n", elapsed);

    meta::TypeDef   td_istringable = get_ns(namespaces, "Windows.Foundation").types.at("IStringable");
    meta::TypeDef   td_ijsonvalue  = get_ns(namespaces, "Windows.Data.Json").types.at("IJsonValue");

    winrt::init_apartment();
    auto factory = winrt::get_activation_factory(L"Windows.Data.Json.JsonObject");

    std::vector<ffi_type const*> arg_types{ &ffi_type_pointer, &ffi_type_pointer };

    winrt::Windows::Foundation::IInspectable instance;
    {
        std::vector<void*> args{ winrt::put_abi(instance) };
        invoke(get_cif(arg_types), factory, 6, args);
    };

    winrt::hstring str;
    {
        winrt::Windows::Foundation::IInspectable istringable;
        winrt::check_hresult(instance.as(get_guid(td_istringable), winrt::put_abi(istringable)));

        auto arg_types2 = get_method_ffi_types(td_istringable.MethodList().first[0]);
        std::vector<void*> args{ winrt::put_abi(str) };
        invoke(get_cif(arg_types2), istringable, 6, args);
    }

    winrt::hstring str2;
    {
        winrt::Windows::Foundation::IInspectable ijsonvalue;
        winrt::check_hresult(instance.as(get_guid(td_ijsonvalue), winrt::put_abi(ijsonvalue)));

        auto arg_types3 = get_method_ffi_types(td_ijsonvalue.MethodList().first[1]);

        std::vector<void*> args2{ winrt::put_abi(str2) };
        invoke(get_cif(arg_types3), ijsonvalue, 7, args2);
    };

    return 0;
}