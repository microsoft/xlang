
namespace Windows { namespace Foundation { namespace Collections
{
    template <class T>
    struct is_pointer { enum { value = false };};
    template <class T>
    struct is_pointer<T*> { enum { value = true};};

    template <class T>
    struct is_foundation_struct { enum {value = false};};
    template <>
    struct is_foundation_struct<GUID> { enum { value = true};};
    template <>
    struct is_foundation_struct<Windows::Foundation::DateTime> { enum { value = true};};
    template <>
    struct is_foundation_struct<Windows::Foundation::TimeSpan> { enum { value = true};};
    template <>
    struct is_foundation_struct<Windows::Foundation::Point> { enum { value = true};};
    template <>
    struct is_foundation_struct<Windows::Foundation::Size> { enum { value = true};};
    template <>
    struct is_foundation_struct<Windows::Foundation::Rect> { enum { value = true};};

    template <class T>
    struct supports_cleanup {
      typedef typename Windows::Foundation::Internal::GetAbiType<T>::type _abi_type;
      enum { value = is_pointer< _abi_type >::value || is_foundation_struct< _abi_type >::value || !__is_class(_abi_type) };
    };

    template <class T, bool isStruct = supports_cleanup<T>::value>
    struct IIterator_impl;

    template <class T>
    struct IIterable_impl;

    template <class T, bool isStruct = supports_cleanup<T>::value>
    struct IVectorView_impl;

    template <class T, bool isStruct = supports_cleanup<T>::value>
    struct IVector_impl;

    template <class K, class V>
    struct IKeyValuePair_impl;

    template <class K, class V>
    struct IMapView_impl;

    template <class K, class V>
    struct IMap_impl;

    enum CollectionsChange : int;

    struct IVectorChangedEventArgs;

    template <class T>
    /*delegate*/ struct VectorChangedEventHandler_impl;

    template <class T>
    struct IObservableVector_impl;


    template <class K>
    struct IMapChangedEventArgs_impl;

    template <class K, class V>
    struct /*delegate*/ MapChangedEventHandler_impl;

    template <class K, class V>
    struct IObservableMap_impl;


    namespace detail
    {
        template <class T>
        struct not_yet_specialized_placeholder
        {
            enum { value = false };
        };

        template <class WasNotSpecialized>
        struct not_yet_specialized
        {
            static_assert(
                not_yet_specialized_placeholder<WasNotSpecialized>::value,
                "This interface instance has not been specialized by MIDL."
                " This may be caused by forgetting a '*' pointer on an interface"
                " type, by omitting a necessary 'declare' clause in your idl"
                " file, by forgetting to include one of the necessary MIDL"
                " generated headers.");
        };
    }

    template <class T>
    struct IIterator
        : IIterator_impl<T>
        , detail::not_yet_specialized<IIterator<T>>
    {
    };

    template <class T>
    struct IIterable
        : IIterable_impl<T>
        , detail::not_yet_specialized<IIterable<T>>
    {
    };

    template <class T>
    struct IVectorView
        : IVectorView_impl<T>
        , detail::not_yet_specialized<IVectorView<T>>
    {
    };

    template <class T>
    struct IVector
        : IVector_impl<T>
        , detail::not_yet_specialized<IVector<T>>
    {
    };

    template <class K, class V>
    struct IKeyValuePair
        : IKeyValuePair_impl<K, V>
        , detail::not_yet_specialized<IKeyValuePair<K, V>>
    {
    };

    template <class K, class V>
    struct IMapView
        : IMapView_impl<K, V>
        , detail::not_yet_specialized<IMapView<K, V>>
    {
    };

    template <class K, class V>
    struct IMap
        : IMap_impl<K, V>
        , detail::not_yet_specialized<IMap<K, V>>
    {
    };

    template <class T>
    /*delegate*/ struct VectorChangedEventHandler
        : VectorChangedEventHandler_impl<T>
        , detail::not_yet_specialized<VectorChangedEventHandler<T>>
    {
    };

    template <class T>
    struct IObservableVector
        : IObservableVector_impl<T>
        , detail::not_yet_specialized<IObservableVector<T>>
    {
    };


    template <class K>
    struct IMapChangedEventArgs
        : IMapChangedEventArgs_impl<K>
        , detail::not_yet_specialized<IMapChangedEventArgs<K>>
    {
    };

