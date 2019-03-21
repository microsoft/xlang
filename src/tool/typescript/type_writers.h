#pragma once

namespace xlang
{
    using namespace std::experimental::filesystem;
    using namespace text;
    using namespace meta::reader;

    struct type_name
    {
        std::string_view name;
        std::string_view name_space;

        explicit type_name(TypeDef const& type) :
            name(type.TypeName()),
            name_space(type.TypeNamespace())
        {
        }

        explicit type_name(TypeRef const& type) :
            name(type.TypeName()),
            name_space(type.TypeNamespace())
        {
        }
    };

    bool operator==(type_name const& left, std::string_view const& right)
    {
        if (left.name.size() + 1 + left.name_space.size() != right.size())
        {
            return false;
        }

        if (right[left.name_space.size()] != '.')
        {
            return false;
        }

        if (0 != right.compare(left.name_space.size() + 1, left.name.size(), left.name))
        {
            return false;
        }

        return 0 == right.compare(0, left.name_space.size(), left.name_space);
    }

    static auto remove_tick(std::string_view const& name)
    {
        return name.substr(0, name.rfind('`'));
    }

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

            generic_param_guard(generic_param_guard&& other)
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

        [[nodiscard]] auto push_generic_params(std::pair<GenericParam, GenericParam> const& params)
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

        void write_value(int32_t value)
        {
            write_printf("%d", value);
        }

        void write_value(uint32_t value)
        {
            write_printf("%#0x", value);
        }

        void write_code(std::string_view const& value)
        {
            for (auto&& c : value)
            {
                if (c == '`')
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
            case ConstantType::Int32:
                write_value(value.ValueInt32());
                break;
            case ConstantType::UInt32:
                write_value(value.ValueUInt32());
                break;
            default:
                throw_invalid("Unexpected constant type");
            }
        }

        void write(TypeDef const& type)
        {
            add_depends(type);
            auto ns = type.TypeNamespace();
            auto name = type.TypeName();
            auto generics = type.GenericParam();

            if (!empty(generics))
            {
                write("@.%<%>", ns, remove_tick(name), bind_list(", ", generics));
                return;
            }
            
            write("@.%", ns, name);
        }

        void write(TypeRef const& type)
        {
            if (type_name(type) == "System.Guid")
            {
                write("string /* System.Guid */");
            }
            else
            {
                write(find_required(type));
            }
        }

        void write(GenericParam const& param)
        {
            write(param.Name());
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
            auto generic_type = type.GenericType().TypeRef();
            auto ns = generic_type.TypeNamespace();
            auto name = generic_type.TypeName();
            name.remove_suffix(name.size() - name.rfind('`'));
            add_depends(find_required(generic_type));

            write("@.%<%>", ns, name, bind_list(", ", type.GenericArgs()));
        }

        void write(TypeSig::value_type const& type)
        {
            call(type,
                [&](ElementType type)
                {
                    switch (type)
                    {
                    case ElementType::Boolean:
                        write("boolean");
                        break;
                    case ElementType::I1:
                    case ElementType::U1:
                    case ElementType::I2:
                    case ElementType::U2:
                    case ElementType::I4:
                    case ElementType::U4:
                    case ElementType::I8:
                    case ElementType::U8:
                    case ElementType::R4:
                    case ElementType::R8:
                        write("number");
                        break;
                    case ElementType::String:
                        write("string");
                        break;
                    case ElementType::Object:
                        write("object");
                        break;
                    default:
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
            write(signature.Type());
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

        void save_definition()
        {
            auto filename{ settings.output_folder + "typescript/" };

            filename += type_namespace;

            filename += ".d.ts";

            flush_to_console();
        }
    };
}
