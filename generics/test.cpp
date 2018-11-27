#include "winrt/Windows.Foundation.Collections.h"
#pragma comment(lib, "windowsapp")

int main()
{
  using namespace winrt;
  using namespace Windows::Foundation::Collections;
  
  IObservableVector<int> v = single_threaded_observable_vector<int>();
  return v.Size();
}
