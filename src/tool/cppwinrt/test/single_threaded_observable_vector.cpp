#include "pch.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

TEST_CASE("single_threaded_observable_vector")
{
    {
        // Basic smoke test for strongly-typed observable vector - exhaustive tests already exist elsewhere.

        IObservableVector<int> vector = single_threaded_observable_vector<int>();
        bool changed{};

        vector.VectorChanged([&](auto && ...)
            {
                changed = vector.GetAt(0) == 123;
            });

        vector.Append(123);
        REQUIRE(changed);
    }
    {
        // Making sure we can still create IInspectable collections are aren't "special".

        IObservableVector<IInspectable> vector = single_threaded_observable_vector<IInspectable>();
        bool changed{};

        vector.VectorChanged([&](auto && ...)
            {
                changed = unbox_value<int>(vector.GetAt(0)) == 123;
            });

        vector.Append(box_value(123));
        REQUIRE(changed);
    }
    {
        IObservableVector<int> vector_i = single_threaded_observable_vector<int>();
        IObservableVector<IInspectable> vector_o = vector_i.as<IObservableVector<IInspectable>>();
        bool changed_i{};
        bool changed_o{};

        vector_i.VectorChanged([&](IObservableVector<int> const& sender, IVectorChangedEventArgs const&)
            {
                changed_i = sender.GetAt(0) == 123;
                REQUIRE(sender == vector_i);
            });

        vector_o.VectorChanged([&](IObservableVector<IInspectable> const& sender, IVectorChangedEventArgs const&)
            {
                changed_o = unbox_value<int>(sender.GetAt(0)) == 123;
                REQUIRE(sender == vector_o);
            });

        vector_i.Append(123);
        REQUIRE(changed_i);
        REQUIRE(changed_o);
    }
}
