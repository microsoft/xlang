
namespace winrt::impl
{
    template <> struct guid_storage<Windows::Foundation::IUnknown>
    {
        static constexpr guid value{ 0x00000000,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<Windows::Foundation::IInspectable>
    {
        static constexpr guid value{ 0xAF86E2E0,0xB12D,0x4C6A,{ 0x9C,0x5A,0xD7,0xAA,0x65,0x10,0x1E,0x90 } };
    };

    template <> struct guid_storage<Windows::Foundation::IActivationFactory>
    {
        static constexpr guid value{ 0x00000035,0x0000,0x0000,{ 0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<IAgileObject>
    {
        static constexpr guid value{ 0x94EA2B94,0xE9CC,0x49E0,{ 0xC0,0xFF,0xEE,0x64,0xCA,0x8F,0x5B,0x90 } };
    };

    template <> struct guid_storage<IMarshal>
    {
        static constexpr guid value{ 0x00000003,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<IStaticLifetime>
    {
        static constexpr guid value{ 0x17b0e613,0x942a,0x422d,{ 0x90,0x4c,0xf9,0x0d,0xc7,0x1a,0x7d,0xae } };
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

    template <> struct guid_storage<IContextCallback>
    {
        static constexpr guid value{ 0x000001da,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<IServerSecurity>
    {
        static constexpr guid value{ 0x0000013E,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<ICallbackWithNoReentrancyToApplicationSTA>
    {
        static constexpr guid value{ 0x0A299774,0x3E4E,0xFC42,{ 0x1D,0x9D,0x72,0xCE,0xE1,0x05,0xCA,0x57 } };
    };

    template <> struct guid_storage<IBufferByteAccess>
    {
        static constexpr guid value{ 0x905a0fef,0xbc53,0x11df,{ 0x8c,0x49,0x00,0x1e,0x4f,0xc6,0x86,0xda } };
    };

    template <> struct guid_storage<wfc::IVectorChangedEventArgs>
    {
        static constexpr guid value{ 0x575933DF,0x34FE,0x4480,{ 0xAF,0x15,0x07,0x69,0x1F,0x3D,0x5D,0x9B } };
    };

    template <> struct guid_storage<Windows::Foundation::IAsyncInfo>
    {
        static constexpr guid value{ 0x00000036,0x0000,0x0000,{ 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 } };
    };

    template <> struct guid_storage<Windows::Foundation::AsyncActionCompletedHandler>
    {
        static constexpr guid value{ 0xA4ED5C81,0x76C9,0x40BD,{ 0x8B,0xE6,0xB1,0xD9,0x0F,0xB2,0x0A,0xE7 } };
    };

    template <> struct guid_storage<Windows::Foundation::IAsyncAction>
    {
        static constexpr guid value{ 0x5A648006,0x843A,0x4DA9,{ 0x86,0x5B,0x9D,0x26,0xE5,0xDF,0xAD,0x7B } };
    };

    template <typename T> struct guid_storage<Windows::Foundation::IReference<T>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::IReference<T>>::value };
    };

    template <typename T> struct guid_storage<Windows::Foundation::IReferenceArray<T>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::IReferenceArray<T>>::value };
    };

    template <typename TResult> struct guid_storage<Windows::Foundation::AsyncOperationCompletedHandler<TResult>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::AsyncOperationCompletedHandler<TResult>>::value };
    };

    template <typename TProgress> struct guid_storage<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>>::value };
    };

    template <typename TProgress> struct guid_storage<Windows::Foundation::AsyncActionProgressHandler<TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::AsyncActionProgressHandler<TProgress>>::value };
    };

    template <typename TResult, typename TProgress> struct guid_storage<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>>::value };
    };

