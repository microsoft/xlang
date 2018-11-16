
namespace xlang::impl
{
    template <typename T>
    auto detach_from(T&& object) noexcept
    {
        return detach_abi(std::forward<T>(object));
    }

    template <typename D> struct produce<D, IActivationFactory> : produce_base<D, IActivationFactory>
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

    template <typename T> struct delegate<VectorChangedEventHandler<T>>
    {
        template <typename H>
        struct type final : implements_delegate<VectorChangedEventHandler<T>, H>
        {
            type(H&& handler) : implements_delegate<VectorChangedEventHandler<T>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, void* args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<IObservableVector<T> const*>(&sender), *reinterpret_cast<IVectorChangedEventArgs const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename K, typename V> struct delegate<MapChangedEventHandler<K, V>>
    {
        template <typename H>
        struct type final : implements_delegate<MapChangedEventHandler<K, V>, H>
        {
            type(H&& handler) : implements_delegate<MapChangedEventHandler<K, V>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, void* args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<IObservableMap<K, V> const*>(&sender), *reinterpret_cast<IMapChangedEventArgs<K> const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename T> struct delegate<EventHandler<T>>
    {
        template <typename H>
        struct type final : implements_delegate<EventHandler<T>, H>
        {
            type(H&& handler) : implements_delegate<EventHandler<T>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<T> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<IObject const*>(&sender), *reinterpret_cast<T const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TSender, typename TArgs> struct delegate<TypedEventHandler<TSender, TArgs>>
    {
        template <typename H>
        struct type final : implements_delegate<TypedEventHandler<TSender, TArgs>, H>
        {
            type(H&& handler) : implements_delegate<TypedEventHandler<TSender, TArgs>, H>(std::forward<H>(handler)) {}

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

    template <typename D> struct produce<D, IAsyncAction> : produce_base<D, IAsyncAction>
    {
        int32_t WINRT_CALL get_Status(xlang::AsyncStatus* status) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *status = this->shim().Status();
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

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<AsyncActionCompletedHandler const*>(&handler));
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
                *handler = detach_from<AsyncActionCompletedHandler>(this->shim().Completed());
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

    template <typename D, typename TProgress> struct produce<D, IAsyncActionWithProgress<TProgress>> : produce_base<D, IAsyncActionWithProgress<TProgress>>
    {
        int32_t WINRT_CALL get_Status(xlang::AsyncStatus* status) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *status = this->shim().Status();
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

        int32_t WINRT_CALL put_Progress(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Progress(*reinterpret_cast<AsyncActionProgressHandler<TProgress> const*>(&handler));
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
                *handler = detach_from<AsyncActionProgressHandler<TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<AsyncActionWithProgressCompletedHandler<TProgress> const*>(&handler));
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
                *handler = detach_from<AsyncActionWithProgressCompletedHandler<TProgress>>(this->shim().Completed());
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

    template <typename D, typename TResult> struct produce<D, IAsyncOperation<TResult>> : produce_base<D, IAsyncOperation<TResult>>
    {
        int32_t WINRT_CALL get_Status(xlang::AsyncStatus* status) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *status = this->shim().Status();
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

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<AsyncOperationCompletedHandler<TResult> const*>(&handler));
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
                *handler = detach_from<AsyncOperationCompletedHandler<TResult>>(this->shim().Completed());
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

    template <typename D, typename TResult, typename TProgress> struct produce<D, IAsyncOperationWithProgress<TResult, TProgress>> : produce_base<D, IAsyncOperationWithProgress<TResult, TProgress>>
    {
        int32_t WINRT_CALL get_Status(xlang::AsyncStatus* status) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *status = this->shim().Status();
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

        int32_t WINRT_CALL put_Progress(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Progress(*reinterpret_cast<AsyncOperationProgressHandler<TResult, TProgress> const*>(&handler));
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
                *handler = detach_from<AsyncOperationProgressHandler<TResult, TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const*>(&handler));
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
                *handler = detach_from<AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>(this->shim().Completed());
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


    template <typename D> struct produce<D, IVectorChangedEventArgs> : produce_base<D, IVectorChangedEventArgs>
    {
        int32_t WINRT_CALL get_CollectionChange(CollectionChange* value) noexcept final
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

    template <typename D, typename T> struct produce<D, IIterator<T>> : produce_base<D, IIterator<T>>
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

    template <typename D, typename T> struct produce<D, IIterable<T>> : produce_base<D, IIterable<T>>
    {
        int32_t WINRT_CALL First(void** first) noexcept final
        {
            try
            {
                *first = nullptr;
                typename D::abi_guard guard(this->shim());
                *first = detach_from<IIterator<T>>(this->shim().First());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename K, typename V> struct produce<D, IKeyValuePair<K, V>> : produce_base<D, IKeyValuePair<K, V>>
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

    template <typename D, typename T> struct produce<D, IVectorView<T>> : produce_base<D, IVectorView<T>>
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

    template <typename D, typename T> struct produce<D, IVector<T>> : produce_base<D, IVector<T>>
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

        int32_t WINRT_CALL GetView(void** view) noexcept final
        {
            try
            {
                *view = nullptr;
                typename D::abi_guard guard(this->shim());
                *view = detach_from<IVectorView<T>>(this->shim().GetView());
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

        int32_t WINRT_CALL SetAt(uint32_t index, arg_in<T> item) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().SetAt(index, *reinterpret_cast<T const*>(&item));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL InsertAt(uint32_t index, arg_in<T> item) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().InsertAt(index, *reinterpret_cast<T const*>(&item));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL RemoveAt(uint32_t index) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().RemoveAt(index);
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL Append(arg_in<T> item) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Append(*reinterpret_cast<T const*>(&item));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL RemoveAtEnd() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().RemoveAtEnd();
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

        int32_t WINRT_CALL ReplaceAll(uint32_t count, arg_out<T> item) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().ReplaceAll(array_view<T const>(reinterpret_cast<T const*>(item), reinterpret_cast<T const*>(item) + count));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename K, typename V> struct produce<D, IMapView<K, V>> : produce_base<D, IMapView<K, V>>
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
                this->shim().Split(*reinterpret_cast<IMapView<K, V>*>(firstPartition), *reinterpret_cast<IMapView<K, V>*>(secondPartition));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename K, typename V> struct produce<D, IMap<K, V>> : produce_base<D, IMap<K, V>>
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
                *view = detach_from<IMapView<K, V>>(this->shim().GetView());
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

    template <typename D, typename K> struct produce<D, IMapChangedEventArgs<K>> : produce_base<D, IMapChangedEventArgs<K>>
    {
        int32_t WINRT_CALL get_CollectionChange(CollectionChange* value) noexcept final
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

    template <typename D, typename K, typename V> struct produce<D, IObservableMap<K, V>> : produce_base<D, IObservableMap<K, V>>
    {
        int32_t WINRT_CALL add_MapChanged(void* handler, xlang::event_token* token) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *token = detach_from<event_token>(this->shim().MapChanged(*reinterpret_cast<MapChangedEventHandler<K, V> const*>(&handler)));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL remove_MapChanged(xlang::event_token token) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().MapChanged(token);
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename T>
    struct produce<D, IObservableVector<T>> : produce_base<D, IObservableVector<T>>
    {
        int32_t WINRT_CALL add_VectorChanged(void* handler, xlang::event_token* token) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *token = this->shim().VectorChanged(*reinterpret_cast<VectorChangedEventHandler<T> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL remove_VectorChanged(xlang::event_token token) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().VectorChanged(token);
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <> struct delegate<AsyncActionCompletedHandler>
    {
        template <typename H>
        struct type final : implements_delegate<AsyncActionCompletedHandler, H>
        {
            type(H&& handler) : implements_delegate<AsyncActionCompletedHandler, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* asyncInfo, AsyncStatus asyncStatus) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<IAsyncAction const*>(&asyncInfo), asyncStatus);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult> struct delegate<AsyncOperationCompletedHandler<TResult>>
    {
        template <typename H>
        struct type final : implements_delegate<AsyncOperationCompletedHandler<TResult>, H>
        {
            type(H&& handler) : implements_delegate<AsyncOperationCompletedHandler<TResult>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<IAsyncOperation<TResult> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<AsyncActionProgressHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<AsyncActionProgressHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<AsyncActionProgressHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<IAsyncActionWithProgress<TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<AsyncActionWithProgressCompletedHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<AsyncActionWithProgressCompletedHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<IAsyncActionWithProgress<TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<AsyncOperationProgressHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<AsyncOperationProgressHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<AsyncOperationProgressHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };
}
