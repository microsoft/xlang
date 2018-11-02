#pragma once

namespace xlang
{
    using namespace std::experimental::filesystem;
    using namespace text;
    using namespace meta::reader;

    template <typename First, typename...Rest>
    auto get_impl_name(First const& first, Rest const&... rest)
    {
        std::string result;

        auto convert = [&](auto&& value)
        {
            for (auto&& c : value)
            {
                result += c == '.' ? '_' : c;
            }
        };

        convert(first);
        ((result += '_', convert(rest)), ...);
        return result;
    }

    struct writer : writer_base<writer>
    {
        using writer_base<writer>::write;

        std::string type_namespace;
        bool abi_types{};
        bool param_names{};
        bool consume_types{};
        bool async_types{};
        std::map<std::string_view, std::set<TypeDef>> depends;
        std::vector<std::vector<std::string>> generic_param_stack;

        struct generic_param_guard
        {
            explicit generic_param_guard(writer* arg = nullptr)
                : owner(arg)
            {}

            ~generic_param_guard()
            {
                if (owner)
                {
                    owner->generic_param_stack.pop_back();
                }
            }

            generic_param_guard(generic_param_guard && other)
                : owner(other.owner)
            {
                owner = nullptr;
            }

            generic_param_guard& operator=(generic_param_guard&& other)
            {
                owner = std::exchange(other.owner, nullptr);
                return *this;
            }

            generic_param_guard& operator=(generic_param_guard const&) = delete;
            writer* owner;
        };

        void add_depends(TypeDef const& type)
        {
            auto ns = type.TypeNamespace();

            if (ns != type_namespace)
            {
                depends[ns].insert(type);
            }
        }

        [[nodiscard]] auto push_generic_params(std::pair<GenericParam, GenericParam>&& params)
        {
            if (empty(params))
            {
                return generic_param_guard{ nullptr };
            }

            std::vector<std::string> names;

            for (auto&& param : params)
            {
                names.push_back(std::string{ param.Name() });
            }

            generic_param_stack.push_back(std::move(names));
            return generic_param_guard{ this };
        }

        [[nodiscard]] auto push_generic_params(GenericTypeInstSig const& signature)
        {
            std::vector<std::string> names;

            for (auto&& arg : signature.GenericArgs())
            {
                names.push_back(write_temp("%", arg));
            }

            generic_param_stack.push_back(std::move(names));
            return generic_param_guard{ this };
        }

        void write_value(bool value)
        {
            write(value ? "TRUE"sv : "FALSE"sv);
        }

        void write_value(char16_t value)
        {
            write_printf("%#0hx", value);
        }

        void write_value(int8_t value)
        {
            write_printf("%hhd", value);
        }

        void write_value(uint8_t value)
        {
            write_printf("%#0hhx", value);
        }

        void write_value(int16_t value)
        {
            write_printf("%hd", value);
        }

        void write_value(uint16_t value)
        {
            write_printf("%#0hx", value);
        }

        void write_value(int32_t value)
        {
            write_printf("%d", value);
        }

        void write_value(uint32_t value)
        {
            write_printf("%#0x", value);
        }

        void write_value(int64_t value)
        {
            write_printf("%lld", value);
        }

        void write_value(uint64_t value)
        {
            write_printf("%#0llx", value);
        }

        void write_value(float value)
        {
            write_printf("%f", value);
        }

        void write_value(double value)
        {
            write_printf("%f", value);
        }

        void write_value(std::string_view value)
        {
            write("\"%\"", value);
        }

        void write_code(std::string_view const& value)
        {
            for (auto&& c : value)
            {
                if (c == '.')
                {
                    write("::");
                }
                else if (c == '`')
                {
                    return;
                }
                else
                {
                    write(c);
                }
            }
        }

        void write(Constant const& value)
        {
            switch (value.Type())
            {
            case ConstantType::Boolean:
                write_value(value.ValueBoolean());
                break;
            case ConstantType::Char:
                write_value(value.ValueChar());
                break;
            case ConstantType::Int8:
                write_value(value.ValueInt8());
                break;
            case ConstantType::UInt8:
                write_value(value.ValueUInt8());
                break;
            case ConstantType::Int16:
                write_value(value.ValueInt16());
                break;
            case ConstantType::UInt16:
                write_value(value.ValueUInt16());
                break;
            case ConstantType::Int32:
                write_value(value.ValueInt32());
                break;
            case ConstantType::UInt32:
                write_value(value.ValueUInt32());
                break;
            case ConstantType::Int64:
                write_value(value.ValueInt64());
                break;
            case ConstantType::UInt64:
                write_value(value.ValueUInt64());
                break;
            case ConstantType::Float32:
                write_value(value.ValueFloat32());
                break;
            case ConstantType::Float64:
                write_value(value.ValueFloat64());
                break;
            case ConstantType::String:
                write_value(value.ValueString());
                break;
            case ConstantType::Class:
                write("null");
                break;
            }
        }

