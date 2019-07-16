#include "pch.h"
#include <pplawait.h>

using namespace concurrency;
using namespace winrt;
using namespace Windows::Foundation;

task<void> ppl(bool& done)
{
    co_await resume_background();
    done = true;
}

IAsyncAction async(bool& done)
{
    co_await resume_background();
    done = true;
}

TEST_CASE("when_all")
{
    bool ppl_done = false;
    bool async_done = false;

    winrt::when_all(ppl(ppl_done), async(async_done)).get();

    REQUIRE(ppl_done);
    REQUIRE(async_done);
}
