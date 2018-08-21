#include "pch.h"

using namespace xlang::meta::reader;

TEST_CASE("meta_reader_cache")
{
    cache c{ R"(test\il\meta_reader_cache.winmd)" };

    auto type = c.find("Test.IStringable");
    REQUIRE(type.TypeName() == "IStringable");
    REQUIRE(type.TypeNamespace() == "Test");
}
