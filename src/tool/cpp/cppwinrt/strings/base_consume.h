
namespace winrt::impl
{
    template <typename Async>
    void blocking_suspend(Async const& async);

    template <typename D> struct consume_IActivationFactory
    {
        template <typename T>
        T ActivateInstance() const
        {
            Windows::Foundation::IInspectable instance;
            check_hresult(WINRT_SHIM(Windows::Foundation::IActivationFactory)->ActivateInstance(put_abi(instance)));
            return instance.try_as<T>();
        }
    };

    template <typename D, typename T> struct consume_IReference
    {
        T Value() const
        {
            T result{};
            check_hresult(WINRT_SHIM(Windows::Foundation::IReference<T>)->get_Value(put_abi(result)));
            return result;
        }
    };

    template <typename D, typename T> struct consume_IReferenceArray
    {
        com_array<T> Value() const
        {
            com_array<T> result{};
            check_hresult(WINRT_SHIM(Windows::Foundation::IReferenceArray<T>)->get_Value(impl::put_size_abi(result), put_abi(result)));
            return result;
        }
    };

    template <typename D> struct consume_IVectorChangedEventArgs
    {
        wfc::CollectionChange CollectionChange() const
        {
            wfc::CollectionChange value{};
            check_hresult(WINRT_SHIM(wfc::IVectorChangedEventArgs)->get_CollectionChange(&value));
            return value;
        }

        uint32_t Index() const
        {
            uint32_t index{};
            check_hresult(WINRT_SHIM(wfc::IVectorChangedEventArgs)->get_Index(&index));
            return index;
        }
    };

    template <typename D, typename K> struct consume_IMapChangedEventArgs
    {
        wfc::CollectionChange CollectionChange() const
        {
            wfc::CollectionChange value{};
            check_hresult(WINRT_SHIM(wfc::IMapChangedEventArgs<K>)->get_CollectionChange(&value));
            return value;
        }

        K Key() const
        {
            K result{ empty_value<K>() };
            check_hresult(WINRT_SHIM(wfc::IMapChangedEventArgs<K>)->get_Key(put_abi(result)));
            return result;
        }
    };

    template <typename D, typename T> struct consume_IIterator
    {
        T Current() const
        {
            T result{ empty_value<T>() };
            check_hresult(WINRT_SHIM(wfc::IIterator<T>)->get_Current(put_abi(result)));
            return result;
        }

        bool HasCurrent() const
        {
            bool result{};
            check_hresult(WINRT_SHIM(wfc::IIterator<T>)->get_HasCurrent(&result));
            return result;
        }

        bool MoveNext() const
        {
            bool result{};
            check_hresult(WINRT_SHIM(wfc::IIterator<T>)->MoveNext(&result));
            return result;
        }

        uint32_t GetMany(array_view<T> values) const
        {
            uint32_t actual{};
            check_hresult(WINRT_SHIM(wfc::IIterator<T>)->GetMany(values.size(), get_abi(values), &actual));
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
        wfc::IIterator<T> First() const
        {
            void* result;
            check_hresult(WINRT_SHIM(wfc::IIterable<T>)->First(&result));
            return { result, take_ownership_from_abi };
        }
    };

    template <typename D, typename T> struct consume_IVectorView
    {
        T GetAt(uint32_t const index) const
        {
            T result{ empty_value<T>() };
            check_hresult(WINRT_SHIM(wfc::IVectorView<T>)->GetAt(index, put_abi(result)));
            return result;
        }

        uint32_t Size() const
        {
            uint32_t size{};
            check_hresult(WINRT_SHIM(wfc::IVectorView<T>)->get_Size(&size));
            return size;
        }

        bool IndexOf(param_type<T> const& value, uint32_t& index) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(wfc::IVectorView<T>)->IndexOf(get_abi(value), &index, &found));
            return found;
        }

