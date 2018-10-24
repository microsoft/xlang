#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

int main(int const argc, char** argv)
{
    // Needs this in the json to get it to work in VS
    //   "currentDir": "c:\\git\\Langworthy\\xlang\\test\\output"

    return Catch::Session().run(argc, argv);
}
