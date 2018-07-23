
namespace xlang::meta::reader
{
    struct cache
    {
        cache(cache const&) = delete;
        cache& operator=(cache const&) = delete;

        explicit cache(std::vector<std::string> const& files)
        {
            for (auto&& file : files)
            {
                auto& db = m_databases.emplace_back(file, this);

                for (auto&& type : db.TypeDef)
                {
                    auto const flags = type.Flags();

                    if (enum_mask(flags, TypeAttributes::WindowsRuntime) != TypeAttributes::WindowsRuntime)
                    {
                        continue;
                    }

                    auto& ns = m_namespaces[type.TypeNamespace()];
                    auto insert = ns.types.try_emplace(type.TypeName(), type);

                    if (insert.second == false)
                    {
                        // TODO: test with MIDL as we may need to support winmd files generated with duplicate TypeDef rows.
                        throw_invalid(L"Duplicate type indicates invalid combination of metadata files");
                    }
                }
            }

            for (auto&& [namespace_name, members] : m_namespaces)
            {
                for (auto&& [name, type] : members.types)
                {
                    auto const flags = type.Flags();

                    if (enum_mask(flags, TypeAttributes::Interface) == TypeAttributes::Interface)
                    {
                        members.interfaces.push_back(type);
                        continue;
                    }

                    auto const extends = type.Extends().TypeRef();
                    auto extends_name = extends.TypeName();
                    auto extends_namespace = extends.TypeNamespace();

                    if (extends_name == "Attribute"sv && extends_namespace == "System"sv)
                    {
                        members.attributes.push_back(type);
                        continue;
                    }

                    if (extends_name == "Enum"sv && extends_namespace == "System"sv)
                    {
                        members.enums.push_back(type);
                        continue;
                    }

                    if (extends_name == "ValueType"sv && extends_namespace == "System"sv)
                    {
                        if (type.has_attribute("Windows.Foundation.Metadata"sv, "ApiContractAttribute"sv))
                        {
                            members.contracts.push_back(type);
                            continue;
                        }

                        members.structs.push_back(type);
                        continue;
                    }

                    if (extends_name == "MulticastDelegate"sv && extends_namespace == "System"sv)
                    {
                        members.delegates.push_back(type);
                        continue;
                    }

                    members.classes.push_back(type);
                }
            }
        }

        std::optional<TypeDef> find(std::string_view const& type_namespace, std::string_view const& type_name) const noexcept
        {
            auto ns = m_namespaces.find(type_namespace);

            if (ns == m_namespaces.end())
            {
                return std::nullopt;
            }

            auto type = ns->second.types.find(type_name);

            if (type == ns->second.types.end())
            {
                return std::nullopt;
            }

            return type->second;
        }

        auto const& databases() const noexcept
        {
            return m_databases;
        }

        auto const& namespaces() const noexcept
        {
            return m_namespaces;
        }

        struct namespace_members
        {
            std::map<std::string_view, TypeDef> types;
            std::vector<TypeDef> interfaces;
            std::vector<TypeDef> classes;
            std::vector<TypeDef> enums;
            std::vector<TypeDef> structs;
            std::vector<TypeDef> delegates;
            std::vector<TypeDef> attributes;
            std::vector<TypeDef> contracts;
        };

        using namespace_type = std::pair<std::string_view const, namespace_members> const&;

    private:

        std::list<database> m_databases;
        std::map<std::string_view, namespace_members> m_namespaces;
    };
}