        uint32_t GetMany(uint32_t startIndex, array_view<T> values) const
        {
            uint32_t actual{};
            check_hresult(WINRT_SHIM(wfc::IVectorView<T>)->GetMany(startIndex, values.size(), get_abi(values), &actual));
            return actual;
        }
    };

    template <typename D, typename T> struct consume_IVector
    {
        T GetAt(uint32_t const index) const
        {
            T result{ empty_value<T>() };
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->GetAt(index, put_abi(result)));
            return result;
        }

        uint32_t Size() const
        {
            uint32_t size = 0;
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->get_Size(&size));
            return size;
        }

        wfc::IVectorView<T> GetView() const
        {
            void* result;
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->GetView(&result));
            return { result, take_ownership_from_abi };
        }

        bool IndexOf(param_type<T> const& value, uint32_t& index) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->IndexOf(get_abi(value), &index, &found));
            return found;
        }

        void SetAt(uint32_t const index, param_type<T> const& value) const
        {
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->SetAt(index, get_abi(value)));
        }

        void InsertAt(uint32_t const index, param_type<T> const& value) const
        {
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->InsertAt(index, get_abi(value)));
        }

        void RemoveAt(uint32_t const index) const
        {
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->RemoveAt(index));
        }

        void Append(param_type<T> const& value) const
        {
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->Append(get_abi(value)));
        }

        void RemoveAtEnd() const
        {
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->RemoveAtEnd());
        }

        void Clear() const
        {
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->Clear());
        }

        uint32_t GetMany(uint32_t startIndex, array_view<T> values) const
        {
            uint32_t actual{};
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->GetMany(startIndex, values.size(), get_abi(values), &actual));
            return actual;
        }

        void ReplaceAll(array_view<T const> value) const
        {
            check_hresult(WINRT_SHIM(wfc::IVector<T>)->ReplaceAll(value.size(), get_abi(value)));
        }
    };

    template <typename D, typename K, typename V> struct consume_IKeyValuePair
    {
        K Key() const
        {
            K result{ empty_value<K>() };
            check_hresult(WINRT_SHIM(wfc::IKeyValuePair<K, V>)->get_Key(put_abi(result)));
            return result;
        }

        V Value() const
        {
            V result{ empty_value<V>() };
            check_hresult(WINRT_SHIM(wfc::IKeyValuePair<K, V>)->get_Value(put_abi(result)));
            return result;
        }

        bool operator==(wfc::IKeyValuePair<K, V> const& other) const
        {
            return Key() == other.Key() && Value() == other.Value();
        }

        bool operator!=(wfc::IKeyValuePair<K, V> const& other) const
        {
            return !(*this == other);
        }
    };

    template <typename D, typename K, typename V> struct consume_IMapView
    {
        V Lookup(param_type<K> const& key) const
        {
            V result{ empty_value<V>() };
            check_hresult(WINRT_SHIM(wfc::IMapView<K, V>)->Lookup(get_abi(key), put_abi(result)));
            return result;
        }

        auto TryLookup(param_type<K> const& key) const noexcept
        {
            if constexpr (std::is_base_of_v<Windows::Foundation::IUnknown, V>)
            {
                V result{ nullptr };
                WINRT_SHIM(wfc::IMapView<K, V>)->Lookup(get_abi(key), put_abi(result));
                return result;
            }
            else
            {
                std::optional<V> result;
                V value{ empty_value<V>() };

                if (error_ok == WINRT_SHIM(wfc::IMapView<K, V>)->Lookup(get_abi(key), put_abi(value)))
                {
                    result = std::move(value);
                }

                return result;
            }
        }

        uint32_t Size() const
        {
            uint32_t size{};
            check_hresult(WINRT_SHIM(wfc::IMapView<K, V>)->get_Size(&size));
            return size;
        }

        bool HasKey(param_type<K> const& key) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(wfc::IMapView<K, V>)->HasKey(get_abi(key), &found));
            return found;
        }

        void Split(wfc::IMapView<K, V>& firstPartition, wfc::IMapView<K, V>& secondPartition)
        {
            check_hresult(WINRT_SHIM(wfc::IMapView<K, V>)->Split(put_abi(firstPartition), put_abi(secondPartition)));
        }
    };

    template <typename D, typename K, typename V> struct consume_IMap
    {
        V Lookup(param_type<K> const& key) const
        {
            V result{ empty_value<V>() };
            check_hresult(WINRT_SHIM(wfc::IMap<K, V>)->Lookup(get_abi(key), put_abi(result)));
            return result;
        }

        auto TryLookup(param_type<K> const& key) const noexcept
        {
            if constexpr (std::is_base_of_v<Windows::Foundation::IUnknown, V>)
            {
                V result{ nullptr };
                WINRT_SHIM(wfc::IMap<K, V>)->Lookup(get_abi(key), put_abi(result));
                return result;
            }
            else
            {
                std::optional<V> result;
                V value{ empty_value<V>() };

                if (error_ok == WINRT_SHIM(wfc::IMap<K, V>)->Lookup(get_abi(key), put_abi(value)))
                {
                    result = std::move(value);
                }

                return result;
            }
        }

        uint32_t Size() const
        {
            uint32_t size{};
            check_hresult(WINRT_SHIM(wfc::IMap<K, V>)->get_Size(&size));
            return size;
        }

        bool HasKey(param_type<K> const& key) const
        {
            bool found{};
            check_hresult(WINRT_SHIM(wfc::IMap<K, V>)->HasKey(get_abi(key), &found));
            return found;
        }

        wfc::IMapView<K, V> GetView() const
        {
            void* result;
            check_hresult(WINRT_SHIM(wfc::IMap<K, V>)->GetView(&result));
            return { result, take_ownership_from_abi };
        }

        bool Insert(param_type<K> const& key, param_type<V> const& value) const
        {
            bool replaced{};
            check_hresult(WINRT_SHIM(wfc::IMap<K, V>)->Insert(get_abi(key), get_abi(value), &replaced));
            return replaced;
        }

        void Remove(param_type<K> const& key) const
        {
            check_hresult(WINRT_SHIM(wfc::IMap<K, V>)->Remove(get_abi(key)));
        }

        void Clear() const
        {
            check_hresult(WINRT_SHIM(wfc::IMap<K, V>)->Clear());
        }
    };

    template <typename D, typename K, typename V> struct consume_IObservableMap
    {
        event_token MapChanged(wfc::MapChangedEventHandler<K, V> const& handler) const
        {
            event_token cookie{};
            check_hresult(WINRT_SHIM(wfc::IObservableMap<K, V>)->add_MapChanged(get_abi(handler), &cookie));
            return cookie;
        }

        void MapChanged(event_token const cookie) const noexcept
        {
            WINRT_SHIM(wfc::IObservableMap<K, V>)->remove_MapChanged(cookie);
        }

        using MapChanged_revoker = event_revoker<wfc::IObservableMap<K, V>, &abi_t<wfc::IObservableMap<K, V>>::remove_MapChanged>;

        MapChanged_revoker MapChanged(auto_revoke_t, wfc::MapChangedEventHandler<K, V> const& handler) const
        {
            return make_event_revoker<D, MapChanged_revoker>(this, MapChanged(handler));
        }
    };

    template <typename D> struct consume_IAsyncInfo
    {
        uint32_t Id() const
        {
            uint32_t id{};
            check_hresult(WINRT_SHIM(Windows::Foundation::IAsyncInfo)->get_Id(&id));
            return id;
        }

        Windows::Foundation::AsyncStatus Status() const
        {
            Windows::Foundation::AsyncStatus status{};
            check_hresult(WINRT_SHIM(Windows::Foundation::IAsyncInfo)->get_Status(&status));
            return status;
        }

        hresult ErrorCode() const
        {
            int32_t code{};
            check_hresult(WINRT_SHIM(Windows::Foundation::IAsyncInfo)->get_ErrorCode(&code));
            return code;
        }

        void Cancel() const
        {
            check_hresult(WINRT_SHIM(Windows::Foundation::IAsyncInfo)->Cancel());
        }

        void Close() const
        {
            check_hresult(WINRT_SHIM(Windows::Foundation::IAsyncInfo)->Close());
        }
    };

    template <typename D> struct consume_IAsyncAction
    {
        void Completed(Windows::Foundation::AsyncActionCompletedHandler const& handler) const;
        Windows::Foundation::AsyncActionCompletedHandler Completed() const;

        void GetResults() const
        {
            check_hresult(WINRT_SHIM(Windows::Foundation::IAsyncAction)->GetResults());
        }

        void get() const
        {
            blocking_suspend(static_cast<Windows::Foundation::IAsyncAction const&>(static_cast<D const&>(*this)));
            GetResults();
        }
    };

    template <typename D, typename TResult> struct consume_IAsyncOperation
    {
        void Completed(Windows::Foundation::AsyncOperationCompletedHandler<TResult> const& handler) const;
        Windows::Foundation::AsyncOperationCompletedHandler<TResult> Completed() const;

        TResult GetResults() const
        {
            TResult result = empty_value<TResult>();
            check_hresult(WINRT_SHIM(Windows::Foundation::IAsyncOperation<TResult>)->GetResults(put_abi(result)));
            return result;
        }

        TResult get() const
        {
            blocking_suspend(static_cast<Windows::Foundation::IAsyncOperation<TResult> const&>(static_cast<D const&>(*this)));
            return GetResults();
        }
    };

    template <typename D, typename TProgress> struct consume_IAsyncActionWithProgress
    {
        void Progress(Windows::Foundation::AsyncActionProgressHandler<TProgress> const& handler) const;
        Windows::Foundation::AsyncActionProgressHandler<TProgress> Progress() const;

        void Completed(Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress> const& handler) const;
        Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress> Completed() const;

        void GetResults() const
        {
            check_hresult(WINRT_SHIM(Windows::Foundation::IAsyncActionWithProgress<TProgress>)->GetResults());
        }

        void get() const
        {
            blocking_suspend(static_cast<Windows::Foundation::IAsyncActionWithProgress<TProgress> const&>(static_cast<D const&>(*this)));
            GetResults();
        }

    };

    template <typename D, typename TResult, typename TProgress> struct consume_IAsyncOperationWithProgress
    {
        void Progress(Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress> const& handler) const;
        Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress> Progress() const;

        void Completed(Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const& handler) const;
        Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> Completed() const;

        TResult GetResults() const
        {
            TResult result = empty_value<TResult>();
            check_hresult(WINRT_SHIM(Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>)->GetResults(put_abi(result)));
            return result;
        }

        TResult get() const
        {
            blocking_suspend(static_cast<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress> const&>(static_cast<D const&>(*this)));
            return GetResults();
        }
    };
}
