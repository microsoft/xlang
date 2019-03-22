
namespace Windows { namespace Foundation
{
    template <class TResult, class TProgress>
    struct IAsyncOperationProgressHandler_impl;

    template <class TResult, class TProgress>
    struct IAsyncOperationWithProgressCompletedHandler_impl;

    template <class TResult>
    struct IAsyncOperationCompletedHandler_impl;

    template <class TResult, class TProgress>
    struct IAsyncOperationWithProgress_impl;

    template <class TProgress>
    struct IAsyncActionWithProgress_impl;

    template <class TProgress>
    struct IAsyncOperation_impl;

    template <class TProgress>
    struct IAsyncActionProgressHandler_impl;

    template <class TProgress>
    struct IAsyncActionWithProgressCompletedHandler_impl;

    template <class T>
    struct IReference_impl;

    template <class T>
    struct IReferenceArray_impl;

    template <class T>
    struct IEventHandler_impl;

    template <class TSender, class TArgs>
    struct ITypedEventHandler_impl;

    template <class TResult, class TProgress>
    struct IAsyncOperationProgressHandler
            : IAsyncOperationProgressHandler_impl<TResult, TProgress>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IAsyncOperationProgressHandler<TResult, TProgress>>
    {
    };

    template <class TResult>
            struct IAsyncOperationCompletedHandler
            : IAsyncOperationCompletedHandler_impl<TResult>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IAsyncOperationCompletedHandler<TResult>>
    {
    };

    template <class TResult, class TProgress>
    struct IAsyncOperationWithProgressCompletedHandler
            : IAsyncOperationWithProgressCompletedHandler_impl<TResult, TProgress>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IAsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
    };

    template <class TProgress>
    struct IAsyncActionProgressHandler
            : IAsyncActionProgressHandler_impl<TProgress>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IAsyncActionProgressHandler<TProgress>>
    {
    };

    template <class TProgress>
    struct IAsyncActionWithProgressCompletedHandler
            : IAsyncActionWithProgressCompletedHandler_impl<TProgress>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IAsyncActionWithProgressCompletedHandler<TProgress>>
    {
    };

    template <class TResult>
            struct IAsyncOperation
            : IAsyncOperation_impl<TResult>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IAsyncOperation<TResult>>
    {
    };

    template <class TResult, class TProgress>
    struct IAsyncOperationWithProgress
            : IAsyncOperationWithProgress_impl<TResult, TProgress>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IAsyncOperationWithProgress<TResult, TProgress>>
    {
    };

    template <class TProgress>
    struct IAsyncActionWithProgress
            : IAsyncActionWithProgress_impl<TProgress>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IAsyncActionWithProgress<TProgress>>
    {
    };

    template <class T>
    struct IReference
            : IReference_impl<T>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IReference<T>>
    {
    };

    template <class T>
    struct IReferenceArray
            : IReferenceArray_impl<T>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IReferenceArray<T>>
    {
    };

    template <class T>
    struct IEventHandler
            : IEventHandler_impl<T>
            , Windows::Foundation::Collections::detail::not_yet_specialized<IEventHandler<T>>
    {
    };

    template <class TSender, class TArgs>
    struct ITypedEventHandler
            : ITypedEventHandler_impl<TSender, TArgs>
            , Windows::Foundation::Collections::detail::not_yet_specialized<ITypedEventHandler<TSender, TArgs>>
    {
    };

    template <class TResult, class TProgress>
    struct IAsyncOperationProgressHandler_impl : IUnknown
    {
        private:
            typedef typename Windows::Foundation::Internal::GetAbiType<TProgress>::type     TProgress_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TProgress>::type TProgress_logical;
            typedef typename Windows::Foundation::Internal::GetAbiType<TResult>::type     TResult_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TResult>::type TResult_logical;
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
        typedef TProgress                                                               TProgress_complex;
        typedef TResult                                                                 TResult_complex;

        virtual HRESULT STDMETHODCALLTYPE Invoke(IAsyncOperationWithProgress<TResult_logical, TProgress_logical> *asyncInfo, TProgress_abi progressInfo) = 0;
    };

    template <class TResult, class TProgress>
    struct IAsyncOperationWithProgressCompletedHandler_impl : IUnknown
    {
        private:
            typedef typename Windows::Foundation::Internal::GetAbiType<TProgress>::type     TProgress_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TProgress>::type TProgress_logical;
            typedef typename Windows::Foundation::Internal::GetAbiType<TResult>::type     TResult_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TResult>::type TResult_logical;
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
        typedef TProgress                                                               TProgress_complex;
        typedef TResult                                                                 TResult_complex;

        virtual HRESULT STDMETHODCALLTYPE Invoke(IAsyncOperationWithProgress<TResult_logical, TProgress_logical> *asyncInfo, Windows::Foundation::AsyncStatus status) = 0;
    };

    template <class TResult>
    struct IAsyncOperationCompletedHandler_impl : IUnknown
    {
        private:
            typedef typename Windows::Foundation::Internal::GetAbiType<TResult>::type     TResult_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TResult>::type TResult_logical;
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
        typedef TResult                                                                 TResult_complex;

        virtual HRESULT STDMETHODCALLTYPE Invoke(IAsyncOperation<TResult_logical> *asyncInfo, Windows::Foundation::AsyncStatus status) = 0;
    };

