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

        std::string_view current_namespace{};
        std::set<std::string> needed_namespaces{};

#pragma region generic param handling
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
#pragma endregion

        int32_t indent{ 0 };

        struct indent_guard
        {
            explicit indent_guard(writer& w) noexcept : _writer(w)
            {
                _writer.indent++;
            }

            ~indent_guard()
            {
                _writer.indent--;
            }

        private:
            writer & _writer;
        };

        template <typename... Args>
        void write_indented(std::string_view const& value, Args const&... args)
        {
            if (indent == 0)
            {
                write(value, args...);
            }
            else
            {
                auto indentation = std::string(indent * 4, ' ');
                if (back() == '\n')
                {
                    write(indentation);
                }

                auto pos = value.find_last_of('\n', value.size() - 2);
                if (pos == std::string_view::npos)
                {
                    write(value, args...);
                }
                else
                {
                    std::string value_{ value };

                    while (pos != std::string::npos)
                    {
                        value_.insert(pos + 1, indentation);
                        if (pos == 0)
                        {
                            break;
                        }
                        pos = value_.find_last_of('\n', pos - 1);
                    }

                    write(value_, args...);
                }
            }
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

        void write(TypeRef const& type)
        {
            auto ns = type.TypeNamespace();
            auto name = type.TypeName();

            if (name == "Guid" && ns == "System")
            {
                write("winrt::guid");
            }
            else
            {
                write(find_required(type));
            }
        }

        void write(TypeDef const& type)
        {
            auto ns = type.TypeNamespace();
            auto name = type.TypeName();

            if (ns != current_namespace)
            {
                needed_namespaces.emplace(ns);
            }

            if (ns == "Windows.Foundation.Numerics")
            {
                if (name == "Matrix3x2") { name = "float3x2"; }
                else if (name == "Matrix4x4") { name = "float4x4"; }
                else if (name == "Plane") { name = "plane"; }
                else if (name == "Quaternion") { name = "quaternion"; }
                else if (name == "Vector2") { name = "float2"; }
                else if (name == "Vector3") { name = "float3"; }
                else if (name == "Vector4") { name = "float4"; }
            }

            write("winrt::@::@", ns, name);
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
            write("%<%>", type.GenericType(), bind_list(", ", type.GenericArgs()));
        }

        void write(ElementType type)
        {
            switch (type)
            {
            case ElementType::Boolean:
                write("bool");
                break;
            case ElementType::Char:
                write("wchar_t");
                break;
            case ElementType::I1:
                write("int8_t");
                break;
            case ElementType::U1:
                write("uint8_t");
                break;
            case ElementType::I2:
                write("int16_t");
                break;
            case ElementType::U2:
                write("uint16_t");
                break;
            case ElementType::I4:
                write("int32_t");
                break;
            case ElementType::U4:
                write("uint32_t");
                break;
            case ElementType::I8:
                write("int64_t");
                break;
            case ElementType::U8:
                write("uint64_t");
                break;
            case ElementType::R4:
                write("float");
                break;
            case ElementType::R8:
                write("double");
                break;
            case ElementType::String:
                write("winrt::hstring");
                break;
            case ElementType::Object:
                write("winrt::Windows::Foundation::IInspectable");
                break;
            default:
                throw_invalid("write_method_comment_type element type not impl");
            }
        }

        void write(GenericTypeIndex var)
        {
            write(generic_param_stack.back()[var.index]);
        }

        void write(TypeSig const& signature)
        {
            visit(signature.Type(),
                [&](auto&& type)
            {
                write(type);
            });
        }

        void write_license()
        {
            write("// WARNING: Please don't edit this file. It was generated by Python/WinRT\n\n");
        }
    };
}