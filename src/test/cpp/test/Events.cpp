#include "pch.h"
#include "winrt/Component.Events.h"

using namespace winrt;
using namespace Component;

TEST_CASE("Events")
{
    Events::Class instance;
    event_token a = instance.Member([](int){});
    instance.Member(a);

    event_token b = Events::Class::Static([](int){});
    Events::Class::Static(b);

    Events::Class::Member_revoker c = instance.Member(auto_revoke, [](int){});
    c.revoke();

    //Events::Class::Static_revoker d = Events::Class::Static(auto_revoke, [](int){});
    //d.revoke();
}
