#include "pch.h"
#include "base.h"
#include "helpers.h"

#include "ffi.h"
#include "sha1.h"

#include <memory>
#include <iostream>
#include <sstream>

namespace meta = xlang::meta::reader;
using IInspectable = winrt::Windows::Foundation::IInspectable;

using namespace fooxlang;

struct writer : xlang::text::writer_base<writer>
{
    using writer_base<writer>::write;

    void write(winrt::hstring const& str)
    {
        write(winrt::to_string(str));
    }

    void write(meta::TypeDef const& type)
    {
        type_name_handler tnh{};
        tnh.handle(type);
        write(tnh.name);
    }

    void write(meta::coded_index<meta::TypeDefOrRef> const& type)
    {
        type_name_handler tnh{};
        tnh.handle(type);
        write(tnh.name);
    }

private:
    struct type_name_handler : signature_handler_base<type_name_handler>
    {
        using signature_handler_base<type_name_handler>::handle;
        std::string name{};

        void insert(std::string_view const& value)
        {
            name.insert(name.end(), value.begin(), value.end());
        }

        void insert(char const value)
        {
            name.push_back(value);
        }

        void handle(meta::TypeDef const& type)
        {
            insert(type.TypeNamespace());
            insert('.');
            insert(type.TypeName());
        }

        void handle(meta::ElementType type)
        {
            switch (type)
            {
            case meta::ElementType::Boolean:
                insert("Boolean"); break;
            case meta::ElementType::Char:
                insert("Char"); break;
            case meta::ElementType::I1:
                insert("I1"); break;
            case meta::ElementType::U1:
                insert("U1"); break;
            case meta::ElementType::I2:
                insert("I2"); break;
            case meta::ElementType::U2:
                insert("U2"); break;
            case meta::ElementType::I4:
                insert("I4"); break;
            case meta::ElementType::U4:
                insert("U4"); break;
            case meta::ElementType::I8:
                insert("I8"); break;
            case meta::ElementType::U8:
                insert("U8"); break;
            case meta::ElementType::R4:
                insert("R4"); break;
            case meta::ElementType::R8:
                insert("R8"); break;
            case meta::ElementType::String:
                insert("String"); break;
            case meta::ElementType::Object:
                insert("Object"); break;
            default:
                xlang::throw_invalid("element type not supported");
            }
        }

        void handle(meta::GenericTypeInstSig const& type)
        {
            handle(type.GenericType());

            insert('<');
            bool first{ true };
            for (auto&& arg : type.GenericArgs())
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    insert(", ");
                }
                handle(arg);
            }
            insert('>');
        }
    };

};

