#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

int main(int const argc, char** argv)
{
    return Catch::Session().run(argc, argv);
}
