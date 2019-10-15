#pragma once

#include "Class.g.h"

namespace winrt::TestComp::implementation
{
    struct Class : ClassT<Class>
    {
        Class()
        {
        }

        winrt::event<EventHandler0> _event0;
        winrt::event<EventHandler1> _event1;
        winrt::event<EventHandler2> _event2;
        winrt::event<EventHandler3> _event3;
        winrt::event<EventHandlerCollection> _collectionEvent;
        winrt::event<Windows::Foundation::EventHandler<Windows::Foundation::Collections::IVector<int32_t>>> _nestedEvent;
        winrt::event<Windows::Foundation::TypedEventHandler<TestComp::Class, Windows::Foundation::Collections::IVector<hstring>>> _nestedTypedEvent;

        int32_t _int = 0;
        winrt::event<Windows::Foundation::EventHandler<int32_t>> _intChanged;
        winrt::hstring _string;
        winrt::hstring _string2;
        winrt::event<Windows::Foundation::TypedEventHandler<TestComp::Class, hstring>> _stringChanged;
        Windows::Foundation::Collections::IVector<hstring> _strings;

        winrt::event_token Event0(TestComp::EventHandler0 const& handler);
        void Event0(winrt::event_token const& token) noexcept;
        void InvokeEvent0();
        winrt::event_token Event1(TestComp::EventHandler1 const& handler);
        void Event1(winrt::event_token const& token) noexcept;
        void InvokeEvent1(TestComp::Class const& sender);
        winrt::event_token Event2(TestComp::EventHandler2 const& handler);
        void Event2(winrt::event_token const& token) noexcept;
        void InvokeEvent2(TestComp::Class const& sender, int32_t arg0);
        winrt::event_token Event3(TestComp::EventHandler3 const& handler);
        void Event3(winrt::event_token const& token) noexcept;
        void InvokeEvent3(TestComp::Class const& sender, int32_t arg0, hstring const& arg1);
        winrt::event_token CollectionEvent(TestComp::EventHandlerCollection const& handler);
        void CollectionEvent(winrt::event_token const& token) noexcept;
        void InvokeCollectionEvent(TestComp::Class const& sender, Windows::Foundation::Collections::IVector<int32_t> const& arg0, Windows::Foundation::Collections::IMap<int32_t, hstring> const& arg1);
        winrt::event_token NestedEvent(Windows::Foundation::EventHandler<Windows::Foundation::Collections::IVector<int32_t>> const& handler);
        void NestedEvent(winrt::event_token const& token) noexcept;
        void InvokeNestedEvent(TestComp::Class const& sender, Windows::Foundation::Collections::IVector<int32_t> const& arg0);
        winrt::event_token NestedTypedEvent(Windows::Foundation::TypedEventHandler<TestComp::Class, Windows::Foundation::Collections::IVector<hstring>> const& handler);
        void NestedTypedEvent(winrt::event_token const& token) noexcept;
        void InvokeNestedTypedEvent(TestComp::Class const& sender, Windows::Foundation::Collections::IVector<hstring> const& arg0);
        int32_t IntProperty();
        void IntProperty(int32_t value);
        winrt::event_token IntPropertyChanged(Windows::Foundation::EventHandler<int32_t> const& handler);
        void IntPropertyChanged(winrt::event_token const& token) noexcept;
        hstring StringProperty();
        void StringProperty(hstring const& value);
        winrt::event_token StringPropertyChanged(Windows::Foundation::TypedEventHandler<TestComp::Class, hstring> const& handler);
        void StringPropertyChanged(winrt::event_token const& token) noexcept;
        hstring StringProperty2();
        void StringProperty2(hstring const& value);
        Windows::Foundation::Collections::IVector<hstring> StringsProperty();
        void StringsProperty(Windows::Foundation::Collections::IVector<hstring> value);
        void GetString();
        void SetString(TestComp::ProvideString const& provideString);

        // IStringable
        hstring ToString();

        // IVector<hstring>
        //Windows::Foundation::Collections::IIterator<hstring> First();
        //hstring GetAt(uint32_t index);
        //uint32_t Size();
        //Windows::Foundation::Collections::IVectorView<hstring> GetView();
        //bool IndexOf(hstring const& value, uint32_t& index);
        //void SetAt(uint32_t index, hstring const& value);
        //void InsertAt(uint32_t index, hstring const& value);
        //void RemoveAt(uint32_t index);
        //void Append(hstring const& value);
        //void RemoveAtEnd();
        //void Clear();
        //uint32_t GetMany(uint32_t startIndex, array_view<hstring> items);
        //void ReplaceAll(array_view<hstring const> items);
    };
}

namespace winrt::TestComp::factory_implementation
{
    struct Class : ClassT<Class, implementation::Class>
    {
    };
}
