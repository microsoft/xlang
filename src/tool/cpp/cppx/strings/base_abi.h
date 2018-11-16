
namespace xlang::impl
{
    template <> struct abi<System::IUnknown>
    {
        struct WINRT_NOVTABLE type
        {
            virtual int32_t WINRT_CALL QueryInterface(guid const& id, void** object) noexcept = 0;
            virtual uint32_t WINRT_CALL AddRef() noexcept = 0;
            virtual uint32_t WINRT_CALL Release() noexcept = 0;
        };
    };

    using unknown_abi = abi_t<System::IUnknown>;

    template <> struct abi<System::IObject>
    {
        struct WINRT_NOVTABLE type : unknown_abi
        {
            virtual int32_t WINRT_CALL GetTypeName(void** name) noexcept = 0;
        };
    };

    using inspectable_abi = abi_t<System::IObject>;

    struct WINRT_NOVTABLE IWeakReference : unknown_abi
    {
        virtual int32_t WINRT_CALL Resolve(guid const& iid, void** objectReference) noexcept = 0;
    };

    struct WINRT_NOVTABLE IWeakReferenceSource : unknown_abi
    {
        virtual int32_t WINRT_CALL GetWeakReference(IWeakReference** weakReference) noexcept = 0;
    };

    struct WINRT_NOVTABLE IRestrictedErrorInfo : unknown_abi
    {
        virtual int32_t WINRT_CALL GetErrorDetails(bstr* description, int32_t* error, bstr* restrictedDescription, bstr* capabilitySid) noexcept = 0;
        virtual int32_t WINRT_CALL GetReference(bstr* reference) noexcept = 0;
    };

    struct WINRT_NOVTABLE ILanguageExceptionErrorInfo : unknown_abi
    {
        virtual int32_t WINRT_CALL GetLanguageException(void** exception) noexcept = 0;
    };

    struct WINRT_NOVTABLE ILanguageExceptionErrorInfo2 : ILanguageExceptionErrorInfo
    {
        virtual int32_t WINRT_CALL GetPreviousLanguageExceptionErrorInfo(ILanguageExceptionErrorInfo2** previous) noexcept = 0;
        virtual int32_t WINRT_CALL CapturePropagationContext(void* exception) noexcept = 0;
        virtual int32_t WINRT_CALL GetPropagationContextHead(ILanguageExceptionErrorInfo2** head) noexcept = 0;
    };

