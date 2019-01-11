#pragma once

#include "impl/base.h"

using namespace std::experimental::filesystem;

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
				extract_option(std::string{ argv[i] }, options, last);
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
            std::set<std::string> files;

            auto add_directory = [&](auto&& path)
            {
                for (auto&& file : std::experimental::filesystem::directory_iterator(path))
                {
                    if (std::experimental::filesystem::is_regular_file(file))
                    {
                        files.insert(canonical(file.path()).string());
                    }
                }
            };

            for (auto&& path : values(name))
            {
                auto canonical = std::experimental::filesystem::canonical(path);

                if (std::experimental::filesystem::is_directory(canonical))
                {
                    add_directory(canonical);
                }
                else if (std::experimental::filesystem::is_regular_file(canonical))
                {
                    files.insert(canonical.string());
                }
#if XLANG_PLATFORM_WINDOWS
                else if (path == "local")
                {
                    std::array<char, MAX_PATH> local{};
#ifdef _WIN64
                    ExpandEnvironmentStringsA("%windir%\\System32\\WinMetadata", local.data(), static_cast<DWORD>(local.size()));
#else
                    ExpandEnvironmentStringsA("%windir%\\SysNative\\WinMetadata", local.data(), static_cast<DWORD>(local.size()));
#endif
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

    private:

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

		template<typename O, typename L>
		void extract_option(std::string const& sarg, O const& options, L &last) 
		{
			std::string_view svarg{ sarg };
			if (svarg[0] == '-')
			{
				svarg.remove_prefix(1);
				last = find(options, svarg);

				if (last == options.end())
				{
					throw_invalid("Option '-", svarg, "' is not supported");
				}

				m_options.try_emplace(last->name);
			}
			else if (svarg[0] == '@')
			{
				svarg.remove_prefix(1);
				extract_respose_file(svarg, options);
			}
			else if (last == options.end())
			{
				throw_invalid("Value '", svarg, "' is not supported");
			}
			else
			{
				m_options[last->name].push_back(std::string{ svarg });
			}
		}

		template<typename O>
		void extract_respose_file(std::string_view const& arg, O const& options)
		{
			path response_path{ std::string{ arg } };
			std::wstring extension = response_path.extension();
			std::transform(extension.begin(), extension.end(), extension.begin(), tolower);
			if (is_directory(response_path) || extension == L".winmd")
			{
				throw_invalid("'@' is reserved for response files");
			}
			std::array<char, 8192> line_buf;
			std::ifstream response_file(absolute(response_path));
			while (response_file.getline(line_buf.data(), line_buf.size()))
			{
				size_t argc = 0;
				std::vector<std::string> argv;

				parse_command_line(line_buf.data(), argv, &argc);

				auto last{ options.end() };
				for (auto i = 0; i < argc; i++)
				{
					extract_option(argv[i], options, last);
				}
			}
		}

		template <typename Character>
		static void parse_command_line(
			Character* cmdstart,
			std::vector<std::string>& argv,
			size_t* argument_count
			) throw()
		{
			*argument_count  = 0;

			Character c;
			std::string args("");
			int copy_character;                   /* 1 = copy char to *args */
			unsigned numslash;              /* num of backslashes seen */
			bool in_quotes;
			bool first_arg;

			/* first scan the program name, copy it, and count the bytes */
			Character* p = cmdstart;
			in_quotes = false;
			first_arg = true;

			// Loop on each argument
			for (;;)
			{
				if (*p)
				{
					while (*p == ' ' || *p == '\t')
						++p;
				}

				// Scan an argument:
				if (!first_arg)
				{
					argv.emplace_back(args);
					args.clear();
					++*argument_count;
				}

				if (*p == '\0')
					break; // End of arguments

				// Loop through scanning one argument:
				for (;;)
				{
					copy_character = 1;

					// Rules:
					// 2N     backslashes   + " ==> N backslashes and begin/end quote
					// 2N + 1 backslashes   + " ==> N backslashes + literal "
					// N      backslashes       ==> N backslashes
					numslash = 0;

					while (*p == '\\')
					{
						// Count number of backslashes for use below
						++p;
						++numslash;
					}

					if (*p == '"')
					{
						// if 2N backslashes before, start/end quote, otherwise
						// copy literally:
						if (numslash % 2 == 0)
						{
							if (in_quotes && p[1] == '"')
							{
								p++; // Double quote inside quoted string
							}
							else
							{
								// Skip first quote char and copy second:
								copy_character = 0; // Don't copy quote
								in_quotes = !in_quotes;
							}
						}

						numslash /= 2;
					}

					// Copy slashes:
					while (numslash--)
					{
						args.push_back('\\');
					}

					// If at end of arg, break loop:
					if (*p == '\0' || (!in_quotes && (*p == ' ' || *p == '\t')))
						break;

					// Copy character into argument:
					if (copy_character)
					{
						args.push_back(*p);
					}

					++p;
				}

				first_arg = false;
			}
		}
    };
}