    template <class K, class V>
    /*delegate*/ struct MapChangedEventHandler
        : MapChangedEventHandler_impl<K, V>
        , detail::not_yet_specialized<MapChangedEventHandler<K, V>>
    {
    };

    template <class K, class V>
    struct IObservableMap
        : IObservableMap_impl<K, V>
        , detail::not_yet_specialized<IObservableMap<K, V>>
    {
    };



    // Template definitions duplicate the members that MIDL generates, without the name redirection.

    template <class T>
    struct IIterable_impl : IInspectable
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<T>::type     T_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<T>::type T_logical;
    public:

        // For all types which are neither InterfaceGroups nor RuntimeClasses, the
        // following three typedefs are synonyms for a single C++ type.  But for
        // InterfaceGroups and RuntimeClasses, they are different types:
        //   T_logical: The C++ Type for the InterfaceGroup or RuntimeClass, when
        //              used as a template parameter.  Eg "RCFoo*"
        //   T_abi:     The C++ type for the default interface used to represent the
        //              InterfaceGroup or RuntimeClass when passed as a method parameter.
        //              Eg "IFoo*"
        //   T_complex: An instantiation of the Internal "AggregateType" template that
        //              combines T_logical with T_abi. Eg "AggregateType<RCFoo*,IFoo*>"
        // See the declaration above of Windows::Foundation::Internal::AggregateType
        // for more details.
        typedef T                                                               T_complex;

        virtual HRESULT STDMETHODCALLTYPE First(_Outptr_result_maybenull_ IIterator<T_logical> **first) = 0;
    };

    namespace Detail
    {

        // known struct types get no-op cleanup
        #define XWINRT_DEF_CLEANUP(type) \
        inline void _Cleanup(type * /* values[] */, unsigned /* actual */) {}
        XWINRT_DEF_CLEANUP(GUID);
        XWINRT_DEF_CLEANUP(Windows::Foundation::DateTime);
        XWINRT_DEF_CLEANUP(Windows::Foundation::TimeSpan);
        XWINRT_DEF_CLEANUP(Windows::Foundation::Point);
        XWINRT_DEF_CLEANUP(Windows::Foundation::Size);
        XWINRT_DEF_CLEANUP(Windows::Foundation::Rect);
        #undef XWINRT_DEF_CLEANUP

        // Template magic for number and enums
        template <bool condition, class T = void*>
        struct enable_if { };
        template <class T>
        struct enable_if<true, T> { typedef T type;};

        // numbers, enums get no-op cleanup.
        template <class T>
        void _Cleanup(T* /*values*/, unsigned /*actual*/, typename enable_if<!__is_class(T) && !(is_pointer<T>::value)>::type = 0) {}



        template <class I, class Number>
        void _Cleanup(_Inout_updates_(actual) I* values[], Number actual)
        {
            for (unsigned i = 0; i < actual; ++i)
            {
                values[i]->Release();
                values[i] = nullptr;
            }
        }


        // make this a template so that we don't deptend on WindowsDeleteString in this file
        template <class Number>
        inline void _Cleanup(_Inout_updates_(actual) HSTRING* values, Number actual)
        {
            for (unsigned i = 0; i < actual; ++i)
            {
                ::WindowsDeleteString(values[i]);
                values[i] = nullptr;
            }
        }