    template <typename TResult, typename TProgress> struct guid_storage<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>::value };
    };

    template <typename TResult> struct guid_storage<Windows::Foundation::IAsyncOperation<TResult>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::IAsyncOperation<TResult>>::value };
    };

    template <typename TProgress> struct guid_storage<Windows::Foundation::IAsyncActionWithProgress<TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::IAsyncActionWithProgress<TProgress>>::value };
    };

    template <typename TResult, typename TProgress> struct guid_storage<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>>::value };
    };

    template <typename K> struct guid_storage<wfc::IMapChangedEventArgs<K>>
    {
        static constexpr guid value{ pinterface_guid<wfc::IMapChangedEventArgs<K>>::value };
    };

    template <typename T> struct guid_storage<wfc::VectorChangedEventHandler<T>>
    {
        static constexpr guid value{ pinterface_guid<wfc::VectorChangedEventHandler<T>>::value };
    };

    template <typename K, typename V> struct guid_storage<wfc::MapChangedEventHandler<K, V>>
    {
        static constexpr guid value{ pinterface_guid<wfc::MapChangedEventHandler<K, V>>::value };
    };

    template <typename T> struct guid_storage<wfc::IIterator<T>>
    {
        static constexpr guid value{ pinterface_guid<wfc::IIterator<T>>::value };
    };

    template <typename T> struct guid_storage<wfc::IIterable<T>>
    {
        static constexpr guid value{ pinterface_guid<wfc::IIterable<T>>::value };
    };

    template <typename T> struct guid_storage<wfc::IVectorView<T>>
    {
        static constexpr guid value{ pinterface_guid<wfc::IVectorView<T>>::value };
    };

    template <typename T> struct guid_storage<wfc::IVector<T>>
    {
        static constexpr guid value{ pinterface_guid<wfc::IVector<T>>::value };
    };

    template <typename K, typename V> struct guid_storage<wfc::IKeyValuePair<K, V>>
    {
        static constexpr guid value{ pinterface_guid<wfc::IKeyValuePair<K, V>>::value };
    };

    template <typename K, typename V> struct guid_storage<wfc::IMapView<K, V>>
    {
        static constexpr guid value{ pinterface_guid<wfc::IMapView<K, V>>::value };
    };

    template <typename K, typename V> struct guid_storage<wfc::IMap<K, V>>
    {
        static constexpr guid value{ pinterface_guid<wfc::IMap<K, V>>::value };
    };

    template <typename K, typename V> struct guid_storage<wfc::IObservableMap<K, V>>
    {
        static constexpr guid value{ pinterface_guid<wfc::IObservableMap<K, V>>::value };
    };

    template <typename T> struct guid_storage<Windows::Foundation::EventHandler<T>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::EventHandler<T>>::value };
    };

    template <typename TSender, typename TArgs> struct guid_storage<Windows::Foundation::TypedEventHandler<TSender, TArgs>>
    {
        static constexpr guid value{ pinterface_guid<Windows::Foundation::TypedEventHandler<TSender, TArgs>>::value };
    };

    template <> struct consume<Windows::Foundation::IActivationFactory>
    {
        template <typename D> using type = consume_IActivationFactory<D>;
    };

    template <> struct consume<wfc::IVectorChangedEventArgs>
    {
        template <typename D> using type = consume_IVectorChangedEventArgs<D>;
    };

    template <typename TResult> struct consume<Windows::Foundation::IAsyncOperation<TResult>>
    {
        template <typename D> using type = consume_IAsyncOperation<D, TResult>;
    };

    template <typename TProgress> struct consume<Windows::Foundation::IAsyncActionWithProgress<TProgress>>
    {
        template <typename D> using type = consume_IAsyncActionWithProgress<D, TProgress>;
    };

    template <typename TResult, typename TProgress> struct consume<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        template <typename D> using type = consume_IAsyncOperationWithProgress<D, TResult, TProgress>;
    };

    template <> struct consume<Windows::Foundation::IAsyncInfo>
    {
        template <typename D> using type = consume_IAsyncInfo<D>;
    };

    template <> struct consume<Windows::Foundation::IAsyncAction>
    {
        template <typename D> using type = consume_IAsyncAction<D>;
    };

    template <typename T> struct consume<Windows::Foundation::IReference<T>>
    {
        template <typename D> using type = consume_IReference<D, T>;
    };

    template <typename T> struct consume<Windows::Foundation::IReferenceArray<T>>
    {
        template <typename D> using type = consume_IReferenceArray<D, T>;
    };

    template <typename K> struct consume<wfc::IMapChangedEventArgs<K>>
    {
        template <typename D> using type = consume_IMapChangedEventArgs<D, K>;
    };

    template <typename T> struct consume<wfc::IIterator<T>>
    {
        template <typename D> using type = consume_IIterator<D, T>;
    };

    template <typename T> struct consume<wfc::IIterable<T>>
    {
        template <typename D> using type = consume_IIterable<D, T>;
    };

    template <typename T> struct consume<wfc::IVectorView<T>>
    {
        template <typename D> using type = consume_IVectorView<D, T>;
    };

    template <typename T> struct consume<wfc::IVector<T>>
    {
        template <typename D> using type = consume_IVector<D, T>;
    };

    template <typename K, typename V> struct consume<wfc::IKeyValuePair<K, V>>
    {
        template <typename D> using type = consume_IKeyValuePair<D, K, V>;
    };

    template <typename K, typename V> struct consume<wfc::IMapView<K, V>>
    {
        template <typename D> using type = consume_IMapView<D, K, V>;
    };

    template <typename K, typename V> struct consume<wfc::IMap<K, V>>
    {
        template <typename D> using type = consume_IMap<D, K, V>;
    };

    template <typename K, typename V> struct consume<wfc::IObservableMap<K, V>>
    {
        template <typename D> using type = consume_IObservableMap<D, K, V>;
    };

    template <> struct name<Windows::Foundation::AsyncActionCompletedHandler>
    {
        static constexpr auto & value{ L"Windows.Foundation.AsyncActionCompletedHandler" };
    };

    template <typename TResult> struct name<Windows::Foundation::AsyncOperationCompletedHandler<TResult>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.AsyncOperationCompletedHandler`1<", name_v<TResult>, L">") };
    };

    template <typename TProgress> struct name<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.AsyncActionWithProgressCompletedHandler`1<", name_v<TProgress>, L">") };
    };

    template <typename TProgress> struct name<Windows::Foundation::AsyncActionProgressHandler<TProgress>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.AsyncActionProgressHandler`1<", name_v<TProgress>, L">") };
    };

    template <typename TResult, typename TProgress> struct name<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.AsyncOperationProgressHandler`2<", name_v<TResult>, L", ", name_v<TProgress>, L">") };
    };

    template <typename TResult, typename TProgress> struct name<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.AsyncOperationWithProgressCompletedHandler`2<", name_v<TResult>, L", ", name_v<TProgress>, L">") };
    };

    template <> struct name<Windows::Foundation::IAsyncInfo>
    {
        static constexpr auto & value{ L"Windows.Foundation.IAsyncInfo" };
    };

    template <> struct name<Windows::Foundation::IAsyncAction>
    {
        static constexpr auto & value{ L"Windows.Foundation.IAsyncAction" };
    };

    template <typename TResult> struct name<Windows::Foundation::IAsyncOperation<TResult>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.IAsyncOperation`1<", name_v<TResult>, L">") };
    };

    template <typename TProgress> struct name<Windows::Foundation::IAsyncActionWithProgress<TProgress>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.IAsyncActionWithProgress`1<", name_v<TProgress>, L">") };
    };

    template <typename TResult, typename TProgress> struct name<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.IAsyncOperationWithProgress`2<", name_v<TResult>, L", ", name_v<TProgress>, L">") };
    };

    template <> struct name<Windows::Foundation::IInspectable>
    {
        static constexpr auto & value{ L"Object" };
        static constexpr auto & data{ "cinterface(IInspectable)" };
    };

    template <> struct name<Windows::Foundation::IActivationFactory>
    {
        static constexpr auto & value{ L"Windows.Foundation.IActivationFactory" };
    };

    template <> struct name<IAgileObject>
    {
        static constexpr auto & value{ L"IAgileObject" };
    };

    template <> struct name<IWeakReferenceSource>
    {
        static constexpr auto & value{ L"IWeakReferenceSource" };
    };

    template <typename T> struct name<Windows::Foundation::IReference<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.IReference`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<Windows::Foundation::IReferenceArray<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.IReferenceArray`1<", name_v<T>, L">") };
    };

    template <> struct name<wfc::IVectorChangedEventArgs>
    {
        static constexpr auto & value{ L"Windows.Foundation.Collections.IVectorChangedEventArgs" };
    };

    template <typename K> struct name<wfc::IMapChangedEventArgs<K>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.IMapChangedEventArgs`1<", name_v<K>, L">") };
    };

    template <typename T> struct name<wfc::VectorChangedEventHandler<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.VectorChangedEventHandler`1<", name_v<T>, L">") };
    };

    template <typename K, typename V> struct name<wfc::MapChangedEventHandler<K, V>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.MapChangedEventHandler`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename T> struct name<wfc::IIterator<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.IIterator`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<wfc::IIterable<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.IIterable`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<wfc::IVectorView<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.IVectorView`1<", name_v<T>, L">") };
    };

    template <typename T> struct name<wfc::IVector<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.IVector`1<", name_v<T>, L">") };
    };

    template <typename K, typename V> struct name<wfc::IKeyValuePair<K, V>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.IKeyValuePair`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename K, typename V> struct name<wfc::IMapView<K, V>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.IMapView`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename K, typename V> struct name<wfc::IMap<K, V>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.IMap`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename K, typename V> struct name<wfc::IObservableMap<K, V>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.Collections.IObservableMap`2<", name_v<K>, L", ", name_v<V>, L">") };
    };

    template <typename T> struct name<Windows::Foundation::EventHandler<T>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.EventHandler`1<", name_v<T>, L">") };
    };

    template <typename TSender, typename TArgs> struct name<Windows::Foundation::TypedEventHandler<TSender, TArgs>>
    {
        static constexpr auto value{ zcombine(L"Windows.Foundation.TypedEventHandler`2<", name_v<TSender>, L", ", name_v<TArgs>, L">") };
    };

    template <> struct category<Windows::Foundation::AsyncActionCompletedHandler>
    {
        using type = delegate_category;
    };

    template <typename TResult> struct category<Windows::Foundation::AsyncOperationCompletedHandler<TResult>>
    {
        using type = pinterface_category<TResult>;
        static constexpr guid value{ 0xfcdcf02c, 0xe5d8, 0x4478,{ 0x91, 0x5a, 0x4d, 0x90, 0xb7, 0x4b, 0x83, 0xa5 } };
    };

    template <typename TProgress> struct category<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        using type = pinterface_category<TProgress>;
        static constexpr guid value{ 0x9c029f91, 0xcc84, 0x44fd,{ 0xac, 0x26, 0x0a, 0x6c, 0x4e, 0x55, 0x52, 0x81 } };
    };

    template <typename TProgress> struct category<Windows::Foundation::AsyncActionProgressHandler<TProgress>>
    {
        using type = pinterface_category<TProgress>;
        static constexpr guid value{ 0x6d844858, 0x0cff, 0x4590,{ 0xae, 0x89, 0x95, 0xa5, 0xa5, 0xc8, 0xb4, 0xb8 } };
    };

    template <typename TResult, typename TProgress> struct category<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        using type = pinterface_category<TResult, TProgress>;
        static constexpr guid value{ 0x55690902, 0x0aab, 0x421a,{ 0x87, 0x78, 0xf8, 0xce, 0x50, 0x26, 0xd7, 0x58 } };
    };

    template <typename TResult, typename TProgress> struct category<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        using type = pinterface_category<TResult, TProgress>;
        static constexpr guid value{ 0xe85df41d, 0x6aa7, 0x46e3,{ 0xa8, 0xe2, 0xf0, 0x09, 0xd8, 0x40, 0xc6, 0x27 } };
    };

    template <> struct category<Windows::Foundation::IAsyncInfo>
    {
        using type = interface_category;
    };

    template <> struct category<Windows::Foundation::IAsyncAction>
    {
        using type = interface_category;
    };

    template <typename TResult> struct category<Windows::Foundation::IAsyncOperation<TResult>>
    {
        using type = pinterface_category<TResult>;
        static constexpr guid value{ 0x9fc2b0bb, 0xe446, 0x44e2,{ 0xaa, 0x61, 0x9c, 0xab, 0x8f, 0x63, 0x6a, 0xf2 } };
    };

    template <typename TProgress> struct category<Windows::Foundation::IAsyncActionWithProgress<TProgress>>
    {
        using type = pinterface_category<TProgress>;
        static constexpr guid value{ 0x1f6db258, 0xe803, 0x48a1,{ 0x95, 0x46, 0xeb, 0x73, 0x53, 0x39, 0x88, 0x84 } };
    };

    template <typename TResult, typename TProgress> struct category<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        using type = pinterface_category<TResult, TProgress>;
        static constexpr guid value{ 0xb5d036d7, 0xe297, 0x498f,{ 0xba, 0x60, 0x02, 0x89, 0xe7, 0x6e, 0x23, 0xdd } };
    };

    template <> struct category<Windows::Foundation::IUnknown>
    {
        using type = interface_category;
    };

    template <> struct category<Windows::Foundation::IInspectable>
    {
        using type = basic_category;
    };

    template <> struct category<Windows::Foundation::IActivationFactory>
    {
        using type = interface_category;
    };

    template <typename T> struct category<Windows::Foundation::IReference<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x61c17706, 0x2d65, 0x11e0,{ 0x9a, 0xe8, 0xd4, 0x85, 0x64, 0x01, 0x54, 0x72 } };
    };

    template <typename T> struct category<Windows::Foundation::IReferenceArray<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x61c17707, 0x2d65, 0x11e0,{ 0x9a, 0xe8, 0xd4, 0x85, 0x64, 0x01, 0x54, 0x72 } };
    };

    template <> struct category<wfc::IVectorChangedEventArgs>
    {
        using type = interface_category;
    };

    template <typename K> struct category<wfc::IMapChangedEventArgs<K>>
    {
        using type = pinterface_category<K>;
        static constexpr guid value{ 0x9939f4df, 0x050a, 0x4c0f,{ 0xaa, 0x60, 0x77, 0x07, 0x5f, 0x9c, 0x47, 0x77 } };
    };

    template <typename T> struct category<wfc::VectorChangedEventHandler<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x0c051752, 0x9fbf, 0x4c70,{ 0xaa, 0x0c, 0x0e, 0x4c, 0x82, 0xd9, 0xa7, 0x61 } };
    };

    template <typename K, typename V> struct category<wfc::MapChangedEventHandler<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0x179517f3, 0x94ee, 0x41f8,{ 0xbd, 0xdc, 0x76, 0x8a, 0x89, 0x55, 0x44, 0xf3 } };
    };

    template <typename T> struct category<wfc::IIterator<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x6a79e863, 0x4300, 0x459a,{ 0x99, 0x66, 0xcb, 0xb6, 0x60, 0x96, 0x3e, 0xe1 } };
    };

    template <typename T> struct category<wfc::IIterable<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0xfaa585ea, 0x6214, 0x4217,{ 0xaf, 0xda, 0x7f, 0x46, 0xde, 0x58, 0x69, 0xb3 } };
    };

    template <typename T> struct category<wfc::IVectorView<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0xbbe1fa4c, 0xb0e3, 0x4583,{ 0xba, 0xef, 0x1f, 0x1b, 0x2e, 0x48, 0x3e, 0x56 } };
    };

    template <typename T> struct category<wfc::IVector<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x913337e9, 0x11a1, 0x4345,{ 0xa3, 0xa2, 0x4e, 0x7f, 0x95, 0x6e, 0x22, 0x2d } };
    };

    template <typename K, typename V> struct category<wfc::IKeyValuePair<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0x02b51929, 0xc1c4, 0x4a7e,{ 0x89, 0x40, 0x03, 0x12, 0xb5, 0xc1, 0x85, 0x00 } };
    };

    template <typename K, typename V> struct category<wfc::IMapView<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0xe480ce40, 0xa338, 0x4ada,{ 0xad, 0xcf, 0x27, 0x22, 0x72, 0xe4, 0x8c, 0xb9 } };
    };

    template <typename K, typename V> struct category<wfc::IMap<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0x3c2925fe, 0x8519, 0x45c1,{ 0xaa, 0x79, 0x19, 0x7b, 0x67, 0x18, 0xc1, 0xc1 } };
    };

    template <typename K, typename V> struct category<wfc::IObservableMap<K, V>>
    {
        using type = pinterface_category<K, V>;
        static constexpr guid value{ 0x65df2bf5, 0xbf39, 0x41b5,{ 0xae, 0xbc, 0x5a, 0x9d, 0x86, 0x5e, 0x47, 0x2b } };
    };

    template <typename T> struct category<Windows::Foundation::EventHandler<T>>
    {
        using type = pinterface_category<T>;
        static constexpr guid value{ 0x9de1c535, 0x6ae1, 0x11e0,{ 0x84, 0xe1, 0x18, 0xa9, 0x05, 0xbc, 0xc5, 0x3f } };
    };

    template <typename TSender, typename TArgs> struct category<Windows::Foundation::TypedEventHandler<TSender, TArgs>>
    {
        using type = pinterface_category<TSender, TArgs>;
        static constexpr guid value{ 0x9de1c534, 0x6ae1, 0x11e0,{ 0x84, 0xe1, 0x18, 0xa9, 0x05, 0xbc, 0xc5, 0x3f } };
    };
}
