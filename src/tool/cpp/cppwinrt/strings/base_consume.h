
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
