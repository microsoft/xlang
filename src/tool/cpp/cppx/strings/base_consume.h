
namespace xlang::impl
{
    template <typename Async>
    void blocking_suspend(Async const& async);

    template <typename D> struct consume_IActivationFactory
    {
        template <typename T>
        T ActivateInstance() const
        {
            Runtime::IObject instance;
            check_hresult(WINRT_SHIM(Runtime::IActivationFactory)->ActivateInstance(put_abi(instance)));
            return instance.try_as<T>();
        }
    };

    template <typename D, typename T> struct consume_IReference
    {
        T Value() const
        {
            T result{};
            check_hresult(WINRT_SHIM(Runtime::IReference<T>)->get_Value(put_abi(result)));
            return result;
        }
    };

    template <typename D, typename T> struct consume_IReferenceArray
    {
        com_array<T> Value() const
        {
            com_array<T> result{};
            check_hresult(WINRT_SHIM(Runtime::IReferenceArray<T>)->get_Value(impl::put_size_abi(result), put_abi(result)));
            return result;
        }
    };

    template <typename D> struct consume_IVectorChangedEventArgs
    {
        Runtime::CollectionChange CollectionChange() const
        {
            Runtime::CollectionChange value{};
            check_hresult(WINRT_SHIM(Runtime::IVectorChangedEventArgs)->get_CollectionChange(&value));
            return value;
        }

        uint32_t Index() const
        {
            uint32_t index{};
            check_hresult(WINRT_SHIM(Runtime::IVectorChangedEventArgs)->get_Index(&index));
            return index;
        }
    };

    template <typename D, typename K> struct consume_IMapChangedEventArgs
    {
        Runtime::CollectionChange CollectionChange() const
        {
            Runtime::CollectionChange value{};
            check_hresult(WINRT_SHIM(Runtime::IMapChangedEventArgs<K>)->get_CollectionChange(&value));
            return value;
        }

        K Key() const
        {
            K result{ empty_value<K>() };
            check_hresult(WINRT_SHIM(Runtime::IMapChangedEventArgs<K>)->get_Key(put_abi(result)));
            return result;
        }
    };

    template <typename D, typename T> struct consume_IIterator
    {
        T Current() const
        {
            T result{ empty_value<T>() };
            check_hresult(WINRT_SHIM(Runtime::IIterator<T>)->get_Current(put_abi(result)));
            return result;
        }

        bool HasCurrent() const
        {
            bool temp{};
            check_hresult(WINRT_SHIM(Runtime::IIterator<T>)->get_HasCurrent(put_abi(temp)));
            return temp;
        }

        bool MoveNext() const
        {
            bool temp{};
            check_hresult(WINRT_SHIM(Runtime::IIterator<T>)->MoveNext(put_abi(temp)));
            return temp;
        }

        uint32_t GetMany(array_view<T> values) const
        {
            uint32_t actual{};
            check_hresult(WINRT_SHIM(Runtime::IIterator<T>)->GetMany(values.size(), get_abi(values), &actual));
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
        Runtime::IIterator<T> First() const
        {
            Runtime::IIterator<T> iterator;
            check_hresult(WINRT_SHIM(Runtime::IIterable<T>)->First(put_abi(iterator)));
            return iterator;
        }
    };

    template <typename D, typename T> struct consume_IVectorView
    {
        T GetAt(uint32_t const index) const
        {
            T result{ empty_value<T>() };
            check_hresult(WINRT_SHIM(Runtime::IVectorView<T>)->GetAt(index, put_abi(result)));
            return result;
        }

        uint32_t Size() const
        {
            uint32_t size{};
            check_hresult(WINRT_SHIM(Runtime::IVectorView<T>)->get_Size(&size));
            return size;
        }

        bool IndexOf(param_type<T> const& value, uint32_t& index) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(Runtime::IVectorView<T>)->IndexOf(get_abi(value), &index, &found));
            return found;
        }