        // Note: Because structs require custom cleanup, the default implementation will not be
        // available to custom collections of structs. They will need to provide their own
        // implementations.
        template <class U, class T>
        HRESULT _VectorGetMany(
                            _In_ U* pThis,
                            _In_ unsigned startIndex,
                            _In_ unsigned capacity,
                            _Out_writes_to_(capacity,*actual)  T *value,
                            _Out_ unsigned *actual)
        {
            unsigned index = 0;
            HRESULT hr = S_OK;
            unsigned size = 0;
            unsigned copied = 0;

            ZeroMemory(value, sizeof(*value) * capacity);
            *actual = 0;

            // Get the size of the vector so that we can do bounds checking
            hr = pThis->get_Size(&size);

            if (SUCCEEDED(hr))
            {
                if (startIndex > size)
                {
                    // If we are more than one past the end, then we return E_BOUNDS;
                    hr = E_BOUNDS;
                }
                else
                {
                    // we are guarenteed to be one past the end or less.  If we are one past the end
                    // we won't enter the for loop, and we'll get S_OK but nothing returned.
                    // If we are at the end or earlier, we'll actually get something in the output
                    for (index = 0; (index < capacity) && (index + startIndex < size) ; index++)
                    {
                        hr = pThis->GetAt(index + startIndex, &value[index] );
                        if (SUCCEEDED(hr))
                        {
                            copied += 1;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                *actual = index;
            }

            if (FAILED(hr))
            {
                Detail::_Cleanup(value, copied);
            }
            return hr;
        }

        template <class U, class T>
        HRESULT _IteratorGetMany(_In_ U* pThis, _In_ unsigned capacity, _Out_writes_to_(capacity,*actual) T *value, _Out_ unsigned *actual)
        {
            HRESULT hr = S_OK;
            ::boolean fHasCurrent = false;
            unsigned count = 0;
            ZeroMemory(value, sizeof(*value) * capacity);
            *actual = 0;

            hr = pThis->get_HasCurrent(&fHasCurrent);
            while (SUCCEEDED(hr) && (fHasCurrent) && (count < capacity))
            {
                hr = pThis->get_Current(&value[count]);
                if (SUCCEEDED(hr))
                {
                    count++;
                    hr = pThis->MoveNext(&fHasCurrent);
                }
            }

            if (SUCCEEDED(hr))
            {
                *actual = count;
            }
            else
            {
                // cleanup output paremeters on failure
                // no need to zero out *actual as it is still
                // initialized to zero.
                Detail::_Cleanup(value, *actual);
            }
            return hr;
        }

        template<class U, class T>
        HRESULT _VectorReplaceAll(_In_ U* pThis, _In_ unsigned count, _In_reads_(count) T *value)
        {
            HRESULT hr = S_OK;
            hr = pThis->Clear();
            if (SUCCEEDED(hr))
            {
                for(unsigned index = 0; index < count; index++)
                {
                    hr = pThis->Append(value[index]);
                    if (FAILED(hr))
                    {
                        // intentionally ignoring the return value here so that hr
                        // can be passed through;
                        pThis->Clear();
                        break;
                    }
                }
            }
            return hr;
        }

    }

    // Note: There are two versions of this template.  The second will compile where T is a struct and the
    // first will compile in all other cases.  This approach is used to ensure that if T is a struct that
    // GetMany will be pure virtual (and must be overloaded), but in the other cases GetMany will
    // be handed by the default implementation.
    // Important Note!:  Both of these templates must have the same vtable!!!  Change one and you
    // must change the other
    template <class T, bool isStruct>
    struct IIterator_impl : IInspectable
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<T>::type     T_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<T>::type T_logical;
    public:
        typedef T                                                               T_complex;

        virtual /* propget */ HRESULT STDMETHODCALLTYPE get_Current(_Out_ T_abi *current) = 0;
        virtual /* propget */ HRESULT STDMETHODCALLTYPE get_HasCurrent(_Out_ boolean *hasCurrent) = 0;
        virtual HRESULT STDMETHODCALLTYPE MoveNext(_Out_ boolean *hasCurrent) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetMany(_In_ unsigned capacity, _Out_writes_to_(capacity,*actual) T_abi *value, _Out_ unsigned *actual)
        {
            return Detail::_IteratorGetMany(this, capacity, value, actual);
        }
    };

    template <class T>
    struct IIterator_impl<T, false> : IInspectable
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<T>::type     T_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<T>::type T_logical;
    public:
        typedef T                                                               T_complex;

        virtual /* propget */ HRESULT STDMETHODCALLTYPE get_Current(_Out_ T_abi *current) = 0;
        virtual /* propget */ HRESULT STDMETHODCALLTYPE get_HasCurrent(_Out_ boolean *hasCurrent) = 0;
        virtual HRESULT STDMETHODCALLTYPE MoveNext(_Out_ boolean *hasCurrent) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetMany(_In_ unsigned capacity, _Out_writes_to_(capacity,*actual) T_abi *value, _Out_ unsigned *actual) = 0;
    };


    // Note: There are two versions of this template.  The second will compile where T is a struct and the
    // first will compile in all other cases.  This approach is used to ensure that if T is a struct that
    // GetMany will be pure virtual (and must be overloaded), but in the other cases GetMany will
    // be handed by the default implementation.
    // Important Note!:  Both of these templates must have the same vtable!!!  Change one and you
    // must change the other
    template <class T, bool isStruct>
    struct IVectorView_impl : IInspectable /* requires IIterable<T> */
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<T>::type     T_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<T>::type T_logical;
    public:
        typedef T                                                               T_complex;