    template <> struct abi<System::IActivationFactory>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL ActivateInstance(void** instance) noexcept = 0;
        };
    };

    template <> struct abi<System::AsyncActionCompletedHandler>
    {
        struct type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(void* asyncInfo, System::AsyncStatus asyncStatus) noexcept = 0;
        };
    };

    template <> struct abi<System::IAsyncAction>
    {
        struct type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_Status(System::AsyncStatus* status) noexcept = 0;
            virtual int32_t WINRT_CALL Cancel() noexcept = 0;
            virtual int32_t WINRT_CALL put_Completed(void* handler) noexcept = 0;
            virtual int32_t WINRT_CALL get_Completed(void** handler) noexcept = 0;
            virtual int32_t WINRT_CALL GetResults() noexcept = 0;
        };
    };

    template <> struct abi<System::IVectorChangedEventArgs>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_CollectionChange(System::CollectionChange* value) noexcept = 0;
            virtual int32_t WINRT_CALL get_Index(uint32_t* value) noexcept = 0;
        };
    };

    template <typename TResult> struct abi<System::AsyncOperationCompletedHandler<TResult>>
    {
        struct type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(void* asyncInfo, System::AsyncStatus status) noexcept = 0;
        };
    };

    template <typename TProgress> struct abi<System::AsyncActionProgressHandler<TProgress>>
    {
        struct type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(void* asyncInfo, arg_in<TProgress> progressInfo) noexcept = 0;
        };
    };

    template <typename TProgress> struct abi<System::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        struct type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(void* asyncInfo, System::AsyncStatus status) noexcept = 0;
        };
    };

    template <typename TResult, typename TProgress> struct abi<System::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        struct type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(void* asyncInfo, arg_in<TProgress> progressInfo) noexcept = 0;
        };
    };

    template <typename TResult, typename TProgress> struct abi<System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        struct type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(void* asyncInfo, System::AsyncStatus status) noexcept = 0;
        };
    };

    template <typename TResult> struct abi<System::IAsyncOperation<TResult>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_Status(System::AsyncStatus* status) noexcept = 0;
            virtual int32_t WINRT_CALL Cancel() noexcept = 0;
            virtual int32_t WINRT_CALL put_Completed(void* handler) noexcept = 0;
            virtual int32_t WINRT_CALL get_Completed(void** handler) noexcept = 0;
            virtual int32_t WINRT_CALL GetResults(arg_out<TResult> results) noexcept = 0;
        };
    };

    template <typename TProgress> struct abi<System::IAsyncActionWithProgress<TProgress>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_Status(System::AsyncStatus* status) noexcept = 0;
            virtual int32_t WINRT_CALL Cancel() noexcept = 0;
            virtual int32_t WINRT_CALL put_Progress(void* handler) noexcept = 0;
            virtual int32_t WINRT_CALL get_Progress(void** handler) noexcept = 0;
            virtual int32_t WINRT_CALL put_Completed(void* handler) noexcept = 0;
            virtual int32_t WINRT_CALL get_Completed(void** handler) noexcept = 0;
            virtual int32_t WINRT_CALL GetResults() noexcept = 0;
        };
    };

    template <typename TResult, typename TProgress> struct abi<System::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_Status(System::AsyncStatus* status) noexcept = 0;
            virtual int32_t WINRT_CALL Cancel() noexcept = 0;
            virtual int32_t WINRT_CALL put_Progress(void* handler) noexcept = 0;
            virtual int32_t WINRT_CALL get_Progress(void** handler) noexcept = 0;
            virtual int32_t WINRT_CALL put_Completed(void* handler) noexcept = 0;
            virtual int32_t WINRT_CALL get_Completed(void** handler) noexcept = 0;
            virtual int32_t WINRT_CALL GetResults(arg_out<TResult> results) noexcept = 0;
        };
    };

    template <typename T> struct abi<System::IReference<T>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_Value(arg_out<T> value) noexcept = 0;
        };
    };

    template <typename T> struct abi<System::IReferenceArray<T>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_Value(uint32_t* __valueSize, arg_out<T>* value) noexcept = 0;
        };
    };

    template <typename K> struct abi<System::IMapChangedEventArgs<K>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_CollectionChange(System::CollectionChange* value) noexcept = 0;
            virtual int32_t WINRT_CALL get_Key(arg_out<K> value) noexcept = 0;
        };
    };

    template <typename T> struct abi<System::VectorChangedEventHandler<T>>
    {
        struct WINRT_NOVTABLE type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(void* sender, void* args) noexcept = 0;
        };
    };

    template <typename K, typename V> struct abi<System::MapChangedEventHandler<K, V>>
    {
        struct WINRT_NOVTABLE type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(void* sender, void* args) noexcept = 0;
        };
    };

    template <typename T> struct abi<System::IIterator<T>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_Current(arg_out<T> current) noexcept = 0;
            virtual int32_t WINRT_CALL get_HasCurrent(bool* hasCurrent) noexcept = 0;
            virtual int32_t WINRT_CALL MoveNext(bool* hasCurrent) noexcept = 0;
            virtual int32_t WINRT_CALL GetMany(uint32_t capacity, arg_out<T> value, uint32_t* actual) noexcept = 0;
        };
    };

    template <typename T> struct abi<System::IIterable<T>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL First(void** first) noexcept = 0;
        };
    };

    template <typename T> struct abi<System::IVectorView<T>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL GetAt(uint32_t index, arg_out<T> item) noexcept = 0;
            virtual int32_t WINRT_CALL get_Size(uint32_t* size) noexcept = 0;
            virtual int32_t WINRT_CALL IndexOf(arg_in<T> value, uint32_t* index, bool* found) noexcept = 0;
            virtual int32_t WINRT_CALL GetMany(uint32_t startIndex, uint32_t capacity, arg_out<T> value, uint32_t* actual) noexcept = 0;
        };
    };

    template <typename T> struct abi<System::IVector<T>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL GetAt(uint32_t index, arg_out<T> item) noexcept = 0;
            virtual int32_t WINRT_CALL get_Size(uint32_t* size) noexcept = 0;
            virtual int32_t WINRT_CALL GetView(void** view) noexcept = 0;
            virtual int32_t WINRT_CALL IndexOf(arg_in<T> value, uint32_t* index, bool* found) noexcept = 0;
            virtual int32_t WINRT_CALL SetAt(uint32_t index, arg_in<T> item) noexcept = 0;
            virtual int32_t WINRT_CALL InsertAt(uint32_t index, arg_in<T> item) noexcept = 0;
            virtual int32_t WINRT_CALL RemoveAt(uint32_t index) noexcept = 0;
            virtual int32_t WINRT_CALL Append(arg_in<T> item) noexcept = 0;
            virtual int32_t WINRT_CALL RemoveAtEnd() noexcept = 0;
            virtual int32_t WINRT_CALL Clear() noexcept = 0;
            virtual int32_t WINRT_CALL GetMany(uint32_t startIndex, uint32_t capacity, arg_out<T> value, uint32_t* actual) noexcept = 0;
            virtual int32_t WINRT_CALL ReplaceAll(uint32_t count, arg_out<T> value) noexcept = 0;
        };
    };

    template <typename T> struct abi<System::IObservableVector<T>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL add_VectorChanged(void* handler, xlang::event_token* token) noexcept = 0;
            virtual int32_t WINRT_CALL remove_VectorChanged(xlang::event_token token) noexcept = 0;
        };
    };

    template <typename K, typename V> struct abi<System::IKeyValuePair<K, V>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL get_Key(arg_out<K> key) noexcept = 0;
            virtual int32_t WINRT_CALL get_Value(arg_out<V> value) noexcept = 0;
        };
    };

    template <typename K, typename V> struct abi<System::IMapView<K, V>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL Lookup(arg_in<K> key, arg_out<V> value) noexcept = 0;
            virtual int32_t WINRT_CALL get_Size(uint32_t* size) noexcept = 0;
            virtual int32_t WINRT_CALL HasKey(arg_in<K> key, bool* found) noexcept = 0;
            virtual int32_t WINRT_CALL Split(void** firstPartition, void** secondPartition) noexcept = 0;
        };
    };

    template <typename K, typename V> struct abi<System::IMap<K, V>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL Lookup(arg_in<K> key, arg_out<V> value) noexcept = 0;
            virtual int32_t WINRT_CALL get_Size(uint32_t* size) noexcept = 0;
            virtual int32_t WINRT_CALL HasKey(arg_in<K> key, bool* found) noexcept = 0;
            virtual int32_t WINRT_CALL GetView(void** view) noexcept = 0;
            virtual int32_t WINRT_CALL Insert(arg_in<K> key, arg_in<V> value, bool* replaced) noexcept = 0;
            virtual int32_t WINRT_CALL Remove(arg_in<K> key) noexcept = 0;
            virtual int32_t WINRT_CALL Clear() noexcept = 0;
        };
    };

    template <typename K, typename V> struct abi<System::IObservableMap<K, V>>
    {
        struct WINRT_NOVTABLE type : inspectable_abi
        {
            virtual int32_t WINRT_CALL add_MapChanged(void* handler, xlang::event_token* token) noexcept = 0;
            virtual int32_t WINRT_CALL remove_MapChanged(xlang::event_token token) noexcept = 0;
        };
    };

    template <typename T> struct abi<System::EventHandler<T>>
    {
        struct WINRT_NOVTABLE type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(void* sender, arg_in<T> args) noexcept = 0;
        };
    };

    template <typename TSender, typename TArgs> struct abi<System::TypedEventHandler<TSender, TArgs>>
    {
        struct WINRT_NOVTABLE type : unknown_abi
        {
            virtual int32_t WINRT_CALL Invoke(arg_in<TSender> sender, arg_in<TArgs> args) noexcept = 0;
        };
    };
}
