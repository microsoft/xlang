
namespace xlang::impl
{
    template <typename T>
    auto detach_from(T&& object) noexcept
    {
        return detach_abi(std::forward<T>(object));
    }

    template <typename D> struct produce<D, Runtime::IActivationFactory> : produce_base<D, Runtime::IActivationFactory>
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

    template <typename T> struct delegate<Runtime::VectorChangedEventHandler<T>>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::VectorChangedEventHandler<T>, H>
        {
            type(H&& handler) : implements_delegate<Runtime::VectorChangedEventHandler<T>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, void* args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Runtime::IObservableVector<T> const*>(&sender), *reinterpret_cast<Runtime::IVectorChangedEventArgs const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename K, typename V> struct delegate<Runtime::MapChangedEventHandler<K, V>>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::MapChangedEventHandler<K, V>, H>
        {
            type(H&& handler) : implements_delegate<Runtime::MapChangedEventHandler<K, V>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, void* args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Runtime::IObservableMap<K, V> const*>(&sender), *reinterpret_cast<Runtime::IMapChangedEventArgs<K> const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename T> struct delegate<Runtime::EventHandler<T>>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::EventHandler<T>, H>
        {
            type(H&& handler) : implements_delegate<Runtime::EventHandler<T>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<T> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Runtime::IObject const*>(&sender), *reinterpret_cast<T const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TSender, typename TArgs> struct delegate<Runtime::TypedEventHandler<TSender, TArgs>>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::TypedEventHandler<TSender, TArgs>, H>
        {
            type(H&& handler) : implements_delegate<Runtime::TypedEventHandler<TSender, TArgs>, H>(std::forward<H>(handler)) {}

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

    template <typename D> struct produce<D, Runtime::IAsyncAction> : produce_base<D, Runtime::IAsyncAction>
    {
        int32_t WINRT_CALL get_Status(xlang::Runtime::AsyncStatus* status) noexcept final
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
                this->shim().Completed(*reinterpret_cast<Runtime::AsyncActionCompletedHandler const*>(&handler));
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
                *handler = detach_from<Runtime::AsyncActionCompletedHandler>(this->shim().Completed());
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

    template <typename D, typename TProgress> struct produce<D, Runtime::IAsyncActionWithProgress<TProgress>> : produce_base<D, Runtime::IAsyncActionWithProgress<TProgress>>
    {
        int32_t WINRT_CALL get_Status(xlang::Runtime::AsyncStatus* status) noexcept final
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
                this->shim().Progress(*reinterpret_cast<Runtime::AsyncActionProgressHandler<TProgress> const*>(&handler));
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
                *handler = detach_from<Runtime::AsyncActionProgressHandler<TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Runtime::AsyncActionWithProgressCompletedHandler<TProgress> const*>(&handler));
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
                *handler = detach_from<Runtime::AsyncActionWithProgressCompletedHandler<TProgress>>(this->shim().Completed());
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

    template <typename D, typename TResult> struct produce<D, Runtime::IAsyncOperation<TResult>> : produce_base<D, Runtime::IAsyncOperation<TResult>>
    {
        int32_t WINRT_CALL get_Status(xlang::Runtime::AsyncStatus* status) noexcept final
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
                this->shim().Completed(*reinterpret_cast<Runtime::AsyncOperationCompletedHandler<TResult> const*>(&handler));
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
                *handler = detach_from<Runtime::AsyncOperationCompletedHandler<TResult>>(this->shim().Completed());
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

    template <typename D, typename TResult, typename TProgress> struct produce<D, Runtime::IAsyncOperationWithProgress<TResult, TProgress>> : produce_base<D, Runtime::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        int32_t WINRT_CALL get_Status(xlang::Runtime::AsyncStatus* status) noexcept final
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
                this->shim().Progress(*reinterpret_cast<Runtime::AsyncOperationProgressHandler<TResult, TProgress> const*>(&handler));
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
                *handler = detach_from<Runtime::AsyncOperationProgressHandler<TResult, TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const*>(&handler));
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
                *handler = detach_from<Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>(this->shim().Completed());
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


    template <typename D> struct produce<D, Runtime::IVectorChangedEventArgs> : produce_base<D, Runtime::IVectorChangedEventArgs>
    {
        int32_t WINRT_CALL get_CollectionChange(Runtime::CollectionChange* value) noexcept final
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

    template <typename D, typename T> struct produce<D, Runtime::IIterator<T>> : produce_base<D, Runtime::IIterator<T>>
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

    template <typename D, typename T> struct produce<D, Runtime::IIterable<T>> : produce_base<D, Runtime::IIterable<T>>
    {
        int32_t WINRT_CALL First(void** first) noexcept final
        {
            try
            {
                *first = nullptr;
                typename D::abi_guard guard(this->shim());
                *first = detach_from<Runtime::IIterator<T>>(this->shim().First());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename K, typename V> struct produce<D, Runtime::IKeyValuePair<K, V>> : produce_base<D, Runtime::IKeyValuePair<K, V>>
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

    template <typename D, typename T> struct produce<D, Runtime::IVectorView<T>> : produce_base<D, Runtime::IVectorView<T>>
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

    template <typename D, typename T> struct produce<D, Runtime::IVector<T>> : produce_base<D, Runtime::IVector<T>>
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
                *view = detach_from<Runtime::IVectorView<T>>(this->shim().GetView());
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

    template <typename D, typename K, typename V> struct produce<D, Runtime::IMapView<K, V>> : produce_base<D, Runtime::IMapView<K, V>>
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
                this->shim().Split(*reinterpret_cast<Runtime::IMapView<K, V>*>(firstPartition), *reinterpret_cast<Runtime::IMapView<K, V>*>(secondPartition));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename K, typename V> struct produce<D, Runtime::IMap<K, V>> : produce_base<D, Runtime::IMap<K, V>>
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
                *view = detach_from<Runtime::IMapView<K, V>>(this->shim().GetView());
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

    template <typename D, typename K> struct produce<D, Runtime::IMapChangedEventArgs<K>> : produce_base<D, Runtime::IMapChangedEventArgs<K>>
    {
        int32_t WINRT_CALL get_CollectionChange(Runtime::CollectionChange* value) noexcept final
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

    template <typename D, typename K, typename V> struct produce<D, Runtime::IObservableMap<K, V>> : produce_base<D, Runtime::IObservableMap<K, V>>
    {
        int32_t WINRT_CALL add_MapChanged(void* handler, xlang::event_token* token) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *token = detach_from<event_token>(this->shim().MapChanged(*reinterpret_cast<Runtime::MapChangedEventHandler<K, V> const*>(&handler)));
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
    struct produce<D, Runtime::IObservableVector<T>> : produce_base<D, Runtime::IObservableVector<T>>
    {
        int32_t WINRT_CALL add_VectorChanged(void* handler, xlang::event_token* token) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *token = this->shim().VectorChanged(*reinterpret_cast<Runtime::VectorChangedEventHandler<T> const*>(&handler));
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

    template <> struct delegate<Runtime::AsyncActionCompletedHandler>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::AsyncActionCompletedHandler, H>
        {
            type(H&& handler) : implements_delegate<Runtime::AsyncActionCompletedHandler, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* asyncInfo, Runtime::AsyncStatus asyncStatus) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Runtime::IAsyncAction const*>(&asyncInfo), asyncStatus);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult> struct delegate<Runtime::AsyncOperationCompletedHandler<TResult>>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::AsyncOperationCompletedHandler<TResult>, H>
        {
            type(H&& handler) : implements_delegate<Runtime::AsyncOperationCompletedHandler<TResult>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, Runtime::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Runtime::IAsyncOperation<TResult> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<Runtime::AsyncActionProgressHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::AsyncActionProgressHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Runtime::AsyncActionProgressHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Runtime::IAsyncActionWithProgress<TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<Runtime::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::AsyncActionWithProgressCompletedHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Runtime::AsyncActionWithProgressCompletedHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, Runtime::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Runtime::IAsyncActionWithProgress<TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<Runtime::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::AsyncOperationProgressHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Runtime::AsyncOperationProgressHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Runtime::IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, Runtime::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Runtime::IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };
}
