#include "pch.h"

using namespace xlang::meta::reader;

TEST_CASE("meta_reader_cache")
{
    auto current = std::experimental::filesystem::current_path();

    cache c{ R"(simple.winmd)" };

    auto type = c.find("Simple.IStringable");
    REQUIRE(type.TypeName() == "IStringable");
    REQUIRE(type.TypeNamespace() == "Simple");
    REQUIRE(type.Flags().Semantics() == TypeSemantics::Interface);
}
