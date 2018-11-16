
namespace xlang::impl
{
    template <typename D>
    void consume_IAsyncAction<D>::Completed(AsyncActionCompletedHandler const& handler) const
    {
        check_hresult(WINRT_SHIM(IAsyncAction)->put_Completed(get_abi(handler)));
    }

    template <typename D>
    AsyncActionCompletedHandler consume_IAsyncAction<D>::Completed() const
    {
        AsyncActionCompletedHandler handler{};
        check_hresult(WINRT_SHIM(IAsyncAction)->get_Completed(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TResult>
    void consume_IAsyncOperation<D, TResult>::Completed(AsyncOperationCompletedHandler<TResult> const& handler) const
    {
        check_hresult(WINRT_SHIM(IAsyncOperation<TResult>)->put_Completed(get_abi(handler)));
    }

    template <typename D, typename TResult>
    AsyncOperationCompletedHandler<TResult> consume_IAsyncOperation<D, TResult>::Completed() const
    {
        AsyncOperationCompletedHandler<TResult> temp;
        check_hresult(WINRT_SHIM(IAsyncOperation<TResult>)->get_Completed(put_abi(temp)));
        return temp;
    }

    template <typename D, typename TProgress>
    void consume_IAsyncActionWithProgress<D, TProgress>::Progress(AsyncActionProgressHandler<TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(IAsyncActionWithProgress<TProgress>)->put_Progress(get_abi(handler)));
    }

    template <typename D, typename TProgress>
    AsyncActionProgressHandler<TProgress> consume_IAsyncActionWithProgress<D, TProgress>::Progress() const
    {
        AsyncActionProgressHandler<TProgress> handler;
        check_hresult(WINRT_SHIM(IAsyncActionWithProgress<TProgress>)->get_Progress(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TProgress>
    void consume_IAsyncActionWithProgress<D, TProgress>::Completed(AsyncActionWithProgressCompletedHandler<TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(IAsyncActionWithProgress<TProgress>)->put_Completed(get_abi(handler)));
    }

    template <typename D, typename TProgress>
    AsyncActionWithProgressCompletedHandler<TProgress> consume_IAsyncActionWithProgress<D, TProgress>::Completed() const
    {
        AsyncActionWithProgressCompletedHandler<TProgress> handler;
        check_hresult(WINRT_SHIM(IAsyncActionWithProgress<TProgress>)->get_Completed(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TResult, typename TProgress>
    void consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Progress(AsyncOperationProgressHandler<TResult, TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(IAsyncOperationWithProgress<TResult, TProgress>)->put_Progress(get_abi(handler)));
    }

    template <typename D, typename TResult, typename TProgress>
    AsyncOperationProgressHandler<TResult, TProgress> consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Progress() const
    {
        AsyncOperationProgressHandler<TResult, TProgress> handler;
        check_hresult(WINRT_SHIM(IAsyncOperationWithProgress<TResult, TProgress>)->get_Progress(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TResult, typename TProgress>
    void consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Completed(AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(IAsyncOperationWithProgress<TResult, TProgress>)->put_Completed(get_abi(handler)));
    }

    template <typename D, typename TResult, typename TProgress>
    AsyncOperationWithProgressCompletedHandler<TResult, TProgress> consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Completed() const
    {
        AsyncOperationWithProgressCompletedHandler<TResult, TProgress> handler;
        check_hresult(WINRT_SHIM(IAsyncOperationWithProgress<TResult, TProgress>)->get_Completed(put_abi(handler)));
        return handler;
    }
}
