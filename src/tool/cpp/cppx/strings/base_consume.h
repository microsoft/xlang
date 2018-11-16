
namespace xlang::impl
{
    template <typename Async>
    void blocking_suspend(Async const& async);

    template <typename D> struct consume_IActivationFactory
    {
        template <typename T>
        T ActivateInstance() const
        {
            System::IObject instance;
            check_hresult(WINRT_SHIM(System::IActivationFactory)->ActivateInstance(put_abi(instance)));
            return instance.try_as<T>();
        }
    };

    template <typename D, typename T> struct consume_IReference
    {
        T Value() const
        {
            T result{};
            check_hresult(WINRT_SHIM(System::IReference<T>)->get_Value(put_abi(result)));
            return result;
        }
    };

    template <typename D, typename T> struct consume_IReferenceArray
    {
        com_array<T> Value() const
        {
            com_array<T> result{};
            check_hresult(WINRT_SHIM(System::IReferenceArray<T>)->get_Value(impl::put_size_abi(result), put_abi(result)));
            return result;
        }
    };

    template <typename D> struct consume_IVectorChangedEventArgs
    {
        System::CollectionChange CollectionChange() const
        {
            System::CollectionChange value{};
            check_hresult(WINRT_SHIM(System::IVectorChangedEventArgs)->get_CollectionChange(&value));
            return value;
        }

        uint32_t Index() const
        {
            uint32_t index{};
            check_hresult(WINRT_SHIM(System::IVectorChangedEventArgs)->get_Index(&index));
            return index;
        }
    };

    template <typename D, typename K> struct consume_IMapChangedEventArgs
    {
        System::CollectionChange CollectionChange() const
        {
            System::CollectionChange value{};
            check_hresult(WINRT_SHIM(System::IMapChangedEventArgs<K>)->get_CollectionChange(&value));
            return value;
        }

        K Key() const
        {
            K result{ empty_value<K>() };
            check_hresult(WINRT_SHIM(System::IMapChangedEventArgs<K>)->get_Key(put_abi(result)));
            return result;
        }
    };

    template <typename D, typename T> struct consume_IIterator
    {
        T Current() const
        {
            T result{ empty_value<T>() };
            check_hresult(WINRT_SHIM(System::IIterator<T>)->get_Current(put_abi(result)));
            return result;
        }

        bool HasCurrent() const
        {
            bool temp{};
            check_hresult(WINRT_SHIM(System::IIterator<T>)->get_HasCurrent(put_abi(temp)));
            return temp;
        }

        bool MoveNext() const
        {
            bool temp{};
            check_hresult(WINRT_SHIM(System::IIterator<T>)->MoveNext(put_abi(temp)));
            return temp;
        }

        uint32_t GetMany(array_view<T> values) const
        {
            uint32_t actual{};
            check_hresult(WINRT_SHIM(System::IIterator<T>)->GetMany(values.size(), get_abi(values), &actual));
            return actual;
        }

        auto& operator++()
        {
            if (!MoveNext())
            {
                static_cast<D&>(*this) = nullptr;
            }

            return *this;
        }

        T operator*() const
        {
            return Current();
        }
    };

    template <typename D, typename T> struct consume_IIterable
    {
        System::IIterator<T> First() const
        {
            System::IIterator<T> iterator;
            check_hresult(WINRT_SHIM(System::IIterable<T>)->First(put_abi(iterator)));
            return iterator;
        }
    };

