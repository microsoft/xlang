#include "pch.h"
#include "meta_reader.h"
#include <iostream>

using namespace xlang::meta::reader;

TEST_CASE("scratch")
{
    std::vector<std::string> files
    {
        R"(c:\windows\system32\WinMetadata\Windows.UI.winmd)",
        R"(c:\windows\system32\WinMetadata\Windows.Storage.winmd)"
    };

    cache c(files);

    for (auto&& [ns, members] : c.namespaces())
    {
        std::cout << ns << '\n';

        for (auto&& type : members.classes)
        {
            std::cout << '\t' << type.TypeName() << '\n';
        }
    }
}
