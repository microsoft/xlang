#include "base.h"

using namespace xlang;
using namespace Windows::Foundation;

IAsyncOperation<hstring> Sample()
{
    co_return L"Hello xlang";
}

int main()
{
    printf("%ls\n", Sample().get().c_str());
}
