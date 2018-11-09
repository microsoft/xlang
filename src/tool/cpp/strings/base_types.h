
WINRT_EXPORT namespace winrt::Windows::Foundation
{
    struct WINRT_EBO IAsyncInfo :
        IInspectable,
        impl::consume_t<IAsyncInfo>
    {
        IAsyncInfo(std::nullptr_t = nullptr) noexcept {}
        IAsyncInfo(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    struct WINRT_EBO IAsyncAction :
        IInspectable,
        impl::consume_t<IAsyncAction>,
        impl::require<IAsyncAction, IAsyncInfo>
    {
        IAsyncAction(std::nullptr_t = nullptr) noexcept {}
        IAsyncAction(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename TProgress>
    struct WINRT_EBO IAsyncActionWithProgress :
        IInspectable,
        impl::consume_t<IAsyncActionWithProgress<TProgress>>,
        impl::require<IAsyncActionWithProgress<TProgress>, IAsyncInfo>
    {
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        IAsyncActionWithProgress(std::nullptr_t = nullptr) noexcept {}
        IAsyncActionWithProgress(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename TResult>
    struct WINRT_EBO IAsyncOperation :
        IInspectable,
        impl::consume_t<IAsyncOperation<TResult>>,
        impl::require<IAsyncOperation<TResult>, IAsyncInfo>
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        IAsyncOperation(std::nullptr_t = nullptr) noexcept {}
        IAsyncOperation(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename TResult, typename TProgress>
    struct WINRT_EBO IAsyncOperationWithProgress :
        IInspectable,
        impl::consume_t<IAsyncOperationWithProgress<TResult, TProgress>>,
        impl::require<IAsyncOperationWithProgress<TResult, TProgress>, IAsyncInfo>
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        IAsyncOperationWithProgress(std::nullptr_t = nullptr) noexcept {}
        IAsyncOperationWithProgress(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename T>
    struct WINRT_EBO EventHandler : IUnknown
    {
        static_assert(impl::has_category_v<T>, "T must be WinRT type.");
        EventHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        EventHandler(L handler) :
            EventHandler(impl::make_delegate<EventHandler<T>>(std::forward<L>(handler)))
        {}

        template <typename F> EventHandler(F* handler) :
            EventHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> EventHandler(O* object, M method) :
            EventHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> EventHandler(com_ptr<O>&& object, M method) :
            EventHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> EventHandler(weak_ref<O>&& object, M method) :
            EventHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IInspectable const& sender, T const& args) const
        {
            check_hresult((*(impl::abi_t<EventHandler<T>>**)this)->Invoke(get_abi(sender), get_abi(args)));
        }
    };

    template <typename TSender, typename TArgs>
    struct WINRT_EBO TypedEventHandler : IUnknown
    {
        static_assert(impl::has_category_v<TSender>, "TSender must be WinRT type.");
        static_assert(impl::has_category_v<TArgs>, "TArgs must be WinRT type.");
        TypedEventHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        TypedEventHandler(L handler) :
            TypedEventHandler(impl::make_delegate<TypedEventHandler<TSender, TArgs>>(std::forward<L>(handler)))
        {}

        template <typename F> TypedEventHandler(F* handler) :
            TypedEventHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> TypedEventHandler(O* object, M method) :
            TypedEventHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> TypedEventHandler(com_ptr<O>&& object, M method) :
            TypedEventHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> TypedEventHandler(weak_ref<O>&& object, M method) :
            TypedEventHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(TSender const& sender, TArgs const& args) const
        {
            check_hresult((*(impl::abi_t<TypedEventHandler<TSender, TArgs>>**)this)->Invoke(get_abi(sender), get_abi(args)));
        }
    };

    struct AsyncActionCompletedHandler : IUnknown
    {
        AsyncActionCompletedHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        AsyncActionCompletedHandler(L handler) :
            AsyncActionCompletedHandler(impl::make_delegate<AsyncActionCompletedHandler>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncActionCompletedHandler(F* handler) :
            AsyncActionCompletedHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncActionCompletedHandler(O* object, M method) :
            AsyncActionCompletedHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionCompletedHandler(com_ptr<O>&& object, M method) :
            AsyncActionCompletedHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionCompletedHandler(weak_ref<O>&& object, M method) :
            AsyncActionCompletedHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncAction const& sender, AsyncStatus args) const
        {
            check_hresult((*(impl::abi_t<AsyncActionCompletedHandler>**)this)->Invoke(get_abi(sender), args));
        }
    };

    template <typename TProgress>
    struct WINRT_EBO AsyncActionProgressHandler : IUnknown
    {
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        AsyncActionProgressHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        AsyncActionProgressHandler(L handler) :
            AsyncActionProgressHandler(impl::make_delegate<AsyncActionProgressHandler<TProgress>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncActionProgressHandler(F* handler) :
            AsyncActionProgressHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncActionProgressHandler(O* object, M method) :
            AsyncActionProgressHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionProgressHandler(com_ptr<O>&& object, M method) :
            AsyncActionProgressHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionProgressHandler(weak_ref<O>&& object, M method) :
            AsyncActionProgressHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncActionWithProgress<TProgress> const& sender, TProgress const& args) const
        {
            check_hresult((*(impl::abi_t<AsyncActionProgressHandler<TProgress>>**)this)->Invoke(get_abi(sender), get_abi(args)));
        }
    };

    template <typename TProgress>
    struct WINRT_EBO AsyncActionWithProgressCompletedHandler : IUnknown
    {
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        AsyncActionWithProgressCompletedHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        AsyncActionWithProgressCompletedHandler(L handler) :
            AsyncActionWithProgressCompletedHandler(impl::make_delegate<AsyncActionWithProgressCompletedHandler<TProgress>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncActionWithProgressCompletedHandler(F* handler) :
            AsyncActionWithProgressCompletedHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncActionWithProgressCompletedHandler(O* object, M method) :
            AsyncActionWithProgressCompletedHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionWithProgressCompletedHandler(com_ptr<O>&& object, M method) :
            AsyncActionWithProgressCompletedHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionWithProgressCompletedHandler(weak_ref<O>&& object, M method) :
            AsyncActionWithProgressCompletedHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncActionWithProgress<TProgress> const& sender, AsyncStatus const args) const
        {
            check_hresult((*(impl::abi_t<AsyncActionWithProgressCompletedHandler<TProgress>>**)this)->Invoke(get_abi(sender), args));
        }
    };

    template <typename TResult, typename TProgress>
    struct WINRT_EBO AsyncOperationProgressHandler : IUnknown
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        AsyncOperationProgressHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        AsyncOperationProgressHandler(L handler) :
            AsyncOperationProgressHandler(impl::make_delegate<AsyncOperationProgressHandler<TResult, TProgress>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncOperationProgressHandler(F* handler) :
            AsyncOperationProgressHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncOperationProgressHandler(O* object, M method) :
            AsyncOperationProgressHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationProgressHandler(com_ptr<O>&& object, M method) :
            AsyncOperationProgressHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationProgressHandler(weak_ref<O>&& object, M method) :
            AsyncOperationProgressHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncOperationWithProgress<TResult, TProgress> const& sender, TProgress const& args) const
        {
            check_hresult((*(impl::abi_t<AsyncOperationProgressHandler<TResult, TProgress>>**)this)->Invoke(get_abi(sender), get_abi(args)));
        }
    };

    template <typename TResult, typename TProgress>
    struct WINRT_EBO AsyncOperationWithProgressCompletedHandler : IUnknown
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        AsyncOperationWithProgressCompletedHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        AsyncOperationWithProgressCompletedHandler(L handler) :
            AsyncOperationWithProgressCompletedHandler(impl::make_delegate<AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncOperationWithProgressCompletedHandler(F* handler) :
            AsyncOperationWithProgressCompletedHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncOperationWithProgressCompletedHandler(O* object, M method) :
            AsyncOperationWithProgressCompletedHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationWithProgressCompletedHandler(com_ptr<O>&& object, M method) :
            AsyncOperationWithProgressCompletedHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationWithProgressCompletedHandler(weak_ref<O>&& object, M method) :
            AsyncOperationWithProgressCompletedHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncOperationWithProgress<TResult, TProgress> const& sender, AsyncStatus const args) const
        {
            check_hresult((*(impl::abi_t<AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>**)this)->Invoke(get_abi(sender), args));
        }
    };

    template <typename TResult>
    struct WINRT_EBO AsyncOperationCompletedHandler : IUnknown
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        AsyncOperationCompletedHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        AsyncOperationCompletedHandler(L handler) :
            AsyncOperationCompletedHandler(impl::make_delegate<AsyncOperationCompletedHandler<TResult>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncOperationCompletedHandler(F* handler) :
            AsyncOperationCompletedHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncOperationCompletedHandler(O* object, M method) :
            AsyncOperationCompletedHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationCompletedHandler(com_ptr<O>&& object, M method) :
            AsyncOperationCompletedHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationCompletedHandler(weak_ref<O>&& object, M method) :
            AsyncOperationCompletedHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncOperation<TResult> const& sender, AsyncStatus args) const
        {
            check_hresult((*(impl::abi_t<AsyncOperationCompletedHandler<TResult>>**)this)->Invoke(get_abi(sender), args));
        }
    };
}

WINRT_EXPORT namespace winrt::Windows::Foundation::Collections
{
    template <typename K>
    struct WINRT_EBO IMapChangedEventArgs :
        IInspectable,
        impl::consume_t<IMapChangedEventArgs<K>>
    {
        static_assert(impl::has_category_v<K>, "K must be WinRT type.");
        IMapChangedEventArgs(std::nullptr_t = nullptr) noexcept {}
        IMapChangedEventArgs(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename T>
    struct WINRT_EBO IIterator :
        IInspectable,
        impl::consume_t<IIterator<T>>
    {
        static_assert(impl::has_category_v<T>, "T must be WinRT type.");
        IIterator(std::nullptr_t = nullptr) noexcept {}
        IIterator(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}

        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T * ;
        using reference = T & ;
    };

    template <typename T>
    struct WINRT_EBO IIterable :
        IInspectable,
        impl::consume_t<IIterable<T>>
    {
        static_assert(impl::has_category_v<T>, "T must be WinRT type.");
        IIterable(std::nullptr_t = nullptr) noexcept {}
        IIterable(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename T>
    struct WINRT_EBO IVectorView :
        IInspectable,
        impl::consume_t<IVectorView<T>>,
        impl::require<IVectorView<T>, IIterable<T>>
    {
        static_assert(impl::has_category_v<T>, "T must be WinRT type.");
        IVectorView(std::nullptr_t = nullptr) noexcept {}
        IVectorView(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename T>
    struct WINRT_EBO IVector :
        IInspectable,
        impl::consume_t<IVector<T>>,
        impl::require<IVector<T>, IIterable<T>>
    {
        static_assert(impl::has_category_v<T>, "T must be WinRT type.");
        IVector(std::nullptr_t = nullptr) noexcept {}
        IVector(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename T>
    struct WINRT_EBO IObservableVector :
        IInspectable,
        impl::consume_t<IObservableVector<T>>,
        impl::require<IObservableVector<T>, IVector<T>, IIterable<T>>
    {
        static_assert(impl::has_category_v<T>, "T must be WinRT type.");
        IObservableVector(std::nullptr_t = nullptr) noexcept {}
        IObservableVector(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename K, typename V>
    struct WINRT_EBO IKeyValuePair :
        IInspectable,
        impl::consume_t<IKeyValuePair<K, V>>
    {
        static_assert(impl::has_category_v<K>, "K must be WinRT type.");
        static_assert(impl::has_category_v<V>, "V must be WinRT type.");
        IKeyValuePair(std::nullptr_t = nullptr) noexcept {}
        IKeyValuePair(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename K, typename V>
    struct WINRT_EBO IMapView :
        IInspectable,
        impl::consume_t<IMapView<K, V>>,
        impl::require<IMapView<K, V>, IIterable<IKeyValuePair<K, V>>>
    {
        static_assert(impl::has_category_v<K>, "K must be WinRT type.");
        static_assert(impl::has_category_v<V>, "V must be WinRT type.");
        IMapView(std::nullptr_t = nullptr) noexcept {}
        IMapView(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename K, typename V>
    struct WINRT_EBO IMap :
        IInspectable,
        impl::consume_t<IMap<K, V>>,
        impl::require<IMap<K, V>, IIterable<IKeyValuePair<K, V>>>
    {
        static_assert(impl::has_category_v<K>, "K must be WinRT type.");
        static_assert(impl::has_category_v<V>, "V must be WinRT type.");
        IMap(std::nullptr_t = nullptr) noexcept {}
        IMap(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename K, typename V>
    struct WINRT_EBO IObservableMap :
        IInspectable,
        impl::consume_t<IObservableMap<K, V>>,
        impl::require<IObservableMap<K, V>, IMap<K, V>, IIterable<IKeyValuePair<K, V>>>
    {
        static_assert(impl::has_category_v<K>, "K must be WinRT type.");
        static_assert(impl::has_category_v<V>, "V must be WinRT type.");
        IObservableMap(std::nullptr_t = nullptr) noexcept {}
        IObservableMap(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    struct WINRT_EBO IVectorChangedEventArgs :
        IInspectable,
        impl::consume_t<IVectorChangedEventArgs>
    {
        IVectorChangedEventArgs(std::nullptr_t = nullptr) noexcept {}
        IVectorChangedEventArgs(take_ownership_from_abi_t, void* ptr) noexcept : IInspectable(take_ownership_from_abi, ptr) {}
    };

    template <typename T>
    struct WINRT_EBO VectorChangedEventHandler : IUnknown
    {
        static_assert(impl::has_category_v<T>, "T must be WinRT type.");
        VectorChangedEventHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        VectorChangedEventHandler(L handler) :
            VectorChangedEventHandler(impl::make_delegate<VectorChangedEventHandler<T>>(std::forward<L>(handler)))
        {}

        template <typename F> VectorChangedEventHandler(F* handler) :
            VectorChangedEventHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> VectorChangedEventHandler(O* object, M method) :
            VectorChangedEventHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> VectorChangedEventHandler(com_ptr<O>&& object, M method) :
            VectorChangedEventHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> VectorChangedEventHandler(weak_ref<O>&& object, M method) :
            VectorChangedEventHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IObservableVector<T> const& sender, IVectorChangedEventArgs const& args) const
        {
            check_hresult((*(impl::abi_t<VectorChangedEventHandler<T>>**)this)->Invoke(get_abi(sender), get_abi(args)));
        }
    };

    template <typename K, typename V>
    struct WINRT_EBO MapChangedEventHandler : IUnknown
    {
        static_assert(impl::has_category_v<K>, "K must be WinRT type.");
        static_assert(impl::has_category_v<V>, "V must be WinRT type.");
        MapChangedEventHandler(std::nullptr_t = nullptr) noexcept {}

        template <typename L>
        MapChangedEventHandler(L handler) :
            MapChangedEventHandler(impl::make_delegate<MapChangedEventHandler<K, V>>(std::forward<L>(handler)))
        {}

        template <typename F> MapChangedEventHandler(F* handler) :
            MapChangedEventHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> MapChangedEventHandler(O* object, M method) :
            MapChangedEventHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> MapChangedEventHandler(com_ptr<O>&& object, M method) :
            MapChangedEventHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> MapChangedEventHandler(weak_ref<O>&& object, M method) :
            MapChangedEventHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IObservableMap<K, V> const& sender, IMapChangedEventArgs<K> const& args) const
        {
            check_hresult((*(impl::abi_t<MapChangedEventHandler<K, V>>**)this)->Invoke(get_abi(sender), get_abi(args)));
        }
    };
}