    template <class TProgress>
    struct IAsyncActionProgressHandler_impl : IUnknown
    {
        private:
            typedef typename Windows::Foundation::Internal::GetAbiType<TProgress>::type     TProgress_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TProgress>::type TProgress_logical;
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
            typedef TProgress                                                               TProgress_complex;

            virtual HRESULT STDMETHODCALLTYPE Invoke(IAsyncActionWithProgress<TProgress_logical> *asyncInfo, TProgress_abi progressInfo) = 0;
    };

    template <class TProgress>
    struct IAsyncActionWithProgressCompletedHandler_impl : IUnknown
    {
        private:
            typedef typename Windows::Foundation::Internal::GetAbiType<TProgress>::type     TProgress_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TProgress>::type TProgress_logical;
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
            typedef TProgress                                                               TProgress_complex;

            virtual HRESULT STDMETHODCALLTYPE Invoke(IAsyncActionWithProgress<TProgress_logical> *asyncInfo, Windows::Foundation::AsyncStatus status) = 0;
    };

    template <class TResult, class TProgress>
    struct IAsyncOperationWithProgress_impl : IInspectable
    {
        private:
            typedef typename Windows::Foundation::Internal::GetAbiType<TProgress>::type     TProgress_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TProgress>::type TProgress_logical;
            typedef typename Windows::Foundation::Internal::GetAbiType<TResult>::type     TResult_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TResult>::type TResult_logical;
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
            typedef TProgress                                                               TProgress_complex;
            typedef TResult                                                                 TResult_complex;

            virtual HRESULT STDMETHODCALLTYPE put_Progress( IAsyncOperationProgressHandler<TResult_logical, TProgress_logical> *handler) = 0;

            virtual HRESULT STDMETHODCALLTYPE get_Progress( IAsyncOperationProgressHandler<TResult_logical, TProgress_logical> **handler) = 0;

            virtual HRESULT STDMETHODCALLTYPE put_Completed( IAsyncOperationWithProgressCompletedHandler<TResult_logical, TProgress_logical> *handler) = 0;

            virtual HRESULT STDMETHODCALLTYPE get_Completed( IAsyncOperationWithProgressCompletedHandler<TResult_logical, TProgress_logical> **handler) = 0;

            virtual HRESULT STDMETHODCALLTYPE GetResults(  TResult_abi *results) = 0;

    };

    template <class TResult>
    struct IAsyncOperation_impl : IInspectable
    {
        private:
            typedef typename Windows::Foundation::Internal::GetAbiType<TResult>::type     TResult_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TResult>::type TResult_logical;
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
            typedef TResult                                                                 TResult_complex;

            virtual HRESULT STDMETHODCALLTYPE put_Completed( IAsyncOperationCompletedHandler<TResult_logical> *handler) = 0;
            virtual HRESULT STDMETHODCALLTYPE get_Completed( IAsyncOperationCompletedHandler<TResult_logical> **handler) = 0;
            virtual HRESULT STDMETHODCALLTYPE GetResults(  TResult_abi *results) = 0;

    };

    template <class TProgress>
    struct IAsyncActionWithProgress_impl : IInspectable
    {
        private:
            typedef typename Windows::Foundation::Internal::GetAbiType<TProgress>::type     TProgress_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TProgress>::type TProgress_logical;
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
            typedef TProgress                                                               TProgress_complex;

            virtual HRESULT STDMETHODCALLTYPE put_Progress( IAsyncActionProgressHandler<TProgress_logical> *handler) = 0;

            virtual HRESULT STDMETHODCALLTYPE get_Progress( IAsyncActionProgressHandler<TProgress_logical> **handler) = 0;

            virtual HRESULT STDMETHODCALLTYPE put_Completed( IAsyncActionWithProgressCompletedHandler<TProgress_logical> *handler) = 0;

            virtual HRESULT STDMETHODCALLTYPE get_Completed( IAsyncActionWithProgressCompletedHandler<TProgress_logical> **handler) = 0;

            virtual HRESULT STDMETHODCALLTYPE GetResults(  ) = 0;

    };

    template <class T>
    struct IReference_impl : IInspectable
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

        virtual HRESULT STDMETHODCALLTYPE get_Value( _Out_ T_abi *value) = 0;
    };

    template <class T>
    struct IReferenceArray_impl : IInspectable
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

        virtual HRESULT STDMETHODCALLTYPE get_Value(_Out_ UINT32 *length, _Outptr_result_buffer_(*length) T_abi **value) = 0;
    };

    template <class T>
    struct IEventHandler_impl : IUnknown
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

            virtual HRESULT STDMETHODCALLTYPE Invoke( _In_ IInspectable *sender, _In_ T_abi args) = 0;
    };

    template <class TSender, class TArgs>
    struct ITypedEventHandler_impl : IUnknown
    {
        private:
            typedef typename Windows::Foundation::Internal::GetAbiType<TSender>::type     TSender_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TSender>::type TSender_logical;
            typedef typename Windows::Foundation::Internal::GetAbiType<TArgs>::type     TArgs_abi;
            typedef typename Windows::Foundation::Internal::GetLogicalType<TArgs>::type TArgs_logical;
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
            typedef TSender                                                               TSender_complex;
            typedef TArgs                                                                 TArgs_complex;

            virtual HRESULT STDMETHODCALLTYPE Invoke( _In_ TSender_abi sender, _In_ TArgs_abi args) = 0;
    };
}} // namespace Windows::Foundation