        void write(TypeDef const& type)
        {
            auto ns = type.TypeNamespace();
            auto name = type.TypeName();
            add_depends(type);

            // TODO: get rid of all these renames once parity with cppwinrt.exe has been reached...

            if (name == "EventRegistrationToken" && ns == "Windows.Foundation")
            {
                write("winrt::event_token");
            }
            else if (name == "HResult" && ns == "Windows.Foundation")
            {
                write("winrt::hresult");
            }
            else if (abi_types)
            {
                auto category = get_category(type);

                if (ns == "Windows.Foundation.Numerics")
                {
                    if (name == "Matrix3x2") { name = "float3x2"; }
                    else if (name == "Matrix4x4") { name = "float4x4"; }
                    else if (name == "Plane") { name = "plane"; }
                    else if (name == "Quaternion") { name = "quaternion"; }
                    else if (name == "Vector2") { name = "float2"; }
                    else if (name == "Vector3") { name = "float3"; }
                    else if (name == "Vector4") { name = "float4"; }

                    write("@::%", ns, name);
                }
                else if (category == category::struct_type)
                {
                    if ((name == "DateTime" || name == "TimeSpan") && ns == "Windows.Foundation")
                    {
                        write("int64_t");
                    }
                    else if ((name == "Point" || name == "Size" || name == "Rect") && ns == "Windows.Foundation")
                    {
                        write("@::%", ns, name);
                    }
                    else
                    {
                        write("struct struct_%_%", get_impl_name(ns), name);
                    }
                }
                else if (category == category::enum_type)
                {
                    write(type.FieldList().first.Signature().Type());
                }
                else
                {
                    write("void*");
                }
            }
            else
            {
                if (ns == "Windows.Foundation.Numerics")
                {
                    if (name == "Matrix3x2") { name = "float3x2"; }
                    else if (name == "Matrix4x4") { name = "float4x4"; }
                    else if (name == "Plane") { name = "plane"; }
                    else if (name == "Quaternion") { name = "quaternion"; }
                    else if (name == "Vector2") { name = "float2"; }
                    else if (name == "Vector3") { name = "float3"; }
                    else if (name == "Vector4") { name = "float4"; }

                    write("@::%", ns, name);
                }
                else
                {
                    write("@::%", ns, name);
                }
            }
        }

        void write(TypeRef const& type)
        {
            if (type.TypeName() == "Guid" && type.TypeNamespace() == "System")
            {
                write("winrt::guid");
            }
            else
            {
                write(find_required(type));
            }
        }

        void write(coded_index<TypeDefOrRef> const& type)
        {
            switch (type.type())
            {
            case TypeDefOrRef::TypeDef:

                write(type.TypeDef());
                break;

            case TypeDefOrRef::TypeRef:
                write(type.TypeRef());
                break;

            case TypeDefOrRef::TypeSpec:
                write(type.TypeSpec().Signature().GenericTypeInst());
                break;
            }
        }

