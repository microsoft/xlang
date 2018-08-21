
namespace xlang::meta::reader
{
    struct cache
    {
        cache(cache const&) = delete;
        cache& operator=(cache const&) = delete;

        explicit cache(std::vector<std::string> const& args)
        {
            for (auto&& file : expand(args))
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
                        throw_invalid("Duplicate type indicates invalid combination of metadata files");
                    }
                }
            }

            for (auto&&[namespace_name, members] : m_namespaces)
            {
                for (auto&&[name, type] : members.types)
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
                        if (get_attribute(type, "Windows.Foundation.Metadata"sv, "ApiContractAttribute"sv))
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

        explicit cache(std::string const& file) : cache{ std::vector<std::string>{ file } }
        {
        }

        TypeDef find(std::string_view const& type_namespace, std::string_view const& type_name) const noexcept
        {
            auto ns = m_namespaces.find(type_namespace);

            if (ns == m_namespaces.end())
            {
                return {};
            }

            auto type = ns->second.types.find(type_name);

            if (type == ns->second.types.end())
            {
                return {};
            }

            return type->second;
        }

        TypeDef find(std::string_view const& type_string) const noexcept
        {
            auto pos = type_string.rfind('.');
            if (pos == std::string_view::npos)
            {
                throw_invalid("Type name is missing namespace separator");
            }
            return find(type_string.substr(0, pos), type_string.substr(pos + 1, type_string.size()));
        }

        auto const& databases() const noexcept
        {
            return m_databases;
        }

        auto const& namespaces() const noexcept
        {
            return m_namespaces;
        }

        void remove_legacy_cppwinrt_foundation_types()
        {
            // TODO: remove this function once cpp.exe generates these base types (soon)...

            auto remove = [&](auto&& ns, auto&& name)
            {
                auto& members = m_namespaces[ns];

                auto remove = [&](auto&& collection, auto&& name)
                {
                    auto pos = std::find_if(collection.begin(), collection.end(), [&](auto&& type)
                    {
                        return type.TypeName() == name;
                    });

                    if (pos != collection.end())
                    {
                        collection.erase(pos);
                    }
                };

                remove(members.interfaces, name);
                remove(members.classes, name);
                remove(members.enums, name);
                remove(members.structs, name);
                remove(members.delegates, name);
                remove(members.classes, name);
            };

            remove("Windows.Foundation", "IAsyncInfo");
            remove("Windows.Foundation", "IAsyncAction");
            remove("Windows.Foundation", "IAsyncActionWithProgress`1");
            remove("Windows.Foundation", "IAsyncOperation`1");
            remove("Windows.Foundation", "IAsyncOperationWithProgress`2");
            remove("Windows.Foundation", "IReference`1");
            remove("Windows.Foundation", "IReferenceArray`1");

            remove("Windows.Foundation", "AsyncStatus");
            remove("Windows.Foundation", "DateTime");
            remove("Windows.Foundation", "EventRegistrationToken");
            remove("Windows.Foundation", "HResult");
            remove("Windows.Foundation", "Point");
            remove("Windows.Foundation", "Rect");
            remove("Windows.Foundation", "Size");
            remove("Windows.Foundation", "TimeSpan");

            remove("Windows.Foundation", "AsyncActionCompletedHandler");
            remove("Windows.Foundation", "AsyncActionProgressHandler`1");
            remove("Windows.Foundation", "AsyncActionWithProgressCompletedHandler`1");
            remove("Windows.Foundation", "AsyncOperationCompletedHandler`1");
            remove("Windows.Foundation", "AsyncOperationProgressHandler`2");
            remove("Windows.Foundation", "AsyncOperationWithProgressCompletedHandler`2");
            remove("Windows.Foundation", "EventHandler`1");
            remove("Windows.Foundation", "TypedEventHandler`2");
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

        static std::vector<std::string> expand(std::vector<std::string> const& args)
        {
            std::vector<std::string> files;

            auto add_directory = [&](auto&& path)
            {
                for (auto&& file : std::experimental::filesystem::directory_iterator(path))
                {
                    if (std::experimental::filesystem::is_regular_file(file))
                    {
                        files.push_back(file.path().string());
                    }
                }
            };

            for (auto&& path : args)
            {
                auto absolute = std::experimental::filesystem::absolute(path);

                if (std::experimental::filesystem::is_directory(absolute))
                {
                    add_directory(absolute);
                }
                else if (std::experimental::filesystem::is_regular_file(absolute))
                {
                    files.push_back(absolute.string());
                }
#if XLANG_PLATFORM_WINDOWS
                else if (path == "local")
                {
                    std::array<char, MAX_PATH> local{};
                    ExpandEnvironmentStringsA("%windir%\\System32\\WinMetadata", local.data(), static_cast<DWORD>(local.size()));
                    add_directory(local.data());
                }
#endif
                else
                {
                    throw_invalid("Path '", path, "' is not a file or directory");
                }
            }

            return files;
        }

        std::list<database> m_databases;
        std::map<std::string_view, namespace_members> m_namespaces;
    };
}
