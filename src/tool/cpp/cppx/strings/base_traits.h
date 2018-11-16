
namespace xlang::impl
{
    template <> struct guid_storage<Runtime::IUnknown>
    {
        static constexpr guid value{ 0x00000000,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<Runtime::IObject>
    {
        static constexpr guid value{ 0x9df517c6, 0x60f2, 0x4e17,{ 0x93,0xc7,0xcd,0xd,0x45,0x92,0xc2,0x42 } };
    };

    template <> struct guid_storage<Runtime::IActivationFactory>
    {
        static constexpr guid value{ 0x00000035,0x0000,0x0000,{ 0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<IWeakReference>
    {
        static constexpr guid value{ 0x00000037,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<IWeakReferenceSource>
    {
        static constexpr guid value{ 0x00000038,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<ILanguageExceptionErrorInfo2>
    {
        static constexpr guid value{ 0x5746E5C4,0x5B97,0x424C,{ 0xB6,0x20,0x28,0x22,0x91,0x57,0x34,0xDD } };
    };

    template <> struct guid_storage<Runtime::IVectorChangedEventArgs>
    {
        static constexpr guid value{ 0x575933DF,0x34FE,0x4480,{ 0xAF,0x15,0x07,0x69,0x1F,0x3D,0x5D,0x9B } };
    };

    template <> struct guid_storage<Runtime::AsyncActionCompletedHandler>
    {
        static constexpr guid value{ 0xA4ED5C81,0x76C9,0x40BD,{ 0x8B,0xE6,0xB1,0xD9,0x0F,0xB2,0x0A,0xE7 } };
    };

    template <> struct guid_storage<Runtime::IAsyncAction>
    {
        static constexpr guid value{ 0xca249118, 0xe1e7, 0x4c85,{ 0x89,0x58,0x2,0x76,0x7,0x78,0x76,0xf0 } };
    };

    template <typename T> struct guid_storage<Runtime::IReference<T>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IReference<T>>::value };
    };

    template <typename T> struct guid_storage<Runtime::IReferenceArray<T>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IReferenceArray<T>>::value };
    };

    template <typename TResult> struct guid_storage<Runtime::AsyncOperationCompletedHandler<TResult>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::AsyncOperationCompletedHandler<TResult>>::value };
    };

    template <typename TProgress> struct guid_storage<Runtime::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::AsyncActionWithProgressCompletedHandler<TProgress>>::value };
    };

    template <typename TProgress> struct guid_storage<Runtime::AsyncActionProgressHandler<TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::AsyncActionProgressHandler<TProgress>>::value };
    };

    template <typename TResult, typename TProgress> struct guid_storage<Runtime::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::AsyncOperationProgressHandler<TResult, TProgress>>::value };
    };

    template <typename TResult, typename TProgress> struct guid_storage<Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>::value };
    };

    template <typename TResult> struct guid_storage<Runtime::IAsyncOperation<TResult>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IAsyncOperation<TResult>>::value };
    };

    template <typename TProgress> struct guid_storage<Runtime::IAsyncActionWithProgress<TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IAsyncActionWithProgress<TProgress>>::value };
    };

    template <typename TResult, typename TProgress> struct guid_storage<Runtime::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IAsyncOperationWithProgress<TResult, TProgress>>::value };
    };

    template <typename K> struct guid_storage<Runtime::IMapChangedEventArgs<K>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IMapChangedEventArgs<K>>::value };
    };

    template <typename T> struct guid_storage<Runtime::VectorChangedEventHandler<T>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::VectorChangedEventHandler<T>>::value };
    };

    template <typename K, typename V> struct guid_storage<Runtime::MapChangedEventHandler<K, V>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::MapChangedEventHandler<K, V>>::value };
    };

    template <typename T> struct guid_storage<Runtime::IIterator<T>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IIterator<T>>::value };
    };

    template <typename T> struct guid_storage<Runtime::IIterable<T>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IIterable<T>>::value };
    };

    template <typename T> struct guid_storage<Runtime::IVectorView<T>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IVectorView<T>>::value };
    };

    template <typename T> struct guid_storage<Runtime::IVector<T>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IVector<T>>::value };
    };

    template <typename T> struct guid_storage<Runtime::IObservableVector<T>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IObservableVector<T>>::value };
    };

    template <typename K, typename V> struct guid_storage<Runtime::IKeyValuePair<K, V>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IKeyValuePair<K, V>>::value };
    };

    template <typename K, typename V> struct guid_storage<Runtime::IMapView<K, V>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IMapView<K, V>>::value };
    };

    template <typename K, typename V> struct guid_storage<Runtime::IMap<K, V>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IMap<K, V>>::value };
    };

    template <typename K, typename V> struct guid_storage<Runtime::IObservableMap<K, V>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::IObservableMap<K, V>>::value };
    };

    template <typename T> struct guid_storage<Runtime::EventHandler<T>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::EventHandler<T>>::value };
    };

    template <typename TSender, typename TArgs> struct guid_storage<Runtime::TypedEventHandler<TSender, TArgs>>
    {
        static constexpr guid value{ pinterface_guid<Runtime::TypedEventHandler<TSender, TArgs>>::value };
    };

    template <> struct consume<Runtime::IActivationFactory>
    {
        template <typename D> using type = consume_IActivationFactory<D>;
    };

    template <> struct consume<Runtime::IVectorChangedEventArgs>
    {
        template <typename D> using type = consume_IVectorChangedEventArgs<D>;
    };

    template <typename TResult> struct consume<Runtime::IAsyncOperation<TResult>>
    {
        template <typename D> using type = consume_IAsyncOperation<D, TResult>;
    };

    template <typename TProgress> struct consume<Runtime::IAsyncActionWithProgress<TProgress>>
    {
        template <typename D> using type = consume_IAsyncActionWithProgress<D, TProgress>;
    };

    template <typename TResult, typename TProgress> struct consume<Runtime::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        template <typename D> using type = consume_IAsyncOperationWithProgress<D, TResult, TProgress>;
    };

    template <> struct consume<Runtime::IAsyncAction>
    {
        template <typename D> using type = consume_IAsyncAction<D>;
    };

    template <typename T> struct consume<Runtime::IReference<T>>
    {
        template <typename D> using type = consume_IReference<D, T>;
    };

    template <typename T> struct consume<Runtime::IReferenceArray<T>>
    {
        template <typename D> using type = consume_IReferenceArray<D, T>;
    };

    template <typename K> struct consume<Runtime::IMapChangedEventArgs<K>>
    {
        template <typename D> using type = consume_IMapChangedEventArgs<D, K>;
    };

    template <typename T> struct consume<Runtime::IIterator<T>>
    {
        template <typename D> using type = consume_IIterator<D, T>;
    };

    template <typename T> struct consume<Runtime::IIterable<T>>
    {
        template <typename D> using type = consume_IIterable<D, T>;
    };

    template <typename T> struct consume<Runtime::IVectorView<T>>
    {
        template <typename D> using type = consume_IVectorView<D, T>;
    };

    template <typename T> struct consume<Runtime::IVector<T>>
    {
        template <typename D> using type = consume_IVector<D, T>;
    };

    template <typename T> struct consume<Runtime::IObservableVector<T>>
    {
        template <typename D> using type = consume_IObservableVector<D, T>;
    };

    template <typename K, typename V> struct consume<Runtime::IKeyValuePair<K, V>>
    {
        template <typename D> using type = consume_IKeyValuePair<D, K, V>;
    };

    template <typename K, typename V> struct consume<Runtime::IMapView<K, V>>
    {
        template <typename D> using type = consume_IMapView<D, K, V>;
    };

    template <typename K, typename V> struct consume<Runtime::IMap<K, V>>
    {
        template <typename D> using type = consume_IMap<D, K, V>;
    };

    template <typename K, typename V> struct consume<Runtime::IObservableMap<K, V>>
    {
        template <typename D> using type = consume_IObservableMap<D, K, V>;
    };

    template <> struct name<Runtime::AsyncActionCompletedHandler>
    {
        static constexpr auto & value{ L"Runtime.AsyncActionCompletedHandler" };
    };

    template <typename TResult> struct name<Runtime::AsyncOperationCompletedHandler<TResult>>
    {
        static constexpr auto value{ zcombine(L"Runtime.AsyncOperationCompletedHandler`1<", name_v<TResult>, L">") };
    };

    template <typename TProgress> struct name<Runtime::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        static constexpr auto value{ zcombine(L"Runtime.AsyncActionWithProgressCompletedHandler`1<", name_v<TProgress>, L">") };
    };

    template <typename TProgress> struct name<Runtime::AsyncActionProgressHandler<TProgress>>
    {
        static constexpr auto value{ zcombine(L"Runtime.AsyncActionProgressHandler`1<", name_v<TProgress>, L">") };
    };

    template <typename TResult, typename TProgress> struct name<Runtime::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        static constexpr auto value{ zcombine(L"Runtime.AsyncOperationProgressHandler`2<", name_v<TResult>, L", ", name_v<TProgress>, L">") };
    };

    template <typename TResult, typename TProgress> struct name<Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        static constexpr auto value{ zcombine(L"Runtime.AsyncOperationWithProgressCompletedHandler`2<", name_v<TResult>, L", ", name_v<TProgress>, L">") };
    };

    template <> struct name<Runtime::IAsyncAction>
    {
        static constexpr auto & value{ L"Runtime.IAsyncAction" };
    };

    template <typename TResult> struct name<Runtime::IAsyncOperation<TResult>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IAsyncOperation`1<", name_v<TResult>, L">") };
    };

    template <typename TProgress> struct name<Runtime::IAsyncActionWithProgress<TProgress>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IAsyncActionWithProgress`1<", name_v<TProgress>, L">") };
    };

    template <typename TResult, typename TProgress> struct name<Runtime::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IAsyncOperationWithProgress`2<", name_v<TResult>, L", ", name_v<TProgress>, L">") };
    };

    template <> struct name<Runtime::IObject>
    {
        static constexpr auto & value{ L"Object" };
        static constexpr auto & data{ "cinterface(IObject)" };
    };

    template <> struct name<Runtime::IActivationFactory>
    {
        static constexpr auto & value{ L"Runtime.IActivationFactory" };
    };

    template <> struct name<IWeakReferenceSource>
    {
        static constexpr auto & value{ L"IWeakReferenceSource" };
    };

    template <typename T> struct name<Runtime::IReference<T>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IReference`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<Runtime::IReferenceArray<T>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IReferenceArray`1<", name_v<T>, L">") };
    };

    template <> struct name<Runtime::IVectorChangedEventArgs>
    {
        static constexpr auto & value{ L"Runtime.IVectorChangedEventArgs" };
    };

    template <typename K> struct name<Runtime::IMapChangedEventArgs<K>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IMapChangedEventArgs`1<", name_v<K>, L">") };
    };

    template <typename T> struct name<Runtime::VectorChangedEventHandler<T>>
    {
        static constexpr auto value{ zcombine(L"Runtime.VectorChangedEventHandler`1<", name_v<T>, L">") };
    };

    template <typename K, typename V> struct name<Runtime::MapChangedEventHandler<K, V>>
    {
        static constexpr auto value{ zcombine(L"Runtime.MapChangedEventHandler`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename T> struct name<Runtime::IIterator<T>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IIterator`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<Runtime::IIterable<T>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IIterable`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<Runtime::IVectorView<T>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IVectorView`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<Runtime::IVector<T>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IVector`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<Runtime::IObservableVector<T>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IObservableVector`1<", name_v<T>, L">") };
    };

    template <typename K, typename V> struct name<Runtime::IKeyValuePair<K, V>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IKeyValuePair`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename K, typename V> struct name<Runtime::IMapView<K, V>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IMapView`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename K, typename V> struct name<Runtime::IMap<K, V>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IMap`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename K, typename V> struct name<Runtime::IObservableMap<K, V>>
    {
        static constexpr auto value{ zcombine(L"Runtime.IObservableMap`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename T> struct name<Runtime::EventHandler<T>>
    {
        static constexpr auto value{ zcombine(L"Runtime.EventHandler`1<", name_v<T>, L">") };
    };

    template <typename TSender, typename TArgs> struct name<Runtime::TypedEventHandler<TSender, TArgs>>
    {
        static constexpr auto value{ zcombine(L"Runtime.TypedEventHandler`2<", name_v<TSender>, L", ", name_v<TArgs>, L">") };
    };

    template <> struct category<Runtime::AsyncActionCompletedHandler>
    {
        using type = delegate_category;
    };

    template <typename TResult> struct category<Runtime::AsyncOperationCompletedHandler<TResult>>
    {
        using type = pinterface_category<TResult>;
        static constexpr guid value{ 0xfcdcf02c, 0xe5d8, 0x4478,{ 0x91, 0x5a, 0x4d, 0x90, 0xb7, 0x4b, 0x83, 0xa5 } };
    };

    template <typename TProgress> struct category<Runtime::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        using type = pinterface_category<TProgress>;
        static constexpr guid value{ 0x9c029f91, 0xcc84, 0x44fd,{ 0xac, 0x26, 0x0a, 0x6c, 0x4e, 0x55, 0x52, 0x81 } };
    };

    template <typename TProgress> struct category<Runtime::AsyncActionProgressHandler<TProgress>>
    {
        using type = pinterface_category<TProgress>;
        static constexpr guid value{ 0x6d844858, 0x0cff, 0x4590,{ 0xae, 0x89, 0x95, 0xa5, 0xa5, 0xc8, 0xb4, 0xb8 } };
    };

    template <typename TResult, typename TProgress> struct category<Runtime::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        using type = pinterface_category<TResult, TProgress>;
        static constexpr guid value{ 0x55690902, 0x0aab, 0x421a,{ 0x87, 0x78, 0xf8, 0xce, 0x50, 0x26, 0xd7, 0x58 } };
    };

    template <typename TResult, typename TProgress> struct category<Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        using type = pinterface_category<TResult, TProgress>;
        static constexpr guid value{ 0xe85df41d, 0x6aa7, 0x46e3,{ 0xa8, 0xe2, 0xf0, 0x09, 0xd8, 0x40, 0xc6, 0x27 } };
    };

    template <> struct category<Runtime::IAsyncAction>
    {
        using type = interface_category;
    };

    template <typename TResult> struct category<Runtime::IAsyncOperation<TResult>>
    {
        using type = pinterface_category<TResult>;
        static constexpr guid value{ 0xc7145ad9, 0x37c, 0x4f2f,{ 0xaa,0xf0,0x42,0xab,0x6c,0xd6,0x4e,0x5e } };
    };

    template <typename TProgress> struct category<Runtime::IAsyncActionWithProgress<TProgress>>
    {
        using type = pinterface_category<TProgress>;
        static constexpr guid value{ 0x76458dd9, 0x3bf1, 0x4f77,{ 0xae,0xd,0x1f,0x66,0x29,0xbc,0x54,0x1a } };
    };

    template <typename TResult, typename TProgress> struct category<Runtime::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        using type = pinterface_category<TResult, TProgress>;
        static constexpr guid value{ 0xc06f3afa, 0x66a1, 0x4f10,{ 0xaa,0x55,0xee,0xeb,0x6f,0xe8,0x25,0xc } };
    };

    template <> struct category<Runtime::IUnknown>
    {
        using type = interface_category;
    };

    template <> struct category<Runtime::IObject>
    {
        using type = basic_category;
    };

    template <> struct category<Runtime::IActivationFactory>
    {
        using type = interface_category;
    };

    template <typename T> struct category<Runtime::IReference<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x61c17706, 0x2d65, 0x11e0,{ 0x9a, 0xe8, 0xd4, 0x85, 0x64, 0x01, 0x54, 0x72 } };
    };

    template <typename T> struct category<Runtime::IReferenceArray<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x61c17707, 0x2d65, 0x11e0,{ 0x9a, 0xe8, 0xd4, 0x85, 0x64, 0x01, 0x54, 0x72 } };
    };

    template <> struct category<Runtime::IVectorChangedEventArgs>
    {
        using type = interface_category;
    };

    template <typename K> struct category<Runtime::IMapChangedEventArgs<K>>
    {
        using type = pinterface_category<K>;
        static constexpr guid value{ 0x9939f4df, 0x050a, 0x4c0f,{ 0xaa, 0x60, 0x77, 0x07, 0x5f, 0x9c, 0x47, 0x77 } };
    };

    template <typename T> struct category<Runtime::VectorChangedEventHandler<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x0c051752, 0x9fbf, 0x4c70,{ 0xaa, 0x0c, 0x0e, 0x4c, 0x82, 0xd9, 0xa7, 0x61 } };
    };

    template <typename K, typename V> struct category<Runtime::MapChangedEventHandler<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0x179517f3, 0x94ee, 0x41f8,{ 0xbd, 0xdc, 0x76, 0x8a, 0x89, 0x55, 0x44, 0xf3 } };
    };

    template <typename T> struct category<Runtime::IIterator<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x6a79e863, 0x4300, 0x459a,{ 0x99, 0x66, 0xcb, 0xb6, 0x60, 0x96, 0x3e, 0xe1 } };
    };

    template <typename T> struct category<Runtime::IIterable<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0xfaa585ea, 0x6214, 0x4217,{ 0xaf, 0xda, 0x7f, 0x46, 0xde, 0x58, 0x69, 0xb3 } };
    };

    template <typename T> struct category<Runtime::IVectorView<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0xbbe1fa4c, 0xb0e3, 0x4583,{ 0xba, 0xef, 0x1f, 0x1b, 0x2e, 0x48, 0x3e, 0x56 } };
    };

    template <typename T> struct category<Runtime::IVector<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x913337e9, 0x11a1, 0x4345,{ 0xa3, 0xa2, 0x4e, 0x7f, 0x95, 0x6e, 0x22, 0x2d } };
    };

    template <typename T> struct category<Runtime::IObservableVector<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x5917eb53, 0x50b4, 0x4a0d,{ 0xb3, 0x09, 0x65, 0x86, 0x2b, 0x3f, 0x1d, 0xbc } };
    };

    template <typename K, typename V> struct category<Runtime::IKeyValuePair<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0x02b51929, 0xc1c4, 0x4a7e,{ 0x89, 0x40, 0x03, 0x12, 0xb5, 0xc1, 0x85, 0x00 } };
    };

    template <typename K, typename V> struct category<Runtime::IMapView<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0xe480ce40, 0xa338, 0x4ada,{ 0xad, 0xcf, 0x27, 0x22, 0x72, 0xe4, 0x8c, 0xb9 } };
    };

    template <typename K, typename V> struct category<Runtime::IMap<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0x3c2925fe, 0x8519, 0x45c1,{ 0xaa, 0x79, 0x19, 0x7b, 0x67, 0x18, 0xc1, 0xc1 } };
    };

    template <typename K, typename V> struct category<Runtime::IObservableMap<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0x65df2bf5, 0xbf39, 0x41b5,{ 0xae, 0xbc, 0x5a, 0x9d, 0x86, 0x5e, 0x47, 0x2b } };
    };

    template <typename T> struct category<Runtime::EventHandler<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x9de1c535, 0x6ae1, 0x11e0,{ 0x84, 0xe1, 0x18, 0xa9, 0x05, 0xbc, 0xc5, 0x3f } };
    };

    template <typename TSender, typename TArgs> struct category<Runtime::TypedEventHandler<TSender, TArgs>>
    {
        using type = pinterface_category<TSender, TArgs>;
        static constexpr guid value{ 0x9de1c534, 0x6ae1, 0x11e0,{ 0x84, 0xe1, 0x18, 0xa9, 0x05, 0xbc, 0xc5, 0x3f } };
    };
}
