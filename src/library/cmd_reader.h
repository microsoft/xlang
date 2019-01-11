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
#ifdef XLANG_DEBUG
            {
                std::set<std::string_view> unique;

                for (auto&& option : options)
                {
                    // If this assertion fails it means there are duplicate options.
                    XLANG_ASSERT(unique.insert(option.name).second);
                }
            }
#endif

            if (argc < 2)
            {
                return;
            }

            auto last{ options.end() };

            for (C i = 1; i < argc; ++i)
            {
                std::string_view arg{ argv[i] };

                if (arg[0] == '-')
                {
                    arg.remove_prefix(1);
                    last = find(options, arg);

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

            if (result == m_options.end() || result->second.empty())
            {
                return std::string{ default_value };
            }

            return result->second.front();
        }

        auto files(std::string_view const& name) const
        {
            return files(name, [](auto&&) {return true; });
        }

        template <typename F>
        auto files(std::string_view const& name, F directory_filter) const
        {
            std::set<std::string> files;

            auto add_directory = [&](auto&& path)
            {
                for (auto&& file : std::experimental::filesystem::directory_iterator(path))
                {
                    if (std::experimental::filesystem::is_regular_file(file))
                    {
                        auto filename = canonical(file.path()).string();

                        if (directory_filter(filename))
                        {
                            files.insert(filename);
                        }
                    }
                }
            };

            for (auto&& path : values(name))
            {
                auto canonical = std::experimental::filesystem::canonical(path);

                if (std::experimental::filesystem::is_directory(canonical))
                {
                    add_directory(canonical);
                    continue;
                }

                if (std::experimental::filesystem::is_regular_file(canonical))
                {
                    files.insert(canonical.string());
                    continue;
                }
#if XLANG_PLATFORM_WINDOWS
                if (path == "local")
                {
                    std::array<char, MAX_PATH> local{};
#ifdef _WIN64
                    ExpandEnvironmentStringsA("%windir%\\System32\\WinMetadata", local.data(), static_cast<DWORD>(local.size()));
#else
                    ExpandEnvironmentStringsA("%windir%\\SysNative\\WinMetadata", local.data(), static_cast<DWORD>(local.size()));
#endif
                    add_directory(local.data());
                    continue;
                }

                std::string sdk_version;

                if (path == "sdk" || path == "sdk+")
                {
                    sdk_version = get_sdk_version();
                }
                else
                {
                    std::regex rx(R"(((\d+)\.(\d+)\.(\d+)\.(\d+))\+?)");
                    std::smatch match;

                    if (std::regex_match(path, match, rx))
                    {
                        sdk_version = match[1].str();
                    }
                }

                if (!sdk_version.empty())
                {
                    auto sdk_path = get_sdk_path();
                    auto xml_path = sdk_path;
                    xml_path /= L"Platforms\\UAP";
                    xml_path /= sdk_version;
                    xml_path /= L"Platform.xml";

                    add_files_from_xml(files, sdk_version, xml_path, sdk_path);

                    if (path.back() != '+')
                    {
                        continue;
                    }

                    for (auto&& item : std::experimental::filesystem::directory_iterator(sdk_path / L"Extension SDKs"))
                    {
                        xml_path = item.path() / sdk_version;
                        xml_path /= L"SDKManifest.xml";

                        add_files_from_xml(files, sdk_version, xml_path, sdk_path);
                    }

                    continue;
                }
#endif
                throw_invalid("Path '", path, "' is not a file or directory");
            }

            return files;
        }

    private:

#if XLANG_PLATFORM_WINDOWS

        struct registry_key
        {
            HKEY handle{};

            registry_key(registry_key const&) = delete;
            registry_key& operator=(registry_key const&) = delete;

            ~registry_key() noexcept
            {
                if (handle)
                {
                    RegCloseKey(handle);
                }
            }
        };

        template <typename T>
        struct com_ptr
        {
            T* ptr{};

            com_ptr(com_ptr const&) = delete;
            com_ptr& operator=(com_ptr const&) = delete;

            com_ptr() noexcept = default;

            ~com_ptr() noexcept
            {
                if (ptr)
                {
                    ptr->Release();
                }
            }

            auto operator->() const noexcept
            {
                return ptr;
            }
        };

        static void check_xml(HRESULT result)
        {
            if (result < 0)
            {
                throw_invalid("Could not read the Windows SDK's Platform.xml");
            }
        }

        static void add_files_from_xml(
            std::set<std::string>& files,
            std::string const& sdk_version,
            std::experimental::filesystem::path const& xml_path,
            std::experimental::filesystem::path const& sdk_path)
        {
            com_ptr<IStream> stream;

            check_xml(SHCreateStreamOnFileW(
                xml_path.c_str(),
                STGM_READ, &stream.ptr));

            com_ptr<IXmlReader> reader;

            check_xml(CreateXmlReader(
                __uuidof(IXmlReader),
                reinterpret_cast<void**>(&reader.ptr),
                nullptr));

            check_xml(reader->SetInput(stream.ptr));
            XmlNodeType node_type = XmlNodeType_None;

            while (S_OK == reader->Read(&node_type))
            {
                if (node_type != XmlNodeType_Element)
                {
                    continue;
                }

                wchar_t const* value{ nullptr };
                check_xml(reader->GetLocalName(&value, nullptr));

                if (0 != wcscmp(value, L"ApiContract"))
                {
                    continue;
                }

                auto path = sdk_path;
                path /= L"References";
                path /= sdk_version;

                check_xml(reader->MoveToAttributeByName(L"name", nullptr));
                check_xml(reader->GetValue(&value, nullptr));
                path /= value;

                check_xml(reader->MoveToAttributeByName(L"version", nullptr));
                check_xml(reader->GetValue(&value, nullptr));
                path /= value;

                check_xml(reader->MoveToAttributeByName(L"name", nullptr));
                check_xml(reader->GetValue(&value, nullptr));
                path /= value;

                path += L".winmd";
                files.insert(path.string());
            }
        }

        static registry_key open_sdk()
        {
            HKEY key;

            if (0 != RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots",
                0,
                KEY_READ,
                &key))
            {
                throw_invalid("Could not find the Windows SDK");
            }

            return { key };
        }

        static std::experimental::filesystem::path get_sdk_path()
        {
            auto key = open_sdk();

            DWORD path_size = 0;

            if (0 != RegQueryValueExW(
                key.handle,
                L"KitsRoot10",
                nullptr,
                nullptr,
                nullptr,
                &path_size))
            {
                throw_invalid("Could not find the Windows SDK");
            }

            std::wstring root((path_size / sizeof(wchar_t)) - 1, L'?');

            RegQueryValueExW(
                key.handle,
                L"KitsRoot10",
                nullptr,
                nullptr,
                reinterpret_cast<BYTE*>(root.data()),
                &path_size);

            return root;
        }

        static std::string get_module_path()
        {
            std::string path(100, '?');
            DWORD actual_size{};

            while (true)
            {
                actual_size = GetModuleFileNameA(nullptr, path.data(), 1 + static_cast<uint32_t>(path.size()));

                if (actual_size < 1 + path.size())
                {
                    path.resize(actual_size);
                    break;
                }
                else
                {
                    path.resize(path.size() * 2, '?');
                }
            }

            return path;
        }

        static std::string get_sdk_version()
        {
            auto module_path = get_module_path();
            std::regex rx(R"(((\d+)\.(\d+)\.(\d+)\.(\d+)))");
            std::cmatch match;

            if (std::regex_search(module_path.c_str(), match, rx))
            {
                return match[1].str();
            }

            auto key = open_sdk();
            uint32_t index{};
            std::array<char, 100> subkey;
            std::array<unsigned long, 4> version_parts{};
            std::string result;

            while (0 == RegEnumKeyA(key.handle, index++, subkey.data(), static_cast<uint32_t>(subkey.size())))
            {
                if (!std::regex_match(subkey.data(), match, rx))
                {
                    continue;
                }

                char* next_part = subkey.data();

                for (size_t i = 0; ; ++i)
                {
                    auto version_part = strtoul(next_part, &next_part, 10);

                    if (version_part < version_parts[i])
                    {
                        break;
                    }

                    version_parts[i] = version_part;

                    if (i == std::size(version_parts) - 1)
                    {
                        result = subkey.data();
                        break;
                    }

                    if (!next_part)
                    {
                        break;
                    }

                    ++next_part;
                }
            }

            if (result.empty())
            {
                throw_invalid("Could not find the Windows SDK");
            }

            return result;
        }

#endif

        std::vector<option>::const_iterator find(std::vector<option> const& options, std::string_view const& arg)
        {
            for (auto current = options.begin(); current != options.end(); ++current)
            {
                if (starts_with(current->name, arg))
                {
                    return current;
                }
            }

            return options.end();
        }

        std::map<std::string_view, std::vector<std::string>> m_options;
    };
}