auto get_TypeDef(meta::coded_index<meta::TypeDefOrRef> const& type)
{
    switch (type.type())
    {
    case meta::TypeDefOrRef::TypeDef:
        return type.TypeDef();
    case meta::TypeDefOrRef::TypeRef:
        return find_required(type.TypeRef());
    case meta::TypeDefOrRef::TypeSpec:
        return find_required(type.TypeSpec().Signature().GenericTypeInst().GenericType().TypeRef());
    }

    xlang::throw_invalid("invalid TypeDefOrRef");
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

auto get_guid_attrib(meta::TypeDef const& type)
{
    meta::CustomAttribute const& attrib = meta::get_attribute(type, "Windows.Foundation.Metadata", "GuidAttribute");

    if (!attrib)
    {
        xlang::throw_invalid("missing guid");
    }

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

struct ptype_guid_calculator : signature_handler_base<ptype_guid_calculator>
{
    using signature_handler_base<ptype_guid_calculator>::handle;
    std::string name{};

    void insert(std::string_view const& value)
    {
        name.insert(name.end(), value.begin(), value.end());
    }

    void insert(char const value)
    {
        name.push_back(value);
    }

    template <typename... Args>
    void insert(char const* format, Args const&... args)
    {
        char buffer[128];
#if XLANG_PLATFORM_WINDOWS
        size_t const size = sprintf_s(buffer, format, args...);
#else
        size_t const size = snprintf(buffer, sizeof(buffer), format, args...);
#endif
        insert(std::string_view{ buffer, size });
    }

    void insert(winrt::guid const& value)
    {
        insert("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
            value.Data1,
            value.Data2,
            value.Data3,
            value.Data4[0],
            value.Data4[1],
            value.Data4[2],
            value.Data4[3],
            value.Data4[4],
            value.Data4[5],
            value.Data4[6],
            value.Data4[7]);
    }

    //void handle_class(xlang::meta::reader::TypeDef const& /*type*/) { xlang::throw_invalid("handle_class not implemented"); }
    //void handle_delegate(xlang::meta::reader::TypeDef const& /*type*/) { xlang::throw_invalid("handle_delegate not implemented"); }
    
    void handle_guid(xlang::meta::reader::TypeRef const& /*type*/) 
    { 
        insert("g16");
    }

    void handle_interface(xlang::meta::reader::TypeDef const& type) 
    { 
        insert(get_guid_attrib(type));
    }

    //void handle_struct(xlang::meta::reader::TypeDef const& /*type*/) { xlang::throw_invalid("handle_struct not implemented"); }

    void handle(meta::ElementType type)
    {
        switch (type)
        {
        case meta::ElementType::Boolean:
            insert("b1"); break;
        case meta::ElementType::Char:
            insert("c2"); break;
        case meta::ElementType::I1:
            insert("i1"); break;
        case meta::ElementType::U1:
            insert("u1"); break;
        case meta::ElementType::I2:
            insert("i2"); break;
        case meta::ElementType::U2:
            insert("u2"); break;
        case meta::ElementType::I4:
            insert("i4"); break;
        case meta::ElementType::U4:
            insert("u4"); break;
        case meta::ElementType::I8:
            insert("i8"); break;
        case meta::ElementType::U8:
            insert("u8"); break;
        case meta::ElementType::R4:
            insert("f4"); break;
        case meta::ElementType::R8:
            insert("f8"); break;
        case meta::ElementType::String:
            insert("string"); break;
        case meta::ElementType::Object:
            insert("cinterface(IInspectable)"); break;
        default:
            xlang::throw_invalid("element type not supported");
        }
    }

    void handle(meta::GenericTypeInstSig const& type)
    {
        auto get_piid = [](meta::coded_index<meta::TypeDefOrRef> type)
        {
            switch (type.type())
            {
            case meta::TypeDefOrRef::TypeRef:
                return get_guid_attrib(meta::find_required(type.TypeRef()));
            case meta::TypeDefOrRef::TypeDef:
                return get_guid_attrib(type.TypeDef());
            }

            xlang::throw_invalid("Invalid TypeDefOrRef");
        };

        insert("pinterface(");
        insert(get_piid(type.GenericType()));
        insert(';');

        bool first{ true };
        for (auto&& arg : type.GenericArgs())
        {
            if (first)
            {
                first = false;
            }
            else
            {
                insert(";");
            }
            handle(arg);
        }
        insert(')');
    }
};

    //template <size_t Size>
    //constexpr guid generate_guid(std::array<char, Size> const& value) noexcept
    //{
    //    guid namespace_guid = { 0xd57af411, 0x737b, 0xc042,{ 0xab, 0xae, 0x87, 0x8b, 0x1e, 0x16, 0xad, 0xee } };

    //    auto buffer = combine(to_array(namespace_guid), char_to_byte_array(value, std::make_index_sequence<Size>()));
    //    auto hash = calculate_sha1(buffer);
    //    auto big_endian_guid = to_guid(hash);
    //    auto little_endian_guid = endian_swap(big_endian_guid);
    //    return set_named_guid_fields(little_endian_guid);
    //}

std::stringbuf to_vector(winrt::guid const& value)
{
    std::stringbuf baz{};

    baz.sputc(static_cast<char>(value.Data1 & 0x000000ff));
    baz.sputc(static_cast<char>((value.Data1 & 0x0000ff00) >> 8));
    baz.sputc(static_cast<char>((value.Data1 & 0x00ff0000) >> 16));
    baz.sputc(static_cast<char>((value.Data1 & 0xff000000) >> 24));
    baz.sputc(static_cast<char>(value.Data2 & 0x00ff));
    baz.sputc(static_cast<char>((value.Data2 & 0xff00) >> 8));
    baz.sputc(static_cast<char>(value.Data3 & 0x00ff));
    baz.sputc(static_cast<char>((value.Data3 & 0xff00) >> 8));
    baz.sputc(static_cast<char>(value.Data4[0]));
    baz.sputc(static_cast<char>(value.Data4[1]));
    baz.sputc(static_cast<char>(value.Data4[2]));
    baz.sputc(static_cast<char>(value.Data4[3]));
    baz.sputc(static_cast<char>(value.Data4[4]));
    baz.sputc(static_cast<char>(value.Data4[5]));
    baz.sputc(static_cast<char>(value.Data4[6]));
    baz.sputc(static_cast<char>(value.Data4[7]));

    return std::move(baz);
}

auto calculate_guid(meta::TypeSpec const& type)
{
    ptype_guid_calculator guid_calc{};
    guid_calc.handle(type.Signature().GenericTypeInst());

    winrt::guid namespace_guid = { 0xd57af411, 0x737b, 0xc042,{ 0xab, 0xae, 0x87, 0x8b, 0x1e, 0x16, 0xad, 0xee } };
    auto buffer = to_vector(namespace_guid);
    std::istream istrm(&buffer);

    SHA1 checksum;
    checksum.update(istrm);
    checksum.update(guid_calc.name);
    auto hash = checksum.final();


    //std::iostri ios{};

    
    
    //for (auto c : guid_calc.name)
    //{
    //    buffer.push_back(static_cast<uint8_t>(c));
    //}

    //auto hash = winrt::impl::calculate_sha1()
    //auto big_endian_guid = to_guid(hash);
    //auto little_endian_guid = endian_swap(big_endian_guid);

    return namespace_guid;
}

auto get_guid(meta::coded_index<meta::TypeDefOrRef> const& type)
{
    switch (type.type())
    {
    case meta::TypeDefOrRef::TypeDef:
        return get_guid_attrib(type.TypeDef());
    case meta::TypeDefOrRef::TypeRef:
        return get_guid_attrib(find_required(type.TypeRef()));
    case meta::TypeDefOrRef::TypeSpec:
        return calculate_guid(type.TypeSpec());
    }

    xlang::throw_invalid("invalid TypeDefOrRef");

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

auto find_interface_method(meta::TypeDef const& type, std::string_view const& method_name)
{
    XLANG_ASSERT(meta::get_category(type) == meta::category::interface_type);

    for (auto&& method : type.MethodList())
    {
        if (method.Name() == method_name)
        {
            return method;
        }
    }

    return meta::MethodDef{};
}

auto find_class_method(meta::TypeDef const& type, std::string_view const& method_name)
{
    XLANG_ASSERT(meta::get_category(type) == meta::category::class_type);

    for (auto&& ii : type.InterfaceImpl())
    {
        auto interface_type = get_TypeDef(ii.Interface());

        auto method = find_interface_method(interface_type, method_name);

        if (method)
        {
            return std::make_tuple(ii.Interface(), method);
        }
    }

    return std::make_tuple(meta::coded_index<meta::TypeDefOrRef>{}, meta::MethodDef{});
}

void interface_invoke(meta::TypeDef const& type, std::string_view const& method_name, IInspectable const& instance, std::vector<void*> const& parameters)
{
    meta::MethodDef method = find_interface_method(type, method_name);

    if (!method)
    {
        xlang::throw_invalid("method not found");
    }

    auto index = method.index() - type.MethodList().first.index();

    IInspectable interface_instance;
    winrt::check_hresult(instance.as(get_guid_attrib(type), winrt::put_abi(interface_instance)));

    auto arg_types = get_method_ffi_types(method);

    invoke(get_cif(arg_types), interface_instance, 6 + index, parameters);
}

void class_invoke(meta::TypeDef const& type, std::string_view method_name, IInspectable const& instance, std::vector<void*> const& parameters)
{
    auto[interface_type, method] = find_class_method(type, method_name);

    if (!interface_type || !method)
    {
        xlang::throw_invalid("method not found");
    }

    auto g = get_guid(interface_type);

    auto index = method.index() - get_TypeDef(interface_type).MethodList().first.index();

    writer w;
    w.write("%::%", interface_type, method.Name());
    w.write_printf("(index: %d)\n", index);
    w.flush_to_console();
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

        auto Append = [&td_JsonArray](IInspectable const& instance, IInspectable const& value)
        {
            std::vector<void*> args{ winrt::get_abi(value) };

            class_invoke(td_JsonArray, "Append", instance, args);
        };        
        
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

        writer w;
        w.write("\n\n");

        w.write_printf("null_value  JsonValueType: %d\n", get_ValueType(null_value));
        w.write_printf("json_array  JsonValueType: %d\n", get_ValueType(json_array));
        w.write_printf("json_object JsonValueType: %d\n", get_ValueType(json_object));

        w.write("null_value  ToString:  %\n", ToString(null_value));
        w.write("json_array  ToString:  %\n", ToString(json_array));
        w.write("json_object ToString:  %\n", ToString(json_object));

        Append(json_array, null_value);
 
        for (auto[key, knight] : knights)
        {
            SetNamedValue(json_object, key, knight);
        }

        SetNamedValue(json_object, L"SirNotAppearingInThisFilm", null_value);

        w.write("json_array  Stringify: %\n", Stringify(json_array));
        w.write("json_object Stringify: %\n", Stringify(json_object));

        w.flush_to_console();
    }
    catch (std::exception const& e)
    {
        printf("%s\n", e.what());
        return -1;
    }

    return 0;
}