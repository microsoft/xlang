
namespace xlang::impl
{
    template <typename D>
    void consume_IAsyncAction<D>::Completed(System::AsyncActionCompletedHandler const& handler) const
    {
        check_hresult(WINRT_SHIM(System::IAsyncAction)->put_Completed(get_abi(handler)));
    }

    template <typename D>
    System::AsyncActionCompletedHandler consume_IAsyncAction<D>::Completed() const
    {
        System::AsyncActionCompletedHandler handler{};
        check_hresult(WINRT_SHIM(System::IAsyncAction)->get_Completed(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TResult>
    void consume_IAsyncOperation<D, TResult>::Completed(System::AsyncOperationCompletedHandler<TResult> const& handler) const
    {
        check_hresult(WINRT_SHIM(System::IAsyncOperation<TResult>)->put_Completed(get_abi(handler)));
    }

    template <typename D, typename TResult>
    System::AsyncOperationCompletedHandler<TResult> consume_IAsyncOperation<D, TResult>::Completed() const
    {
        System::AsyncOperationCompletedHandler<TResult> temp;
        check_hresult(WINRT_SHIM(System::IAsyncOperation<TResult>)->get_Completed(put_abi(temp)));
        return temp;
    }

    template <typename D, typename TProgress>
    void consume_IAsyncActionWithProgress<D, TProgress>::Progress(System::AsyncActionProgressHandler<TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(System::IAsyncActionWithProgress<TProgress>)->put_Progress(get_abi(handler)));
    }

    template <typename D, typename TProgress>
    System::AsyncActionProgressHandler<TProgress> consume_IAsyncActionWithProgress<D, TProgress>::Progress() const
    {
        System::AsyncActionProgressHandler<TProgress> handler;
        check_hresult(WINRT_SHIM(System::IAsyncActionWithProgress<TProgress>)->get_Progress(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TProgress>
    void consume_IAsyncActionWithProgress<D, TProgress>::Completed(System::AsyncActionWithProgressCompletedHandler<TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(System::IAsyncActionWithProgress<TProgress>)->put_Completed(get_abi(handler)));
    }

    template <typename D, typename TProgress>
    System::AsyncActionWithProgressCompletedHandler<TProgress> consume_IAsyncActionWithProgress<D, TProgress>::Completed() const
    {
        System::AsyncActionWithProgressCompletedHandler<TProgress> handler;
        check_hresult(WINRT_SHIM(System::IAsyncActionWithProgress<TProgress>)->get_Completed(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TResult, typename TProgress>
    void consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Progress(System::AsyncOperationProgressHandler<TResult, TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(System::IAsyncOperationWithProgress<TResult, TProgress>)->put_Progress(get_abi(handler)));
    }

    template <typename D, typename TResult, typename TProgress>
    System::AsyncOperationProgressHandler<TResult, TProgress> consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Progress() const
    {
        System::AsyncOperationProgressHandler<TResult, TProgress> handler;
        check_hresult(WINRT_SHIM(System::IAsyncOperationWithProgress<TResult, TProgress>)->get_Progress(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TResult, typename TProgress>
    void consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Completed(System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(System::IAsyncOperationWithProgress<TResult, TProgress>)->put_Completed(get_abi(handler)));
    }

    template <typename D, typename TResult, typename TProgress>
    System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Completed() const
    {
        System::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> handler;
        check_hresult(WINRT_SHIM(System::IAsyncOperationWithProgress<TResult, TProgress>)->get_Completed(put_abi(handler)));
        return handler;
    }
}
