
namespace Windows { namespace Foundation { namespace Collections
{
    struct IVectorChangedEventArgs;

    template <class T>
    /*delegate*/ struct VectorChangedEventHandler_impl : IUnknown
    {
    private:
        typedef typename Windows::Foundation::Internal::GetLogicalType<T>::type T_logical;
    public:
        typedef T                                                               T_complex;

        virtual HRESULT STDMETHODCALLTYPE Invoke(_In_opt_ IObservableVector<T_logical>* sender, _In_opt_ IVectorChangedEventArgs* e) = 0;
    };

    template <class T>
    struct IObservableVector_impl : IInspectable /* requires IVector<T> */
    {
    private:
        typedef typename Windows::Foundation::Internal::GetLogicalType<T>::type T_logical;
    public:
        typedef T                                                               T_complex;

        virtual /*eventadd*/ HRESULT STDMETHODCALLTYPE add_VectorChanged (_In_opt_ VectorChangedEventHandler<T_logical>* handler, _Out_ EventRegistrationToken*  token) = 0;
        virtual /*eventremove*/ HRESULT STDMETHODCALLTYPE remove_VectorChanged(_In_ EventRegistrationToken  token) = 0;
    };

    template <class K>
    struct IMapChangedEventArgs_impl : IInspectable
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<K>::type K_abi;
    public:
        typedef K                                                           K_complex;

        virtual /*propget*/ HRESULT STDMETHODCALLTYPE get_CollectionChange (_Out_ CollectionChange* value) = 0;
        virtual /*propget*/ HRESULT STDMETHODCALLTYPE get_Key (_Out_ K_abi* value) = 0;
    };

    template <class K, class V>
    /*delegate*/ struct MapChangedEventHandler_impl : IUnknown
    {
    private:
        typedef typename Windows::Foundation::Internal::GetLogicalType<K>::type K_logical;
        typedef typename Windows::Foundation::Internal::GetLogicalType<V>::type V_logical;
    public:
        typedef K                                                               K_complex;
        typedef V                                                               V_complex;

        virtual HRESULT STDMETHODCALLTYPE Invoke(_In_opt_ IObservableMap<K_logical, V_logical>* sender, _In_opt_ IMapChangedEventArgs<K_logical>* e) = 0;
    };

    template <class K, class V>
    struct IObservableMap_impl : IInspectable /* requires IMap<K,V> */
    {
    private:
        typedef typename Windows::Foundation::Internal::GetLogicalType<K>::type K_logical;
        typedef typename Windows::Foundation::Internal::GetLogicalType<V>::type V_logical;
    public:
        typedef K                                                               K_complex;
        typedef V                                                               V_complex;

        virtual /*eventadd*/ HRESULT STDMETHODCALLTYPE add_MapChanged (_In_opt_ MapChangedEventHandler<K_logical, V_logical>* handler, _Out_ EventRegistrationToken*  token) = 0;
        virtual /*eventremove*/ HRESULT STDMETHODCALLTYPE remove_MapChanged(_In_ EventRegistrationToken  token) = 0;
    };
}}} // namespace Windows::Foundation::Collections