        virtual HRESULT STDMETHODCALLTYPE GetAt(_In_ unsigned index, _Out_ T_abi *item) = 0;
        virtual /* propget */ HRESULT STDMETHODCALLTYPE get_Size(_Out_ unsigned *size) = 0;
        virtual HRESULT STDMETHODCALLTYPE IndexOf(_In_opt_ T_abi value, _Out_ unsigned *index, _Out_ boolean *found) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetMany(_In_  unsigned startIndex, _In_ unsigned capacity, _Out_writes_to_(capacity,*actual) T_abi *value, _Out_ unsigned *actual)
        {
            return Detail::_VectorGetMany(this, startIndex, capacity, value, actual);
        }
    };

    template <class T>
    struct IVectorView_impl<T, false> : IInspectable /* requires IIterable<T> */
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<T>::type     T_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<T>::type T_logical;
    public:
        typedef T                                                               T_complex;

        virtual HRESULT STDMETHODCALLTYPE GetAt(_In_ unsigned index, _Out_ T_abi *item) = 0;
        virtual /* propget */ HRESULT STDMETHODCALLTYPE get_Size(_Out_ unsigned *size) = 0;
        virtual HRESULT STDMETHODCALLTYPE IndexOf(_In_opt_ T_abi value, _Out_ unsigned *index, _Out_ boolean *found) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetMany(_In_  unsigned startIndex, _In_ unsigned capacity, _Out_writes_to_(capacity,*actual) T_abi *value, _Out_ unsigned *actual) = 0;
    };

    // Note: There are two versions of this template.  The second will compile where T is a struct and the
    // first will compile in all other cases.  This approach is used to ensure that if T is a struct that
    // GetMany and ReplaceAllwill be pure virtual (and must be overloaded), but in the other cases GetMany will
    // be handed by the default implementation.
    // Important Note!:  Both of these templates must have the same vtable!!!  Change one and you
    // must change the other
    template <class T, bool isStruct>
    struct IVector_impl : IInspectable /* requires IIterable<T> */
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<T>::type     T_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<T>::type T_logical;
    public:
        typedef T                                                               T_complex;

        // read methods
        virtual HRESULT STDMETHODCALLTYPE GetAt(_In_opt_ unsigned index, _Out_ T_abi *item) = 0;
        virtual /* propget */ HRESULT STDMETHODCALLTYPE get_Size(_Out_ unsigned *size) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetView(_Outptr_result_maybenull_ IVectorView<T_logical> **view) = 0;
        virtual HRESULT STDMETHODCALLTYPE IndexOf(_In_opt_ T_abi value, _Out_ unsigned *index, _Out_ boolean *found) = 0;

        // write methods
        virtual HRESULT STDMETHODCALLTYPE SetAt(_In_ unsigned index, _In_opt_ T_abi item) = 0;
        virtual HRESULT STDMETHODCALLTYPE InsertAt(_In_ unsigned index, _In_opt_ T_abi item) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveAt(_In_ unsigned index) = 0;
        virtual HRESULT STDMETHODCALLTYPE Append(_In_opt_ T_abi item) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveAtEnd() = 0;
        virtual HRESULT STDMETHODCALLTYPE Clear() = 0;

        // bulk transfer methods
        virtual HRESULT STDMETHODCALLTYPE GetMany(_In_  unsigned startIndex, _In_ unsigned capacity, _Out_writes_to_(capacity,*actual) T_abi *value, _Out_ unsigned *actual)
        {
            return Detail::_VectorGetMany(this, startIndex, capacity, value, actual);
        }

