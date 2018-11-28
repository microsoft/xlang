#include "winrt/Windows.Foundation.Collections.h"
#pragma comment(lib, "windowsapp")

  using namespace winrt;
  using namespace Windows::Foundation;
  using namespace Windows::Foundation::Collections;

IAsyncOperation<hstring> Async()
{
  co_return L"Hello";
}

int main()
{
  init_apartment();
  IObservableVector<int> v = single_threaded_observable_vector<int>();
  //return v.Size();
  
  TypedEventHandler<int, bool> d = [](int, bool){};
  
  printf("%ls\n", Async().get().c_str());
}
