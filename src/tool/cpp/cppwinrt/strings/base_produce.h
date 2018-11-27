
namespace winrt::impl
{
    template <typename T>
    auto detach_from(T&& object) noexcept
    {
        return detach_abi(std::forward<T>(object));
    }

    template <typename D> struct produce<D, Windows::Foundation::IActivationFactory> : produce_base<D, Windows::Foundation::IActivationFactory>
    {
        int32_t WINRT_CALL ActivateInstance(void** instance) noexcept final
        {
            try
            {
                *instance = nullptr;
                typename D::abi_guard guard(this->shim());
                *instance = detach_abi(this->shim().ActivateInstance());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename T> struct delegate<wfc::VectorChangedEventHandler<T>>
    {
        template <typename H>
        struct type final : implements_delegate<wfc::VectorChangedEventHandler<T>, H>
        {
            type(H&& handler) : implements_delegate<wfc::VectorChangedEventHandler<T>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, void* args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<wfc::IObservableVector<T> const*>(&sender), *reinterpret_cast<wfc::IVectorChangedEventArgs const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename K, typename V> struct delegate<wfc::MapChangedEventHandler<K, V>>
    {
        template <typename H>
        struct type final : implements_delegate<wfc::MapChangedEventHandler<K, V>, H>
        {
            type(H&& handler) : implements_delegate<wfc::MapChangedEventHandler<K, V>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, void* args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<wfc::IObservableMap<K, V> const*>(&sender), *reinterpret_cast<wfc::IMapChangedEventArgs<K> const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename T> struct delegate<Windows::Foundation::EventHandler<T>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::EventHandler<T>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::EventHandler<T>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<T> args) noexcept final
            {
                try
                {
                    H::operator()(*reinterpret_cast<Windows::Foundation::IInspectable const*>(&sender), *reinterpret_cast<T const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TSender, typename TArgs> struct delegate<Windows::Foundation::TypedEventHandler<TSender, TArgs>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::TypedEventHandler<TSender, TArgs>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::TypedEventHandler<TSender, TArgs>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(arg_in<TSender> sender, arg_in<TArgs> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<TSender const*>(&sender), *reinterpret_cast<TArgs const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename D> struct produce<D, Windows::Foundation::IAsyncAction> : produce_base<D, Windows::Foundation::IAsyncAction>
    {
        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Windows::Foundation::AsyncActionCompletedHandler const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Completed(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncActionCompletedHandler>(this->shim().Completed());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetResults() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().GetResults();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D> struct produce<D, Windows::Foundation::IAsyncInfo> : produce_base<D, Windows::Foundation::IAsyncInfo>
    {
        int32_t WINRT_CALL get_Id(uint32_t* id) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *id = this->shim().Id();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Status(winrt::Windows::Foundation::AsyncStatus* status) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *status = this->shim().Status();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_ErrorCode(int32_t* errorCode) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *errorCode = this->shim().ErrorCode();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL Cancel() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Cancel();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL Close() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Close();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename TProgress> struct produce<D, Windows::Foundation::IAsyncActionWithProgress<TProgress>> : produce_base<D, Windows::Foundation::IAsyncActionWithProgress<TProgress>>
    {
        int32_t WINRT_CALL put_Progress(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Progress(*reinterpret_cast<Windows::Foundation::AsyncActionProgressHandler<TProgress> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Progress(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncActionProgressHandler<TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Completed(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>>(this->shim().Completed());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetResults() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().GetResults();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename TResult> struct produce<D, Windows::Foundation::IAsyncOperation<TResult>> : produce_base<D, Windows::Foundation::IAsyncOperation<TResult>>
    {
        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Windows::Foundation::AsyncOperationCompletedHandler<TResult> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Completed(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncOperationCompletedHandler<TResult>>(this->shim().Completed());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetResults(arg_out<TResult> results) noexcept final
        {
            try
            {
                clear_abi(results);
                typename D::abi_guard guard(this->shim());
                *results = detach_from<TResult>(this->shim().GetResults());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename TResult, typename TProgress> struct produce<D, Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>> : produce_base<D, Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        int32_t WINRT_CALL put_Progress(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Progress(*reinterpret_cast<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Progress(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Completed(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>(this->shim().Completed());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetResults(arg_out<TResult> results) noexcept final
        {
            try
            {
                clear_abi(results);
                typename D::abi_guard guard(this->shim());
                *results = detach_from<TResult>(this->shim().GetResults());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };


    template <typename D> struct produce<D, wfc::IVectorChangedEventArgs> : produce_base<D, wfc::IVectorChangedEventArgs>
    {
        int32_t WINRT_CALL get_CollectionChange(wfc::CollectionChange* value) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *value = this->shim().CollectionChange();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Index(uint32_t* value) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *value = this->shim().Index();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename T> struct produce<D, wfc::IIterator<T>> : produce_base<D, wfc::IIterator<T>>
    {
        int32_t WINRT_CALL get_Current(arg_out<T> current) noexcept final
        {
            try
            {
                clear_abi(current);
                typename D::abi_guard guard(this->shim());
                *current = detach_from<T>(this->shim().Current());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_HasCurrent(bool* hasCurrent) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *hasCurrent = this->shim().HasCurrent();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL MoveNext(bool* hasCurrent) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *hasCurrent = this->shim().MoveNext();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetMany(uint32_t capacity, arg_out<T> value, uint32_t* actual) noexcept final
        {
            try
            {
                clear_abi(value);
                typename D::abi_guard guard(this->shim());
                *actual = this->shim().GetMany(array_view<T>(reinterpret_cast<T*>(value), reinterpret_cast<T*>(value) + capacity));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename T> struct produce<D, wfc::IIterable<T>> : produce_base<D, wfc::IIterable<T>>
    {
        int32_t WINRT_CALL First(void** first) noexcept final
        {
            try
            {
                *first = nullptr;
                typename D::abi_guard guard(this->shim());
                *first = detach_from<wfc::IIterator<T>>(this->shim().First());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename K, typename V> struct produce<D, wfc::IKeyValuePair<K, V>> : produce_base<D, wfc::IKeyValuePair<K, V>>
    {
        int32_t WINRT_CALL get_Key(arg_out<K> key) noexcept final
        {
            try
            {
                clear_abi(key);
                typename D::abi_guard guard(this->shim());
                *key = detach_from<K>(this->shim().Key());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Value(arg_out<V> value) noexcept final
        {
            try
            {
                clear_abi(value);
                typename D::abi_guard guard(this->shim());
                *value = detach_from<V>(this->shim().Value());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename T> struct produce<D, wfc::IVectorView<T>> : produce_base<D, wfc::IVectorView<T>>
    {
        int32_t WINRT_CALL GetAt(uint32_t index, arg_out<T> item) noexcept final
        {
            try
            {
                clear_abi(item);
                typename D::abi_guard guard(this->shim());
                *item = detach_from<T>(this->shim().GetAt(index));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Size(uint32_t* size) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *size = this->shim().Size();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL IndexOf(arg_in<T> value, uint32_t* index, bool* found) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *found = this->shim().IndexOf(*reinterpret_cast<T const*>(&value), *index);
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetMany(uint32_t startIndex, uint32_t capacity, arg_out<T> value, uint32_t* actual) noexcept final
        {
            try
            {
                clear_abi(value);
                typename D::abi_guard guard(this->shim());
                *actual = this->shim().GetMany(startIndex, array_view<T>(reinterpret_cast<T*>(value), reinterpret_cast<T*>(value) + capacity));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename K, typename V> struct produce<D, wfc::IMapView<K, V>> : produce_base<D, wfc::IMapView<K, V>>
    {
        int32_t WINRT_CALL Lookup(arg_in<K> key, arg_out<V> value) noexcept final
        {
            try
            {
                clear_abi(value);
                typename D::abi_guard guard(this->shim());
                *value = detach_from<V>(this->shim().Lookup(*reinterpret_cast<K const*>(&key)));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Size(uint32_t* size) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *size = this->shim().Size();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL HasKey(arg_in<K> key, bool* found) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *found = this->shim().HasKey(*reinterpret_cast<K const*>(&key));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL Split(void** firstPartition, void** secondPartition) noexcept final
        {
            try
            {
                *firstPartition = nullptr;
                *secondPartition = nullptr;
                typename D::abi_guard guard(this->shim());
                this->shim().Split(*reinterpret_cast<wfc::IMapView<K, V>*>(firstPartition), *reinterpret_cast<wfc::IMapView<K, V>*>(secondPartition));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename K, typename V> struct produce<D, wfc::IMap<K, V>> : produce_base<D, wfc::IMap<K, V>>
    {
        int32_t WINRT_CALL Lookup(arg_in<K> key, arg_out<V> value) noexcept final
        {
            try
            {
                clear_abi(value);
                typename D::abi_guard guard(this->shim());
                *value = detach_from<V>(this->shim().Lookup(*reinterpret_cast<K const*>(&key)));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Size(uint32_t* size) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *size = this->shim().Size();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL HasKey(arg_in<K> key, bool* found) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *found = this->shim().HasKey(*reinterpret_cast<K const*>(&key));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetView(void** view) noexcept final
        {
            try
            {
                *view = nullptr;
                typename D::abi_guard guard(this->shim());
                *view = detach_from<wfc::IMapView<K, V>>(this->shim().GetView());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL Insert(arg_in<K> key, arg_in<V> value, bool* replaced) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *replaced = this->shim().Insert(*reinterpret_cast<K const*>(&key), *reinterpret_cast<V const*>(&value));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL Remove(arg_in<K> key) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Remove(*reinterpret_cast<K const*>(&key));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL Clear() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Clear();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename K> struct produce<D, wfc::IMapChangedEventArgs<K>> : produce_base<D, wfc::IMapChangedEventArgs<K>>
    {
        int32_t WINRT_CALL get_CollectionChange(wfc::CollectionChange* value) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *value = this->shim().CollectionChange();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Key(arg_out<K> value) noexcept final
        {
            try
            {
                clear_abi(value);
                typename D::abi_guard guard(this->shim());
                *value = detach_from<K>(this->shim().Key());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <> struct delegate<Windows::Foundation::AsyncActionCompletedHandler>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncActionCompletedHandler, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncActionCompletedHandler, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* asyncInfo, Windows::Foundation::AsyncStatus asyncStatus) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncAction const*>(&asyncInfo), asyncStatus);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult> struct delegate<Windows::Foundation::AsyncOperationCompletedHandler<TResult>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncOperationCompletedHandler<TResult>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncOperationCompletedHandler<TResult>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, Windows::Foundation::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncOperation<TResult> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<Windows::Foundation::AsyncActionProgressHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncActionProgressHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncActionProgressHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncActionWithProgress<TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, Windows::Foundation::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncActionWithProgress<TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, Windows::Foundation::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };
}