    template <typename D, typename T> struct consume_IVectorView
    {
        T GetAt(uint32_t const index) const
        {
            T result{ empty_value<T>() };
            check_hresult(WINRT_SHIM(System::IVectorView<T>)->GetAt(index, put_abi(result)));
            return result;
        }

        uint32_t Size() const
        {
            uint32_t size{};
            check_hresult(WINRT_SHIM(System::IVectorView<T>)->get_Size(&size));
            return size;
        }

        bool IndexOf(param_type<T> const& value, uint32_t& index) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(System::IVectorView<T>)->IndexOf(get_abi(value), &index, &found));
            return found;
        }

        uint32_t GetMany(uint32_t startIndex, array_view<T> values) const
        {
            uint32_t actual{};
            check_hresult(WINRT_SHIM(System::IVectorView<T>)->GetMany(startIndex, values.size(), get_abi(values), &actual));
            return actual;
        }
    };

    template <typename D, typename T> struct consume_IVector
    {
        T GetAt(uint32_t const index) const
        {
            T result{ empty_value<T>() };
            check_hresult(WINRT_SHIM(System::IVector<T>)->GetAt(index, put_abi(result)));
            return result;
        }

        uint32_t Size() const
        {
            uint32_t size = 0;
            check_hresult(WINRT_SHIM(System::IVector<T>)->get_Size(&size));
            return size;
        }

        System::IVectorView<T> GetView() const
        {
            System::IVectorView<T> view;
            check_hresult(WINRT_SHIM(System::IVector<T>)->GetView(put_abi(view)));
            return view;
        }

        bool IndexOf(param_type<T> const& value, uint32_t& index) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(System::IVector<T>)->IndexOf(get_abi(value), &index, &found));
            return found;
        }

        void SetAt(uint32_t const index, param_type<T> const& value) const
        {
            check_hresult(WINRT_SHIM(System::IVector<T>)->SetAt(index, get_abi(value)));
        }

        void InsertAt(uint32_t const index, param_type<T> const& value) const
        {
            check_hresult(WINRT_SHIM(System::IVector<T>)->InsertAt(index, get_abi(value)));
        }

        void RemoveAt(uint32_t const index) const
        {
            check_hresult(WINRT_SHIM(System::IVector<T>)->RemoveAt(index));
        }

        void Append(param_type<T> const& value) const
        {
            check_hresult(WINRT_SHIM(System::IVector<T>)->Append(get_abi(value)));
        }

        void RemoveAtEnd() const
        {
            check_hresult(WINRT_SHIM(System::IVector<T>)->RemoveAtEnd());
        }

        void Clear() const
        {
            check_hresult(WINRT_SHIM(System::IVector<T>)->Clear());
        }

        uint32_t GetMany(uint32_t startIndex, array_view<T> values) const
        {
            uint32_t actual{};
            check_hresult(WINRT_SHIM(System::IVector<T>)->GetMany(startIndex, values.size(), get_abi(values), &actual));
            return actual;
        }

        void ReplaceAll(array_view<T const> value) const
        {
            check_hresult(WINRT_SHIM(System::IVector<T>)->ReplaceAll(value.size(), get_abi(value)));
        }
    };

    template <typename D, typename T> struct consume_IObservableVector
    {
        event_token VectorChanged(System::VectorChangedEventHandler<T> const& handler) const
        {
            event_token cookie{};
            check_hresult(WINRT_SHIM(System::IObservableVector<T>)->add_VectorChanged(get_abi(handler), &cookie));
            return cookie;
        }

        void VectorChanged(event_token const cookie) const noexcept
        {
            WINRT_SHIM(System::IObservableVector<T>)->remove_VectorChanged(cookie);
        }

        using VectorChanged_revoker = event_revoker<System::IObservableVector<T>, &abi_t<System::IObservableVector<T>>::remove_VectorChanged>;

        VectorChanged_revoker VectorChanged(auto_revoke_t, System::VectorChangedEventHandler<T> const& handler) const
        {
            return make_event_revoker<D, VectorChanged_revoker>(this, VectorChanged(handler));
        }
    };

    template <typename D, typename K, typename V> struct consume_IKeyValuePair
    {
        K Key() const
        {
            K result{ empty_value<K>() };
            check_hresult(WINRT_SHIM(System::IKeyValuePair<K, V>)->get_Key(put_abi(result)));
            return result;
        }

        V Value() const
        {
            V result{ empty_value<V>() };
            check_hresult(WINRT_SHIM(System::IKeyValuePair<K, V>)->get_Value(put_abi(result)));
            return result;
        }

        bool operator==(System::IKeyValuePair<K, V> const& other) const
        {
            return Key() == other.Key() && Value() == other.Value();
        }

        bool operator!=(System::IKeyValuePair<K, V> const& other) const
        {
            return !(*this == other);
        }
    };

    template <typename D, typename K, typename V> struct consume_IMapView
    {
        V Lookup(param_type<K> const& key) const
        {
            V result{ empty_value<V>() };
            check_hresult(WINRT_SHIM(System::IMapView<K, V>)->Lookup(get_abi(key), put_abi(result)));
            return result;
        }

        auto TryLookup(param_type<K> const& key) const noexcept
        {
            if constexpr (std::is_base_of_v<System::IUnknown, V>)
            {
                V result{ nullptr };
                WINRT_SHIM(System::IMapView<K, V>)->Lookup(get_abi(key), put_abi(result));
                return result;
            }
            else
            {
                std::optional<V> result;
                V value{ empty_value<V>() };

                if (error_ok == WINRT_SHIM(System::IMapView<K, V>)->Lookup(get_abi(key), put_abi(value)))
                {
                    result = std::move(value);
                }

                return result;
            }
        }

        uint32_t Size() const
        {
            uint32_t size{};
            check_hresult(WINRT_SHIM(System::IMapView<K, V>)->get_Size(&size));
            return size;
        }

        bool HasKey(param_type<K> const& key) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(System::IMapView<K, V>)->HasKey(get_abi(key), &found));
            return found;
        }
        void Split(System::IMapView<K, V>& firstPartition, System::IMapView<K, V>& secondPartition)
        {
            check_hresult(WINRT_SHIM(System::IMapView<K, V>)->Split(put_abi(firstPartition), put_abi(secondPartition)));
        }
    };

    template <typename D, typename K, typename V> struct consume_IMap
    {
        V Lookup(param_type<K> const& key) const
        {
            V result{ empty_value<V>() };
            check_hresult(WINRT_SHIM(System::IMap<K, V>)->Lookup(get_abi(key), put_abi(result)));
            return result;
        }

        auto TryLookup(param_type<K> const& key) const noexcept
        {
            if constexpr (std::is_base_of_v<System::IUnknown, V>)
            {
                V result{ nullptr };
                WINRT_SHIM(System::IMap<K, V>)->Lookup(get_abi(key), put_abi(result));
                return result;
            }
            else
            {
                std::optional<V> result;
                V value{ empty_value<V>() };

                if (error_ok == WINRT_SHIM(System::IMap<K, V>)->Lookup(get_abi(key), put_abi(value)))
                {
                    result = std::move(value);
                }

                return result;
            }
        }

        uint32_t Size() const
        {
            uint32_t size{};
            check_hresult(WINRT_SHIM(System::IMap<K, V>)->get_Size(&size));
            return size;
        }

        bool HasKey(param_type<K> const& key) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(System::IMap<K, V>)->HasKey(get_abi(key), &found));
            return found;
        }

        System::IMapView<K, V> GetView() const
        {
            System::IMapView<K, V> view;
            check_hresult(WINRT_SHIM(System::IMap<K, V>)->GetView(put_abi(view)));
            return view;
        }

        bool Insert(param_type<K> const& key, param_type<V> const& value) const
        {
            bool replaced{};
            check_hresult(WINRT_SHIM(System::IMap<K, V>)->Insert(get_abi(key), get_abi(value), &replaced));
            return replaced;
        }

        void Remove(param_type<K> const& key) const
        {
            check_hresult(WINRT_SHIM(System::IMap<K, V>)->Remove(get_abi(key)));
        }

        void Clear() const
        {
            check_hresult(WINRT_SHIM(System::IMap<K, V>)->Clear());
        }
    };

    template <typename D, typename K, typename V> struct consume_IObservableMap
    {
        event_token MapChanged(System::MapChangedEventHandler<K, V> const& handler) const
        {
            event_token cookie{};
            check_hresult(WINRT_SHIM(System::IObservableMap<K, V>)->add_MapChanged(get_abi(handler), &cookie));
            return cookie;
        }

        void MapChanged(event_token const cookie) const noexcept
        {
            WINRT_SHIM(System::IObservableMap<K, V>)->remove_MapChanged(cookie);
        }

        using MapChanged_revoker = event_revoker<System::IObservableMap<K, V>, &abi_t<System::IObservableMap<K, V>>::remove_MapChanged>;

        MapChanged_revoker MapChanged(auto_revoke_t, System::MapChangedEventHandler<K, V> const& handler) const
        {
            return make_event_revoker<D, MapChanged_revoker>(this, MapChanged(handler));
        }
    };

    template <typename D> struct consume_IAsyncAction
    {
        System::AsyncStatus Status() const
        {
            System::AsyncStatus status{};
            check_hresult(WINRT_SHIM(System::IAsyncAction)->get_Status(&status));
            return status;
        }

        void Cancel() const
        {
            check_hresult(WINRT_SHIM(System::IAsyncAction)->Cancel());
        }

        void Completed(System::AsyncActionCompletedHandler const& handler) const;
        System::AsyncActionCompletedHandler Completed() const;

        void GetResults() const
        {
            check_hresult(WINRT_SHIM(System::IAsyncAction)->GetResults());
        }

        void get() const
        {
            blocking_suspend(static_cast<System::IAsyncAction const&>(static_cast<D const&>(*this)));
            GetResults();
        }
    };

    template <typename D, typename TResult> struct consume_IAsyncOperation
    {
        System::AsyncStatus Status() const
        {
            System::AsyncStatus status{};
            check_hresult(WINRT_SHIM(System::IAsyncOperation<TResult>)->get_Status(&status));
            return status;
        }

        void Cancel() const
        {
            check_hresult(WINRT_SHIM(System::IAsyncOperation<TResult>)->Cancel());
        }

        void Completed(System::AsyncOperationCompletedHandler<TResult> const& handler) const;
        System::AsyncOperationCompletedHandler<TResult> Completed() const;

        TResult GetResults() const
        {
            TResult result = empty_value<TResult>();
            check_hresult(WINRT_SHIM(System::IAsyncOperation<TResult>)->GetResults(put_abi(result)));
            return result;
        }

        TResult get() const
        {
            blocking_suspend(static_cast<System::IAsyncOperation<TResult> const&>(static_cast<D const&>(*this)));
            return GetResults();
        }
    };

    template <typename D, typename TProgress> struct consume_IAsyncActionWithProgress
    {
        System::AsyncStatus Status() const
        {
            System::AsyncStatus status{};
            check_hresult(WINRT_SHIM(System::IAsyncActionWithProgress<TProgress>)->get_Status(&status));
            return status;
        }

        void Cancel() const
        {
            check_hresult(WINRT_SHIM(System::IAsyncActionWithProgress<TProgress>)->Cancel());
        }

        void Progress(System::AsyncActionProgressHandler<TProgress> const& handler) const;
        System::AsyncActionProgressHandler<TProgress> Progress() const;

        void Completed(System::AsyncActionWithProgressCompletedHandler<TProgress> const& handler) const;
        System::AsyncActionWithProgressCompletedHandler<TProgress> Completed() const;

        void GetResults() const
        {
            check_hresult(WINRT_SHIM(System::IAsyncActionWithProgress<TProgress>)->GetResults());
        }

        void get() const
        {
            blocking_suspend(static_cast<System::IAsyncActionWithProgress<TProgress> const&>(static_cast<D const&>(*this)));
            GetResults();
        }
    };

    template <typename D, typename TResult, typename TProgress> struct consume_IAsyncOperationWithProgress
    {
        System::AsyncStatus Status() const
        {
            System::AsyncStatus status{};
            check_hresult(WINRT_SHIM(System::IAsyncOperationWithProgress<TResult, TProgress>)->get_Status(&status));
            return status;
        }

        void Cancel() const
        {
            check_hresult(WINRT_SHIM(System::IAsyncOperationWithProgress<TResult, TProgress>)->Cancel());
        }

        void Progress(System::AsyncOperationProgressHandler<TResult, TProgress> const& handler) const;
        System::AsyncOperationProgressHandler<TResult, TProgress> Progress() const;

        void Completed(System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const& handler) const;
        System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> Completed() const;

        TResult GetResults() const
        {
            TResult result = empty_value<TResult>();
            check_hresult(WINRT_SHIM(System::IAsyncOperationWithProgress<TResult, TProgress>)->GetResults(put_abi(result)));
            return result;
        }

        TResult get() const
        {
            blocking_suspend(static_cast<System::IAsyncOperationWithProgress<TResult, TProgress> const&>(static_cast<D const&>(*this)));
            return GetResults();
        }
    };
}