        void write(GenericTypeInstSig const& type)
        {
            if (abi_types)
            {
                write("void*");
            }
            else
            {
                auto generic_type = type.GenericType().TypeRef();
                auto ns = generic_type.TypeNamespace();
                auto name = generic_type.TypeName();
                name.remove_suffix(name.size() - name.rfind('`'));

                if (consume_types)
                {
                    static constexpr std::string_view optional("Windows::Foundation::IReference"sv);
                    static constexpr std::string_view iterable("Windows::Foundation::Collections::IIterable"sv);
                    static constexpr std::string_view vector_view("Windows::Foundation::Collections::IVectorView"sv);
                    static constexpr std::string_view map_view("Windows::Foundation::Collections::IMapView"sv);
                    static constexpr std::string_view vector("Windows::Foundation::Collections::IVector"sv);
                    static constexpr std::string_view map("Windows::Foundation::Collections::IMap"sv);

                    consume_types = false;
                    auto full_name = write_temp("@::%<%>", ns, name, bind_list(", ", type.GenericArgs()));
                    consume_types = true;

                    if (starts_with(full_name, optional))
                    {
                        write("optional%", full_name.substr(optional.size()));
                    }
                    else if (starts_with(full_name, iterable))
                    {
                        if (async_types)
                        {
                            write("param::async_iterable%", full_name.substr(iterable.size()));
                        }
                        else
                        {
                            write("param::iterable%", full_name.substr(iterable.size()));
                        }
                    }
                    else if (starts_with(full_name, vector_view))
                    {
                        if (async_types)
                        {
                            write("param::async_vector_view%", full_name.substr(vector_view.size()));
                        }
                        else
                        {
                            write("param::vector_view%", full_name.substr(vector_view.size()));
                        }
                    }

                    else if (starts_with(full_name, map_view))
                    {
                        if (async_types)
                        {
                            write("param::async_map_view%", full_name.substr(map_view.size()));
                        }
                        else
                        {
                            write("param::map_view%", full_name.substr(map_view.size()));
                        }
                    }
                    else if (starts_with(full_name, vector))
                    {
                        write("param::vector%", full_name.substr(vector.size()));
                    }
                    else if (starts_with(full_name, map))
                    {
                        write("param::map%", full_name.substr(map.size()));
                    }
                    else
                    {
                        write(full_name);
                    }
                }
                else
                {
                    write("@::%<%>", ns, name, bind_list(", ", type.GenericArgs()));
                }
            }
        }

        void write(TypeSig::value_type const& type)
        {
            xlang::visit(type,
                [&](ElementType type)
            {
                if (type == ElementType::Boolean) { write("bool"); }
                else if (type == ElementType::Char) { write("char16_t"); }
                else if (type == ElementType::I1) { write("int8_t"); }
                else if (type == ElementType::U1) { write("uint8_t"); }
                else if (type == ElementType::I2) { write("int16_t"); }
                else if (type == ElementType::U2) { write("uint16_t"); }
                else if (type == ElementType::I4) { write("int32_t"); }
                else if (type == ElementType::U4) { write("uint32_t"); }
                else if (type == ElementType::I8) { write("int64_t"); }
                else if (type == ElementType::U8) { write("uint64_t"); }
                else if (type == ElementType::R4) { write("float"); }
                else if (type == ElementType::R8) { write("double"); }
                else if (type == ElementType::String)
                {
                    if (abi_types)
                    {
                        write("void*");
                    }
                    else if (consume_types)
                    {
                        write("param::hstring");
                    }
                    else
                    {
                        write("hstring");
                    }
                }
                else if (type == ElementType::Object)
                {
                    if (abi_types)
                    {
                        write("void*");
                    }
                    else
                    {
                        write("Windows::Foundation::IInspectable");
                    }
                }
                else
                {
                    XLANG_ASSERT(false);
                }
            },
                [&](GenericTypeIndex var)
            {
                write(generic_param_stack.back()[var.index]);
            },
                [&](auto&& type)
            {
                write(type);
            });
        }

        void write(TypeSig const& signature)
        {
            if (!abi_types && signature.is_szarray())
            {
                write("com_array<%>", signature.Type());
            }
            else
            {
                write(signature.Type());
            }
        }

        void write(RetTypeSig const& value)
        {
            if (value)
            {
                write(value.Type());
            }
            else
            {
                write("void");
            }
        }

        void write(Field const& value)
        {
            write(value.Signature().Type());
        }

        void write_depends(std::string_view const& ns, char impl = 0)
        {
            if (impl)
            {
                auto format = R"(#include "%/impl/%.%.h"
)";

                write(format, settings.root, ns, impl);
            }
            else
            {
                auto format = R"(#include "%/%.h"
)";

                write(format, settings.root, ns);
            }
        }

        void write_parent_depends(cache const& c)
        {
            auto pos = type_namespace.rfind('.');

            if (pos == std::string::npos)
            {
                return;
            }

            auto parent = type_namespace.substr(0, pos);

            if (c.namespaces().find(parent) == c.namespaces().end())
            {
                return;
            }

            write("#include \"%/%.h\"\n", settings.root, parent);
        }

        void save_header(char impl = 0)
        {
            auto filename{ settings.output_folder + settings.root + "/" };

            if (impl)
            {
                filename += "impl/";
            }

            filename += type_namespace;

            if (impl)
            {
                filename += '.';
                filename += impl;
            }

            filename += ".h";
            flush_to_file(filename);
        }
    };
}
