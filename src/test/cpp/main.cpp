#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "winrt/base.h"

CATCH_TRANSLATE_EXCEPTION(winrt::hresult_error const& e)
{
    return winrt::to_string(e.message());
}

int main(int const argc, char** argv)
{
    return Catch::Session().run(argc, argv);
}
