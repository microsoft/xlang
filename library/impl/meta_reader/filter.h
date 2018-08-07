
namespace xlang::meta::reader
{
    struct filter
    {
        filter() noexcept = default;
        filter(filter const&) = delete;
        filter& operator=(filter const&) = delete;

        void include(std::vector<std::string> const& values)
        {
            m_includes = values;
        }

        void exclude(std::vector<std::string> const& values)
        {
            m_excludes = values;
        }

        bool match(TypeDef const& type) const
        {
            return match(type.TypeNamespace(), type.TypeName());
        }

        bool match(cache::namespace_members const& members) const
        {
            if (m_includes.empty() && m_excludes.empty())
            {
                return true;
            }

            for (auto&& type : members.types)
            {
                if (match(type.second.TypeNamespace(), type.second.TypeName()))
                {
                    return true;
                }
            }

            return false;
        }

        template <auto F>
        auto bind_each(std::vector<TypeDef> const& types) const
        {
            return [&](auto& writer)
            {
                for (auto&& type : types)
                {
                    if (match(type))
                    {
                        F(writer, type);
                    }
                }
            };
        }

    private:

        bool match(std::string_view const& type_namespace, std::string_view const& type_name) const noexcept
        {
            if (m_includes.empty() && m_excludes.empty())
            {
                return true;
            }

            if (!m_includes.empty())
            {
                bool included{};

                for (auto&& include : m_includes)
                {
                    if (match(type_namespace, type_name, include))
                    {
                        included = true;
                        break;
                    }
                }

                if (!included)
                {
                    return false;
                }
            }

            for (auto&& exclude : m_excludes)
            {
                if (match(type_namespace, type_name, exclude))
                {
                    return false;
                }
            }

            return true;
        }

        static bool match(std::string_view const& type_namespace, std::string_view const& type_name, std::string_view const& match) noexcept
        {
            if (match.size() <= type_namespace.size())
            {
                return starts_with(type_namespace, match);
            }

            if (!starts_with(match, type_namespace))
            {
                return false;
            }

            if (match[type_namespace.size()] != '.')
            {
                return false;
            }

            return starts_with(type_name, match.substr(type_namespace.size() + 1));
        }

        std::vector<std::string> m_includes;
        std::vector<std::string> m_excludes;
    };
}
