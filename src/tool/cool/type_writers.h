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

		template <typename F>
		[[nodiscard]] auto push_generic_params(type_semantics const& semantics, F func)
		{
			auto type_instance = std::get_if<generic_type_instance>(&semantics);
			if (type_instance)
			{
				std::vector<std::string> names;

				for (auto&& arg : type_instance->generic_args)
				{
					names.push_back(func(arg));
				}

				generic_param_stack.push_back(std::move(names));
				return generic_param_guard{ this };
			}
			else
			{
				return generic_param_guard{ nullptr };
			}
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