        virtual HRESULT STDMETHODCALLTYPE ReplaceAll(_In_ unsigned count, _In_reads_(count) T_abi *value)
        {
            return Detail::_VectorReplaceAll(this, count, value);
        }
    };

    template <class T>
    struct IVector_impl<T, false> : IInspectable /* requires IIterable<T> */
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<T>::type     T_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<T>::type T_logical;
    public:
        typedef T                                                               T_complex;

        // read methods
        virtual HRESULT STDMETHODCALLTYPE GetAt(_In_opt_ unsigned index, _Out_ T_abi *item) = 0;
        virtual /* propget */ HRESULT STDMETHODCALLTYPE get_Size(_Out_ unsigned *size) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetView(_Outptr_result_maybenull_ IVectorView<T_logical> **view) = 0;
        virtual HRESULT STDMETHODCALLTYPE IndexOf(_In_opt_ T_abi value, _Out_ unsigned *index, _Out_ boolean *found) = 0;

        // write methods
        virtual HRESULT STDMETHODCALLTYPE SetAt(_In_ unsigned index, _In_opt_ T_abi item) = 0;
        virtual HRESULT STDMETHODCALLTYPE InsertAt(_In_ unsigned index, _In_opt_ T_abi item) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveAt(_In_ unsigned index) = 0;
        virtual HRESULT STDMETHODCALLTYPE Append(_In_opt_ T_abi item) = 0;
        virtual HRESULT STDMETHODCALLTYPE RemoveAtEnd() = 0;
        virtual HRESULT STDMETHODCALLTYPE Clear() = 0;

        // bulk transfer methods
        virtual HRESULT STDMETHODCALLTYPE GetMany(_In_  unsigned startIndex, _In_ unsigned capacity, _Out_writes_to_(capacity,*actual) T_abi *value, _Out_ unsigned *actual) = 0;
        virtual HRESULT STDMETHODCALLTYPE ReplaceAll(_In_ unsigned count, _In_reads_(count) T_abi *value) = 0;
    };

    template <class K, class V>
    struct IKeyValuePair_impl : IInspectable
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<K>::type     K_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<K>::type K_logical;
        typedef typename Windows::Foundation::Internal::GetAbiType<V>::type     V_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<V>::type V_logical;
    public:
        typedef K                                                               K_complex;
        typedef V                                                               V_complex;

        virtual HRESULT STDMETHODCALLTYPE get_Key(_Out_ K_abi *key) = 0;
        virtual HRESULT STDMETHODCALLTYPE get_Value(_Out_ V_abi *value) = 0;
    };

    template <class K, class V>
    struct IMapView_impl : IInspectable /* requires IIterable< IKeyValuePair<GetLogicalType<TKey>::type,GetLogicalType<TValue>::type> *> */
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<K>::type     K_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<K>::type K_logical;
        typedef typename Windows::Foundation::Internal::GetAbiType<V>::type     V_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<V>::type V_logical;
    public:
        typedef K                                                               K_complex;
        typedef V                                                               V_complex;

        virtual HRESULT STDMETHODCALLTYPE Lookup(_In_opt_ K_abi key, _Out_ V_abi *value) = 0;
        virtual/*propget*/ HRESULT STDMETHODCALLTYPE get_Size(_Out_ unsigned int *size) = 0;
        virtual HRESULT STDMETHODCALLTYPE HasKey(_In_opt_ K_abi key, _Out_ boolean *found) = 0;
        virtual HRESULT STDMETHODCALLTYPE Split(_Outptr_result_maybenull_ IMapView<K_logical,V_logical> **firstPartition, _Outptr_result_maybenull_ IMapView<K_logical,V_logical> **secondPartition) = 0;
    };

    template <class K, class V>
    struct IMap_impl : IInspectable /* requires IIterable< IKeyValuePair<K_logical,V_logical> *> */
    {
    private:
        typedef typename Windows::Foundation::Internal::GetAbiType<K>::type     K_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<K>::type K_logical;
        typedef typename Windows::Foundation::Internal::GetAbiType<V>::type     V_abi;
        typedef typename Windows::Foundation::Internal::GetLogicalType<V>::type V_logical;
    public:
        typedef K                                                               K_complex;
        typedef V                                                               V_complex;

        // read methods
        virtual HRESULT STDMETHODCALLTYPE Lookup(_In_opt_ K_abi key, _Out_ V_abi *value) = 0;
        virtual /*propget*/ HRESULT STDMETHODCALLTYPE get_Size(_Out_ unsigned int *size) = 0;
        virtual HRESULT STDMETHODCALLTYPE HasKey(_In_opt_ K_abi key, _Out_ boolean *found) = 0;
        virtual HRESULT STDMETHODCALLTYPE GetView(_Outptr_result_maybenull_ IMapView<K_logical,V_logical> **view) = 0;

        // write methods
        virtual HRESULT STDMETHODCALLTYPE Insert(_In_opt_ K_abi key, _In_opt_ V_abi value, _Out_ boolean *replaced) = 0;
        virtual HRESULT STDMETHODCALLTYPE Remove(_In_opt_ K_abi key) = 0;
        virtual HRESULT STDMETHODCALLTYPE Clear() = 0;
    };
}}} // namespace Windows::Foundation::Collections
