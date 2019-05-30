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
    {
        IObservableVector<int> vector_i = single_threaded_observable_vector<int>();
        IObservableVector<IInspectable> vector_o = vector_i.as<IObservableVector<IInspectable>>();

        vector_i.Append(0);
        vector_o.Append(box_value(1));

        REQUIRE(vector_i.Size() == 2);
        REQUIRE(vector_o.Size() == 2);

        REQUIRE(vector_i.GetAt(0) == 0);
        REQUIRE(unbox_value<int>(vector_o.GetAt(0)) == 0);

        REQUIRE(vector_i.GetAt(1) == 1);
        REQUIRE(unbox_value<int>(vector_o.GetAt(1)) == 1);
    }
    {
        IObservableVector<int> vector_i = single_threaded_observable_vector<int>({ 1,2 });
        IObservableVector<IInspectable> vector_o = vector_i.as<IObservableVector<IInspectable>>();

        IIterator<int> iterator_i = vector_i.First();
        IIterator<IInspectable> iterator_o = vector_o.First();

        REQUIRE(iterator_i.HasCurrent());
        REQUIRE(iterator_o.HasCurrent());

        REQUIRE(iterator_i.Current() == 1);
        REQUIRE(unbox_value<int>(iterator_o.Current()) == 1);

        REQUIRE(iterator_i.MoveNext());
        REQUIRE(iterator_o.MoveNext());

        REQUIRE(iterator_i.HasCurrent());
        REQUIRE(iterator_o.HasCurrent());

        REQUIRE(iterator_i.Current() == 2);
        REQUIRE(unbox_value<int>(iterator_o.Current()) == 2);

        REQUIRE(!iterator_i.MoveNext());
        REQUIRE(!iterator_o.MoveNext());

        REQUIRE(!iterator_i.HasCurrent());
        REQUIRE(!iterator_o.HasCurrent());

        REQUIRE_THROWS_AS(iterator_i.Current(), hresult_out_of_bounds);
        REQUIRE_THROWS_AS(iterator_o.Current(), hresult_out_of_bounds);
    }
    {
        IObservableVector<int> vector_i = single_threaded_observable_vector<int>({ 1,2 });
        IObservableVector<IInspectable> vector_o = vector_i.as<IObservableVector<IInspectable>>();

        IIterator<int> iterator_i = vector_i.First();
        IIterator<IInspectable> iterator_o = vector_o.First();

        std::array<int, 3> array_i{};
        REQUIRE(2 == iterator_i.GetMany(array_i));
        REQUIRE(array_i[0] == 1);
        REQUIRE(array_i[1] == 2);
        REQUIRE(array_i[2] == 0);

        std::array<IInspectable, 3> array_o{};
        REQUIRE(2 == iterator_o.GetMany(array_o));
        REQUIRE(unbox_value<int>(array_o[0]) == 1);
        REQUIRE(unbox_value<int>(array_o[1]) == 2);
        REQUIRE(array_o[2] == nullptr);

        REQUIRE(0 == iterator_i.GetMany(array_i));
        REQUIRE(0 == iterator_o.GetMany(array_o));
    }
    {
        IObservableVector<int> vector_i = single_threaded_observable_vector<int>({ 1,2 });
        IObservableVector<IInspectable> vector_o = vector_i.as<IObservableVector<IInspectable>>();

        IIterator<int> iterator_i = vector_i.First();
        IIterator<IInspectable> iterator_o = vector_o.First();

        std::array<int, 1> array_i{};
        REQUIRE(1 == iterator_i.GetMany(array_i));
        REQUIRE(array_i[0] == 1);

        std::array<IInspectable, 1> array_o{};
        REQUIRE(1 == iterator_o.GetMany(array_o));
        REQUIRE(unbox_value<int>(array_o[0]) == 1);
    }

    {
        IObservableVector<int> vector_i = single_threaded_observable_vector<int>({ 1,2 });
        IObservableVector<IInspectable> vector_o = vector_i.as<IObservableVector<IInspectable>>();

        IIterator<int> iterator_i = vector_i.First();
        IIterator<IInspectable> iterator_o = vector_o.First();

        iterator_i.HasCurrent();
        iterator_o.HasCurrent();

        vector_i.Append(3);

        REQUIRE_THROWS_AS(iterator_i.HasCurrent(), hresult_changed_state);
        REQUIRE_THROWS_AS(iterator_o.HasCurrent(), hresult_changed_state);
    }
}
