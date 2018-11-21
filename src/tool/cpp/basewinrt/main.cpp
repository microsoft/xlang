#include "base.h"

int main()
{
    using namespace winrt::Windows::Foundation;

    EventHandler<int> a;
    EventHandler<int> h = [](auto&&...){};
    EventHandler<int> g = { nullptr, winrt::take_ownership_from_abi };
}