        uint32_t GetMany(uint32_t startIndex, array_view<T> values) const
        {
            uint32_t actual{};
            check_hresult(WINRT_SHIM(Runtime::IVectorView<T>)->GetMany(startIndex, values.size(), get_abi(values), &actual));
            return actual;
        }
    };

    template <typename D, typename T> struct consume_IVector
    {
        T GetAt(uint32_t const index) const
        {
            T result{ empty_value<T>() };
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->GetAt(index, put_abi(result)));
            return result;
        }

        uint32_t Size() const
        {
            uint32_t size = 0;
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->get_Size(&size));
            return size;
        }

        Runtime::IVectorView<T> GetView() const
        {
            Runtime::IVectorView<T> view;
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->GetView(put_abi(view)));
            return view;
        }

        bool IndexOf(param_type<T> const& value, uint32_t& index) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->IndexOf(get_abi(value), &index, &found));
            return found;
        }

        void SetAt(uint32_t const index, param_type<T> const& value) const
        {
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->SetAt(index, get_abi(value)));
        }

        void InsertAt(uint32_t const index, param_type<T> const& value) const
        {
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->InsertAt(index, get_abi(value)));
        }

        void RemoveAt(uint32_t const index) const
        {
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->RemoveAt(index));
        }

        void Append(param_type<T> const& value) const
        {
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->Append(get_abi(value)));
        }

        void RemoveAtEnd() const
        {
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->RemoveAtEnd());
        }

        void Clear() const
        {
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->Clear());
        }

        uint32_t GetMany(uint32_t startIndex, array_view<T> values) const
        {
            uint32_t actual{};
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->GetMany(startIndex, values.size(), get_abi(values), &actual));
            return actual;
        }

        void ReplaceAll(array_view<T const> value) const
        {
            check_hresult(WINRT_SHIM(Runtime::IVector<T>)->ReplaceAll(value.size(), get_abi(value)));
        }
    };

    template <typename D, typename T> struct consume_IObservableVector
    {
        event_token VectorChanged(Runtime::VectorChangedEventHandler<T> const& handler) const
        {
            event_token cookie{};
            check_hresult(WINRT_SHIM(Runtime::IObservableVector<T>)->add_VectorChanged(get_abi(handler), &cookie));
            return cookie;
        }

        void VectorChanged(event_token const cookie) const noexcept
        {
            WINRT_SHIM(Runtime::IObservableVector<T>)->remove_VectorChanged(cookie);
        }

        using VectorChanged_revoker = event_revoker<Runtime::IObservableVector<T>, &abi_t<Runtime::IObservableVector<T>>::remove_VectorChanged>;

        VectorChanged_revoker VectorChanged(auto_revoke_t, Runtime::VectorChangedEventHandler<T> const& handler) const
        {
            return make_event_revoker<D, VectorChanged_revoker>(this, VectorChanged(handler));
        }
    };

    template <typename D, typename K, typename V> struct consume_IKeyValuePair
    {
        K Key() const
        {
            K result{ empty_value<K>() };
            check_hresult(WINRT_SHIM(Runtime::IKeyValuePair<K, V>)->get_Key(put_abi(result)));
            return result;
        }

        V Value() const
        {
            V result{ empty_value<V>() };
            check_hresult(WINRT_SHIM(Runtime::IKeyValuePair<K, V>)->get_Value(put_abi(result)));
            return result;
        }

        bool operator==(Runtime::IKeyValuePair<K, V> const& other) const
        {
            return Key() == other.Key() && Value() == other.Value();
        }

        bool operator!=(Runtime::IKeyValuePair<K, V> const& other) const
        {
            return !(*this == other);
        }
    };

    template <typename D, typename K, typename V> struct consume_IMapView
    {
        V Lookup(param_type<K> const& key) const
        {
            V result{ empty_value<V>() };
            check_hresult(WINRT_SHIM(Runtime::IMapView<K, V>)->Lookup(get_abi(key), put_abi(result)));
            return result;
        }

        auto TryLookup(param_type<K> const& key) const noexcept
        {
            if constexpr (std::is_base_of_v<Runtime::IUnknown, V>)
            {
                V result{ nullptr };
                WINRT_SHIM(Runtime::IMapView<K, V>)->Lookup(get_abi(key), put_abi(result));
                return result;
            }
            else
            {
                std::optional<V> result;
                V value{ empty_value<V>() };

                if (error_ok == WINRT_SHIM(Runtime::IMapView<K, V>)->Lookup(get_abi(key), put_abi(value)))
                {
                    result = std::move(value);
                }

                return result;
            }
        }

        uint32_t Size() const
        {
            uint32_t size{};
            check_hresult(WINRT_SHIM(Runtime::IMapView<K, V>)->get_Size(&size));
            return size;
        }

        bool HasKey(param_type<K> const& key) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(Runtime::IMapView<K, V>)->HasKey(get_abi(key), &found));
            return found;
        }
        void Split(Runtime::IMapView<K, V>& firstPartition, Runtime::IMapView<K, V>& secondPartition)
        {
            check_hresult(WINRT_SHIM(Runtime::IMapView<K, V>)->Split(put_abi(firstPartition), put_abi(secondPartition)));
        }
    };

    template <typename D, typename K, typename V> struct consume_IMap
    {
        V Lookup(param_type<K> const& key) const
        {
            V result{ empty_value<V>() };
            check_hresult(WINRT_SHIM(Runtime::IMap<K, V>)->Lookup(get_abi(key), put_abi(result)));
            return result;
        }

        auto TryLookup(param_type<K> const& key) const noexcept
        {
            if constexpr (std::is_base_of_v<Runtime::IUnknown, V>)
            {
                V result{ nullptr };
                WINRT_SHIM(Runtime::IMap<K, V>)->Lookup(get_abi(key), put_abi(result));
                return result;
            }
            else
            {
                std::optional<V> result;
                V value{ empty_value<V>() };

                if (error_ok == WINRT_SHIM(Runtime::IMap<K, V>)->Lookup(get_abi(key), put_abi(value)))
                {
                    result = std::move(value);
                }

                return result;
            }
        }

        uint32_t Size() const
        {
            uint32_t size{};
            check_hresult(WINRT_SHIM(Runtime::IMap<K, V>)->get_Size(&size));
            return size;
        }

        bool HasKey(param_type<K> const& key) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(Runtime::IMap<K, V>)->HasKey(get_abi(key), &found));
            return found;
        }

        Runtime::IMapView<K, V> GetView() const
        {
            Runtime::IMapView<K, V> view;
            check_hresult(WINRT_SHIM(Runtime::IMap<K, V>)->GetView(put_abi(view)));
            return view;
        }

        bool Insert(param_type<K> const& key, param_type<V> const& value) const
        {
            bool replaced{};
            check_hresult(WINRT_SHIM(Runtime::IMap<K, V>)->Insert(get_abi(key), get_abi(value), &replaced));
            return replaced;
        }

        void Remove(param_type<K> const& key) const
        {
            check_hresult(WINRT_SHIM(Runtime::IMap<K, V>)->Remove(get_abi(key)));
        }

        void Clear() const
        {
            check_hresult(WINRT_SHIM(Runtime::IMap<K, V>)->Clear());
        }
    };

    template <typename D, typename K, typename V> struct consume_IObservableMap
    {
        event_token MapChanged(Runtime::MapChangedEventHandler<K, V> const& handler) const
        {
            event_token cookie{};
            check_hresult(WINRT_SHIM(Runtime::IObservableMap<K, V>)->add_MapChanged(get_abi(handler), &cookie));
            return cookie;
        }

        void MapChanged(event_token const cookie) const noexcept
        {
            WINRT_SHIM(Runtime::IObservableMap<K, V>)->remove_MapChanged(cookie);
        }

        using MapChanged_revoker = event_revoker<Runtime::IObservableMap<K, V>, &abi_t<Runtime::IObservableMap<K, V>>::remove_MapChanged>;

        MapChanged_revoker MapChanged(auto_revoke_t, Runtime::MapChangedEventHandler<K, V> const& handler) const
        {
            return make_event_revoker<D, MapChanged_revoker>(this, MapChanged(handler));
        }
    };

    template <typename D> struct consume_IAsyncAction
    {
        Runtime::AsyncStatus Status() const
        {
            Runtime::AsyncStatus status{};
            check_hresult(WINRT_SHIM(Runtime::IAsyncAction)->get_Status(&status));
            return status;
        }

        void Cancel() const
        {
            check_hresult(WINRT_SHIM(Runtime::IAsyncAction)->Cancel());
        }

        void Completed(Runtime::AsyncActionCompletedHandler const& handler) const;
        Runtime::AsyncActionCompletedHandler Completed() const;

        void GetResults() const
        {
            check_hresult(WINRT_SHIM(Runtime::IAsyncAction)->GetResults());
        }

        void get() const
        {
            blocking_suspend(static_cast<Runtime::IAsyncAction const&>(static_cast<D const&>(*this)));
            GetResults();
        }
    };

    template <typename D, typename TResult> struct consume_IAsyncOperation
    {
        Runtime::AsyncStatus Status() const
        {
            Runtime::AsyncStatus status{};
            check_hresult(WINRT_SHIM(Runtime::IAsyncOperation<TResult>)->get_Status(&status));
            return status;
        }

        void Cancel() const
        {
            check_hresult(WINRT_SHIM(Runtime::IAsyncOperation<TResult>)->Cancel());
        }

        void Completed(Runtime::AsyncOperationCompletedHandler<TResult> const& handler) const;
        Runtime::AsyncOperationCompletedHandler<TResult> Completed() const;

        TResult GetResults() const
        {
            TResult result = empty_value<TResult>();
            check_hresult(WINRT_SHIM(Runtime::IAsyncOperation<TResult>)->GetResults(put_abi(result)));
            return result;
        }

        TResult get() const
        {
            blocking_suspend(static_cast<Runtime::IAsyncOperation<TResult> const&>(static_cast<D const&>(*this)));
            return GetResults();
        }
    };

    template <typename D, typename TProgress> struct consume_IAsyncActionWithProgress
    {
        Runtime::AsyncStatus Status() const
        {
            Runtime::AsyncStatus status{};
            check_hresult(WINRT_SHIM(Runtime::IAsyncActionWithProgress<TProgress>)->get_Status(&status));
            return status;
        }

        void Cancel() const
        {
            check_hresult(WINRT_SHIM(Runtime::IAsyncActionWithProgress<TProgress>)->Cancel());
        }

        void Progress(Runtime::AsyncActionProgressHandler<TProgress> const& handler) const;
        Runtime::AsyncActionProgressHandler<TProgress> Progress() const;

        void Completed(Runtime::AsyncActionWithProgressCompletedHandler<TProgress> const& handler) const;
        Runtime::AsyncActionWithProgressCompletedHandler<TProgress> Completed() const;

        void GetResults() const
        {
            check_hresult(WINRT_SHIM(Runtime::IAsyncActionWithProgress<TProgress>)->GetResults());
        }

        void get() const
        {
            blocking_suspend(static_cast<Runtime::IAsyncActionWithProgress<TProgress> const&>(static_cast<D const&>(*this)));
            GetResults();
        }
    };

    template <typename D, typename TResult, typename TProgress> struct consume_IAsyncOperationWithProgress
    {
        Runtime::AsyncStatus Status() const
        {
            Runtime::AsyncStatus status{};
            check_hresult(WINRT_SHIM(Runtime::IAsyncOperationWithProgress<TResult, TProgress>)->get_Status(&status));
            return status;
        }

        void Cancel() const
        {
            check_hresult(WINRT_SHIM(Runtime::IAsyncOperationWithProgress<TResult, TProgress>)->Cancel());
        }

        void Progress(Runtime::AsyncOperationProgressHandler<TResult, TProgress> const& handler) const;
        Runtime::AsyncOperationProgressHandler<TResult, TProgress> Progress() const;

        void Completed(Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const& handler) const;
        Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> Completed() const;

        TResult GetResults() const
        {
            TResult result = empty_value<TResult>();
            check_hresult(WINRT_SHIM(Runtime::IAsyncOperationWithProgress<TResult, TProgress>)->GetResults(put_abi(result)));
            return result;
        }

        TResult get() const
        {
            blocking_suspend(static_cast<Runtime::IAsyncOperationWithProgress<TResult, TProgress> const&>(static_cast<D const&>(*this)));
            return GetResults();
        }
    };
}
