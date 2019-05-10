#pragma once

namespace coolrt
{
    using namespace std::literals;
    using namespace std::experimental::filesystem;
    using namespace xlang;
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    template <typename T>
    struct indented_writer_base : public writer_base<T>
    {
        struct indent_guard
        {
            explicit indent_guard(indented_writer_base<T>& w, int32_t offset = 1) noexcept : _writer(w), _offset(offset)
            {
                _writer.indent += _offset;
            }

            ~indent_guard()
            {
                _writer.indent -= _offset;
            }

        private:
            indented_writer_base<T>& _writer;
            int32_t _offset;
        };

        void write_indent()
        {
            for (int32_t i = 0; i < indent; i++)
            {
                writer_base<T>::write_impl("    ");
            }
        }

        void write_impl(std::string_view const& value)
        {
            std::string_view::size_type current_pos{ 0 };
            auto on_new_line = writer_base<T>::back() == '\n';

            while (true)
            {
                const auto pos = value.find('\n', current_pos);

                if (pos == std::string_view::npos)
                {
                    if (current_pos < value.size())
                    {
                        if (on_new_line)
                        {
                            write_indent();
                        }

                        writer_base<T>::write_impl(value.substr(current_pos));
                    }

                    return;
                }

                auto current_line = value.substr(current_pos, pos - current_pos + 1);
                auto empty_line = current_line[0] == '\n';

                if (on_new_line && !empty_line)
                {
                    write_indent();
                }

                writer_base<T>::write_impl(current_line);

                on_new_line = true;
                current_pos = pos + 1;
            }
        }

        void write_impl(char const value)
        {
            if (writer_base<T>::back() == '\n' && value != '\n')
            {
                write_indent();
            }

            writer_base<T>::write_impl(value);
        }

        template <typename... Args>
        std::string write_temp(std::string_view const& value, Args const& ... args)
        {
            auto restore_indent = indent;
            indent = 0;

            auto result = writer_base<T>::write_temp(value, args...);

            indent = restore_indent;

            return result;
        }

        int32_t indent{};
    };

    struct writer : indented_writer_base<writer>
    {
        using indented_writer_base<writer>::write;

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
                names.push_back(write_temp("%", get_type_semantics(arg)));
            }

            generic_param_stack.push_back(std::move(names));
            return generic_param_guard{ this };
        }

        [[nodiscard]] auto push_generic_params(std::vector<type_semantics> const& type_arguments)
        {
            if (type_arguments.size() == 0)
            {
                return generic_param_guard{ nullptr };
            }

            std::vector<std::string> names;

            for (auto&& arg : type_arguments)
            {
                names.push_back(write_temp("%", arg));
            }

            generic_param_stack.push_back(std::move(names));
            return generic_param_guard{ this };
        }

#pragma endregion

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

        void write_value(int32_t value)
        {
            write_printf("%d", value);
        }

        void write_value(uint32_t value)
        {
            write_printf("%#0x", value);
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
            }
        }
   
        void write(fundamental_type const& type)
        {
            switch (type)
            {
            case fundamental_type::Boolean:
                write("bool");
                break;
            case fundamental_type::Char:
                write("char");
                break;
            case fundamental_type::Int8:
                write("sbyte");
                break;
            case fundamental_type::UInt8:
                write("byte");
                break;
            case fundamental_type::Int16:
                write("short");
                break;
            case fundamental_type::UInt16:
                write("ushort");
                break;
            case fundamental_type::Int32:
                write("int");
                break;
            case fundamental_type::UInt32:
                write("uint");
                break;
            case fundamental_type::Int64:
                write("long");
                break;
            case fundamental_type::UInt64:
                write("ulong");
                break;
            case fundamental_type::Float:
                write("float");
                break;
            case fundamental_type::Double:
                write("double");
                break;
            case fundamental_type::String:
                write("string");
                break;
            default:
                throw_invalid("invalid fundamental type");
            }
        }

        void write(object_type)
        {
            write("object");
        }

        void write(guid_type)
        {
            write("System.Guid");
        }

        void write(type_definition const& type)
        {
            write("%.@", type.TypeNamespace(), type.TypeName());
        }

        void write(generic_type_instance const& type)
        {
            write(type.generic_type);
            write("<");
            bool first{ true };
            for (auto&& type_arg : type.generic_args)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    write(", ");
                }
                write(type_arg);
            }
            write(">");
        }

        void write(generic_type_index const& var)
        {
            write(generic_param_stack.back()[var.index]);
        }

        void write(type_semantics const& semantics)
        {
            call(semantics,
                [&](auto && type)
                {
                    write(type);
                });
        }

        void write(TypeSig const& signature)
        {
            write(get_type_semantics(signature));
        }

        void write(coded_index<TypeDefOrRef> const& index)
        {
            write(get_type_semantics(index));
        }

        void write(RetTypeSig const& signature)
        {
            if (signature)
            {
                write(signature.Type());
            }
            else
            {
                write("void");
            }
        }

        void write(PropertySig const& signature)
        {
            write(signature.Type());
        }
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
}