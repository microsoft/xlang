
namespace xlang::impl
{
    template <typename T>
    auto detach_from(T&& object) noexcept
    {
        return detach_abi(std::forward<T>(object));
    }

    template <typename D> struct produce<D, System::IActivationFactory> : produce_base<D, System::IActivationFactory>
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

    template <typename T> struct delegate<System::EventHandler<T>>
    {
        template <typename H>
        struct type final : implements_delegate<System::EventHandler<T>, H>
        {
            type(H&& handler) : implements_delegate<System::EventHandler<T>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<T> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<System::IInspectable const*>(&sender), *reinterpret_cast<T const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TSender, typename TArgs> struct delegate<System::TypedEventHandler<TSender, TArgs>>
    {
        template <typename H>
        struct type final : implements_delegate<System::TypedEventHandler<TSender, TArgs>, H>
        {
            type(H&& handler) : implements_delegate<System::TypedEventHandler<TSender, TArgs>, H>(std::forward<H>(handler)) {}

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

    template <typename D> struct produce<D, System::IAsyncAction> : produce_base<D, System::IAsyncAction>
    {
        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<System::AsyncActionCompletedHandler const*>(&handler));
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
                *handler = detach_from<System::AsyncActionCompletedHandler>(this->shim().Completed());
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

    template <typename D> struct produce<D, System::IAsyncInfo> : produce_base<D, System::IAsyncInfo>
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

        int32_t WINRT_CALL get_Status(xlang::System::AsyncStatus* status) noexcept final
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

    template <typename D, typename TProgress> struct produce<D, System::IAsyncActionWithProgress<TProgress>> : produce_base<D, System::IAsyncActionWithProgress<TProgress>>
    {
        int32_t WINRT_CALL put_Progress(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Progress(*reinterpret_cast<System::AsyncActionProgressHandler<TProgress> const*>(&handler));
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
                *handler = detach_from<System::AsyncActionProgressHandler<TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<System::AsyncActionWithProgressCompletedHandler<TProgress> const*>(&handler));
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
                *handler = detach_from<System::AsyncActionWithProgressCompletedHandler<TProgress>>(this->shim().Completed());
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

    template <typename D, typename TResult> struct produce<D, System::IAsyncOperation<TResult>> : produce_base<D, System::IAsyncOperation<TResult>>
    {
        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<System::AsyncOperationCompletedHandler<TResult> const*>(&handler));
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
                *handler = detach_from<System::AsyncOperationCompletedHandler<TResult>>(this->shim().Completed());
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

    template <typename D, typename TResult, typename TProgress> struct produce<D, System::IAsyncOperationWithProgress<TResult, TProgress>> : produce_base<D, System::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        int32_t WINRT_CALL put_Progress(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Progress(*reinterpret_cast<System::AsyncOperationProgressHandler<TResult, TProgress> const*>(&handler));
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
                *handler = detach_from<System::AsyncOperationProgressHandler<TResult, TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const*>(&handler));
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
                *handler = detach_from<System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>(this->shim().Completed());
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

    template <typename D, typename T> struct produce<D, wfc::IVector<T>> : produce_base<D, wfc::IVector<T>>
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
                *view = detach_from<wfc::IVectorView<T>>(this->shim().GetView());
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

    template <typename D, typename K, typename V> struct produce<D, wfc::IObservableMap<K, V>> : produce_base<D, wfc::IObservableMap<K, V>>
    {
        int32_t WINRT_CALL add_MapChanged(void* handler, xlang::event_token* token) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *token = detach_from<event_token>(this->shim().MapChanged(*reinterpret_cast<wfc::MapChangedEventHandler<K, V> const*>(&handler)));
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
    struct produce<D, wfc::IObservableVector<T>> : produce_base<D, wfc::IObservableVector<T>>
    {
        int32_t WINRT_CALL add_VectorChanged(void* handler, xlang::event_token* token) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *token = this->shim().VectorChanged(*reinterpret_cast<wfc::VectorChangedEventHandler<T> const*>(&handler));
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

    template <> struct delegate<System::AsyncActionCompletedHandler>
    {
        template <typename H>
        struct type final : implements_delegate<System::AsyncActionCompletedHandler, H>
        {
            type(H&& handler) : implements_delegate<System::AsyncActionCompletedHandler, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* asyncInfo, System::AsyncStatus asyncStatus) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<System::IAsyncAction const*>(&asyncInfo), asyncStatus);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult> struct delegate<System::AsyncOperationCompletedHandler<TResult>>
    {
        template <typename H>
        struct type final : implements_delegate<System::AsyncOperationCompletedHandler<TResult>, H>
        {
            type(H&& handler) : implements_delegate<System::AsyncOperationCompletedHandler<TResult>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, System::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<System::IAsyncOperation<TResult> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<System::AsyncActionProgressHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<System::AsyncActionProgressHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<System::AsyncActionProgressHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<System::IAsyncActionWithProgress<TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<System::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<System::AsyncActionWithProgressCompletedHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<System::AsyncActionWithProgressCompletedHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, System::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<System::IAsyncActionWithProgress<TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<System::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<System::AsyncOperationProgressHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<System::AsyncOperationProgressHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<System::IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, System::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<System::IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };
}
