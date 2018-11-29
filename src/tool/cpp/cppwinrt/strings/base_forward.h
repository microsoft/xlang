
WINRT_EXPORT namespace winrt::Windows::Foundation
{
    enum class AsyncStatus : int32_t
    {
        Started,
        Completed,
        Canceled,
        Error,
    };

    enum class TrustLevel : int32_t
    {
        BaseTrust,
        PartialTrust,
        FullTrust
    };

    struct IUnknown;
    struct IInspectable;
    struct IActivationFactory;
    struct IAsyncInfo;
    struct IAsyncAction;
    struct AsyncActionCompletedHandler;
    template <typename T> struct IReference;
    template <typename T> struct IReferenceArray;
    template <typename TResult> struct AsyncOperationCompletedHandler;
    template <typename TProgress> struct AsyncActionProgressHandler;
    template <typename TProgress> struct AsyncActionWithProgressCompletedHandler;
    template <typename TResult, typename TProgress> struct AsyncOperationProgressHandler;
    template <typename TResult, typename TProgress> struct AsyncOperationWithProgressCompletedHandler;
    template <typename TResult> struct IAsyncOperation;
    template <typename TProgress> struct IAsyncActionWithProgress;
    template <typename TResult, typename TProgress> struct IAsyncOperationWithProgress;
    template <typename T> struct EventHandler;
    template <typename TSender, typename TArgs> struct TypedEventHandler;
}

WINRT_EXPORT namespace winrt::Windows::Foundation::Collections
{
    enum class CollectionChange : int32_t
    {
        Reset,
        ItemInserted,
        ItemRemoved,
        ItemChanged,
    };

    struct IVectorChangedEventArgs;
    template <typename K> struct IMapChangedEventArgs;
    template <typename T> struct VectorChangedEventHandler;
    template <typename K, typename V> struct MapChangedEventHandler;
    template <typename T> struct IIterator;
    template <typename T> struct IIterable;
    template <typename T> struct IVectorView;
    template <typename T> struct IVector;
    template <typename T> struct IObservableVector;
    template <typename K, typename V> struct IKeyValuePair;
    template <typename K, typename V> struct IMapView;
    template <typename K, typename V> struct IMap;
    template <typename K, typename V> struct IObservableMap;
}

WINRT_EXPORT namespace winrt
{
    struct hresult
    {
        int32_t value{};

        constexpr hresult() noexcept = default;

        constexpr hresult(int32_t const value) noexcept : value(value)
        {
        }

        constexpr operator int32_t() const noexcept
        {
            return value;
        }
    };

    template <typename T>
    using optional = Windows::Foundation::IReference<T>;

    void check_hresult(hresult const result);
    hresult to_hresult() noexcept;

    struct take_ownership_from_abi_t {};
    constexpr take_ownership_from_abi_t take_ownership_from_abi{};

    template <typename T>
    struct com_ptr;
}
