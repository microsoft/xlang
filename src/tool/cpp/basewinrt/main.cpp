#include "base.h"
#include <stdio.h>

int main()
{
    using namespace winrt;

    delegate<> d = [] { puts("delegate"); };

    d();
}
