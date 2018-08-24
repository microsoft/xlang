#pragma once

#include "impl/base.h"

namespace xlang::cmd
{
    struct option
    {
        std::string_view name;
        uint32_t min{ 0 };
        uint32_t max{ std::numeric_limits<uint32_t>::max() };
    };

    struct reader
    {
        template <typename C, typename V>
        reader(C const argc, V argv, std::vector<option> const& options)
        {
            if (argc < 2)
            {
                return;
            }

            auto last{ options.end() };

            for (int i = 1; i < argc; ++i)
            {
                std::string_view arg{ argv[i] };

                if (arg[0] == '-')
                {
                    arg.remove_prefix(1);

                    last = std::find_if(options.begin(), options.end(), [&](auto&& option)
                    {
                        return option.name == arg;
                    });

                    if (last == options.end())
                    {
                        throw_invalid("Option '-", arg, "' is not supported");
                    }

                    m_options.try_emplace(last->name);
                }
                else if (last == options.end())
                {
                    throw_invalid("Value '", arg, "' is not supported");
                }
                else
                {
                    m_options[last->name].push_back(std::string{ arg });
                }
            }

            for (auto&& option : options)
            {
                auto args = m_options.find(option.name);
                std::size_t const count = args == m_options.end() ? 0 : args->second.size();

                if (option.min == 0 && option.max == 0 && count > 0)
                {
                    throw_invalid("Option '", option.name, "' does not accept a value");
                }
                else if (option.max == option.min && count != option.max)
                {
                    throw_invalid("Option '", option.name, "' requires exactly ", std::to_string(option.max), " value(s)");
                }
                else if (count < option.min)
                {
                    throw_invalid("Option '", option.name, "' requires at least ", std::to_string(option.min), " value(s)");
                }
                else if (count > option.max)
                {
                    throw_invalid("Option '", option.name, "' accepts at most ", std::to_string(option.max), " value(s)");
                }
            }
        }

        explicit operator bool() const noexcept
        {
            return !m_options.empty();
        }

        bool exists(std::string_view const& name) const noexcept
        {
            return m_options.count(name);
        }

        auto const& values(std::string_view const& name) const noexcept
        {
            auto result = m_options.find(name);

            if (result == m_options.end())
            {
                static std::vector<std::string> empty{};
                return empty;
            }

            return result->second;
        }

        auto value(std::string_view const& name, std::string_view const& default_value = {}) const
        {
            auto result = m_options.find(name);

            if (result == m_options.end())
            {
                return std::string{ default_value };
            }

            return result->second.front();
        }

    private:

        std::map<std::string_view, std::vector<std::string>> m_options;
    };
}
