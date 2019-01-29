#include "pch.h"
#include "base.h"
#include "helpers.h"

#include "ffi.h"

#include <memory>

namespace meta = xlang::meta::reader;
using IInspectable = winrt::Windows::Foundation::IInspectable;

using namespace fooxlang;

struct writer : xlang::text::writer_base<writer>
{
};

struct separator
{
    writer& w;
    std::string_view _separator{ ", " };
    bool first{ true };

    void operator()()
    {
        if (first)
        {
            first = false;
        }
        else
        {
            w.write(_separator);
        }
    }
};

auto get_system_metadata()
{
    namespace fs = std::experimental::filesystem;

#ifdef _WIN64
    auto sys32 = "c:\\Windows\\System32";
#else
    auto sys32 = "c:\\Windows\\Sysnative";
#endif

    auto system_winmd_path = fs::path{ sys32 } / "WinMetadata";

    std::vector<std::string> system_winmd_files;
    for (auto& p : fs::directory_iterator(system_winmd_path))
    {
        system_winmd_files.push_back(p.path().string());
    }

    return std::move(system_winmd_files);
}

auto get_guid(meta::TypeDef const& type)
{
    meta::CustomAttribute const& attrib = meta::get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute");

    // TODO: validate attrib

    std::vector<meta::FixedArgSig> args = attrib.Value().FixedArgs();

    auto getarg = [&args](std::size_t index) { return std::get<meta::ElemSig>(args[index].value).value; };

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

std::vector<ffi_type const*> get_method_ffi_types(meta::MethodDef const& method)
{
    method_signature signature{ method };
    std::vector<ffi_type const*> arg_types{ &ffi_type_pointer }; // first param is always a vtable pointer

    for (auto&& param : signature.params())
    {
        if (param.second->Type().is_szarray())
        {
            xlang::throw_invalid("not implemented");
        }

        if (auto e = std::get_if<meta::ElementType>(&(param.second->Type().Type())))
        {
            switch (*e)
            {
            case meta::ElementType::I1:
                arg_types.push_back(&ffi_type_sint8);
                break;
            case meta::ElementType::U1:
                arg_types.push_back(&ffi_type_uint8);
                break;
            case meta::ElementType::I2:
                arg_types.push_back(&ffi_type_sint16);
                break;
            case meta::ElementType::U2:
                arg_types.push_back(&ffi_type_uint16);
                break;
            case meta::ElementType::I4:
                arg_types.push_back(&ffi_type_sint32);
                break;
            case meta::ElementType::U4:
                arg_types.push_back(&ffi_type_uint32);
                break;
            case meta::ElementType::I8:
                arg_types.push_back(&ffi_type_sint64);
                break;
            case meta::ElementType::U8:
                arg_types.push_back(&ffi_type_uint64);
                break;
            case meta::ElementType::R4:
                arg_types.push_back(&ffi_type_float);
                break;
            case meta::ElementType::R8:
                arg_types.push_back(&ffi_type_double);
                break;
            case meta::ElementType::String:
                arg_types.push_back(&ffi_type_pointer);
                break;
            case meta::ElementType::Object:
                arg_types.push_back(&ffi_type_pointer);
                break;
                //case meta::ElementType::Boolean:
                //case meta::ElementType::Char:
            default:
                xlang::throw_invalid("element type not supported");
            }
        }
        else
        {
            arg_types.push_back(&ffi_type_pointer);
        }
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

auto get_activation_factory(meta::TypeDef const& type)
{
    // TODO: need a version of get_activation_factory that takes IID as param
    // TODO: need a factory caching scheme that doesn't use compile time type name

    winrt::hstring type_name = winrt::to_hstring(std::string{ type.TypeNamespace() } +"." + std::string{ type.TypeName() });
    return winrt::get_activation_factory(type_name);
}

// copy of boost::hash_combine, as per https://stackoverflow.com/a/2595226
template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std
{
    // vector hash algorithm, as per https://stackoverflow.com/a/27216842
    template<> struct hash<std::vector<ffi_type const*>>
    {
        std::size_t operator()(std::vector<ffi_type const*> const& vec) const noexcept
        {
            std::size_t seed = vec.size();

            for (auto&& t : vec)
            {
                hash_combine(seed, t);
            }

            return seed;
        }
    };
}

std::unordered_map<std::vector<ffi_type const*>, std::pair<ffi_cif, std::vector<ffi_type const*>>> cif_cache{};

ffi_cif* get_cif(std::vector<ffi_type const*> const& arg_types)
{
    if (cif_cache.find(arg_types) == cif_cache.end())
    {
        // Make a copy of arg_types for cached ffi_cif struct to point to 
        std::vector<ffi_type const*> arg_types_copy{ arg_types.begin(), arg_types.end() };
        auto pair = std::make_pair<ffi_cif, std::vector<ffi_type const*>>(ffi_cif{}, std::move(arg_types_copy));

        auto ffi_return = ffi_prep_cif(&pair.first, FFI_STDCALL, pair.second.size(), &ffi_type_sint32, const_cast<ffi_type**>(pair.second.data()));
        if (ffi_return != FFI_OK) { xlang::throw_invalid("ffi_prep_cif failure"); }

        cif_cache.emplace(arg_types, std::move(pair));
    }

    return &(cif_cache.at(arg_types).first);
}

auto find_method(meta::TypeDef const& type, std::string_view name)
{
    meta::MethodDef method = std::find_if(
        type.MethodList().first, type.MethodList().second,
        [name](meta::MethodDef const& method) {
        return method.Name() == name; });

    auto n = method.Name();

    if (method == type.MethodList().second)
    {
        xlang::throw_invalid("method not found");
    }

    return std::make_tuple(method.index() - type.MethodList().first.index(), method);
}

void invoke(ffi_cif* cif, IInspectable const& instance, int offset, std::vector<void*> const& parameters)
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

void interface_invoke(meta::TypeDef const& type, std::string_view method_name, IInspectable const& instance, std::vector<void*> const& parameters)
{
    XLANG_ASSERT(meta::get_category(type) == meta::category::interface_type);

    auto[index, method_def] = find_method(type, method_name);

    IInspectable interface_instance;
    winrt::check_hresult(instance.as(get_guid(type), winrt::put_abi(interface_instance)));

    auto arg_types = get_method_ffi_types(method_def);

    invoke(get_cif(arg_types), interface_instance, 6 + index, parameters);
}

IInspectable default_activation(IInspectable const& factory)
{
    IInspectable instance;

    std::vector<ffi_type const*> arg_types{ &ffi_type_pointer, &ffi_type_pointer };
    std::vector<void*> args{ winrt::put_abi(instance) };
    invoke(get_cif(arg_types), factory, 6, args);

    return std::move(instance);
}

IInspectable default_activation(meta::TypeDef const& type)
{
    auto factory = get_activation_factory(type);
    return default_activation(factory);
}

void write_type_name(writer& w, meta::coded_index<meta::TypeDefOrRef> const& tdrs)
{
    struct type_name_handler : signature_handler_base<type_name_handler>
    {
        using signature_handler_base<type_name_handler>::handle;
        writer& w;

        type_name_handler(writer& w_) : w(w_) {}

        void handle(meta::TypeDef const& type)
        {
            w.write("%.%", type.TypeNamespace(), type.TypeName());
        }

        void handle(meta::ElementType type)
        {
            switch (type)
            {
            case meta::ElementType::Boolean:
                w.write("Boolean"); break;
            case meta::ElementType::Char:
                w.write("Char"); break;
            case meta::ElementType::I1:
                w.write("I1"); break;
            case meta::ElementType::U1:
                w.write("U1"); break;
            case meta::ElementType::I2:
                w.write("I2"); break;
            case meta::ElementType::U2:
                w.write("U2"); break;
            case meta::ElementType::I4:
                w.write("I4"); break;
            case meta::ElementType::U4:
                w.write("U4"); break;
            case meta::ElementType::I8:
                w.write("I8"); break;
            case meta::ElementType::U8:
                w.write("U8"); break;
            case meta::ElementType::R4:
                w.write("R4"); break;
            case meta::ElementType::R8:
                w.write("R8"); break;
            case meta::ElementType::String:
                w.write("String"); break;
            case meta::ElementType::Object:
                w.write("Object"); break;
            default:
                xlang::throw_invalid("element type not supported");
            }
        }

        void handle(meta::GenericTypeInstSig const& type)
        {
            handle(type.GenericType());

            w.write("<");
            separator s{ w };
            for (auto&& arg : type.GenericArgs())
            {
                s();
                handle(arg);
            }
            w.write(">");
        }
    };

    type_name_handler tnh{ w };
    tnh.handle(tdrs);
}

int main(int const /*argc*/, char** /*argv*/)
{
    try
    {
        meta::cache c{ get_system_metadata() };

        auto ToString = [&c](IInspectable const& instance)
        {
            winrt::hstring str;
            std::vector<void*> args{ winrt::put_abi(str) };

            meta::TypeDef td_istringable = c.find("Windows.Foundation", "IStringable");
            interface_invoke(td_istringable, "ToString", instance, args);

            return std::move(str);
        };

        auto Stringify = [&c](IInspectable const& instance)
        {
            winrt::hstring str;
            std::vector<void*> args{ winrt::put_abi(str) };

            meta::TypeDef td_IJsonValue = c.find("Windows.Data.Json", "IJsonValue");
            interface_invoke(td_IJsonValue, "Stringify", instance, args);

            return std::move(str);
        };

        auto get_ValueType = [&c](IInspectable const& instance)
        {
            int32_t jsonValueType = -1;
            std::vector<void*> args{ &jsonValueType };

            meta::TypeDef td_ijsonvalue = c.find("Windows.Data.Json", "IJsonValue");
            interface_invoke(td_ijsonvalue, "get_ValueType", instance, args);

            return jsonValueType;
        };

        auto SetNamedValue = [&c](IInspectable const& instance, std::wstring_view key, IInspectable const& value)
        {
            winrt::hstring name{ key };
            std::vector<void*> args{ winrt::get_abi(name), winrt::get_abi(value) };

            meta::TypeDef td_IJsonObject = c.find("Windows.Data.Json", "IJsonObject");
            interface_invoke(td_IJsonObject, "SetNamedValue", instance, args);
        };

        meta::TypeDef td_JsonObject = c.find("Windows.Data.Json", "JsonObject");
        meta::TypeDef td_JsonArray = c.find("Windows.Data.Json", "JsonArray");
        meta::TypeDef td_JsonValue = c.find("Windows.Data.Json", "JsonValue");

        winrt::init_apartment();
        auto val_factory = get_activation_factory(td_JsonValue);

        auto CreateStringValue = [&c, &val_factory](winrt::hstring str)
        {
            IInspectable value;
            std::vector<void*> args{ winrt::get_abi(str), winrt::put_abi(value) };

            meta::TypeDef td_IJsonValueStatics = c.find("Windows.Data.Json", "IJsonValueStatics");
            interface_invoke(td_IJsonValueStatics, "CreateStringValue", val_factory, args);

            return std::move(value);
        };
        
        auto CreateNullValue = [&c, &val_factory]()
        {
            IInspectable value;
            std::vector<void*> args{ winrt::put_abi(value) };

            meta::TypeDef td_IJsonValueStatics2 = c.find("Windows.Data.Json", "IJsonValueStatics2");
            interface_invoke(td_IJsonValueStatics2, "CreateNullValue", val_factory, args);

            return std::move(value);
        };

        IInspectable json_object = default_activation(td_JsonObject);
        IInspectable json_array = default_activation(td_JsonArray);

        IInspectable null_value = CreateNullValue();

        std::map<std::wstring_view, IInspectable> knights = {
            { L"arthur", CreateStringValue(L"Arthur, King of the Britons") },
            { L"lancelot", CreateStringValue(L"Sir Lancelot the Brave") },
            { L"robin", CreateStringValue(L"Sir Robin the Not-Quite-So-Brave-as-Sir-Lancelot") },
            { L"bedevere", CreateStringValue(L"Sir Bedevere the Wise") },
            { L"galahad", CreateStringValue(L"Sir Galahad the Pure")} };

        printf("null_value  JsonValueType: %d\n", get_ValueType(null_value));
        printf("json_array  JsonValueType: %d\n", get_ValueType(json_array));
        printf("json_object JsonValueType: %d\n", get_ValueType(json_object));

        printf("null_value  ToString:  %S\n", ToString(null_value).c_str());
        printf("json_array  ToString:  %S\n", ToString(json_array).c_str());
        printf("json_object ToString:  %S\n", ToString(json_object).c_str());

        for (auto[key, knight] : knights)
        {
            SetNamedValue(json_object, key, knight);
        }

        SetNamedValue(json_object, L"SirNotAppearingInThisFilm", null_value);

        printf("json_array  Stringify: %S\n", Stringify(json_array).c_str());
        printf("json_object Stringify: %S\n", Stringify(json_object).c_str());
    }
    catch (std::exception const& e)
    {
        printf("%s\n", e.what());
        return -1;
    }

    return 0;
}