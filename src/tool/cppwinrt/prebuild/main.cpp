#include "pch.h"

struct writer : xlang::text::writer_base<writer>
{

};

int main(int const argc, char** argv)
{
    if (argc < 3)
    {
        puts("Usage: prebuild.exe input output");
        return 0;
    }

    writer strings_h;
    writer strings_cpp;

    strings_h.write(R"(
#pragma once
namespace xlang::strings {
)");

    strings_cpp.write(R"(
#include "pch.h"
namespace xlang::strings {
)");

    for (auto&& file : std::experimental::filesystem::directory_iterator(argv[1]))
    {
        if (!std::experimental::filesystem::is_regular_file(file))
        {
            continue;
        }

        auto path = file.path();
        auto name = path.filename();
        name.replace_extension();

        xlang::meta::reader::file_view const view{ path.string() };

        strings_h.write(R"(extern char const %[%];
)",
            name.string(),
            static_cast<uint64_t>(view.size()));

        strings_cpp.write(R"(extern char const %[] = R"xyz()xyz"
)",
            name.string());

        xlang::meta::reader::byte_view remainder = view;

        while (remainder.size())
        {
            auto const size = std::min(16'000u, remainder.size());
            auto const chunk = remainder.sub(0, size);

            strings_cpp.write(R"(R"xyz(%)xyz"
)",
                std::string_view{ reinterpret_cast<char const*>(chunk.begin()), chunk.size() }); // TODO: This conversion should not be required

            remainder = remainder.seek(size);
        }

        strings_cpp.write(";\n");
    }

    strings_h.write(R"(
}
)");

    strings_cpp.write(R"(
}
)");

    writer version_rc;

    version_rc.write(R"(
#include "winres.h"

VS_VERSION_INFO VERSIONINFO
 FILEVERSION 2,0,0,0
 PRODUCTVERSION 2,0,0,0
 FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x40004L
 FILETYPE 0x0L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "CompanyName", "Microsoft Corporation"
            VALUE "FileDescription", "C++/WinRT"
            VALUE "FileVersion", "2.0.0.0"
            VALUE "LegalCopyright", "Microsoft Corporation. All rights reserved."
            VALUE "OriginalFilename", "cppwinrt.exe"
            VALUE "ProductName", "C++/WinRT"
            VALUE "ProductVersion", "%"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END
)",
    XLANG_VERSION_STRING);

    auto const output = std::experimental::filesystem::canonical(argv[2]);
    std::experimental::filesystem::create_directories(output);
    strings_h.flush_to_file(output / "strings.h");
    strings_cpp.flush_to_file(output / "strings.cpp");
    version_rc.flush_to_file(output / "version.rc");
}
