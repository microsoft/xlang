
namespace xlang::impl
{
    template <typename D>
    void consume_IAsyncAction<D>::Completed(Runtime::AsyncActionCompletedHandler const& handler) const
    {
        check_hresult(WINRT_SHIM(Runtime::IAsyncAction)->put_Completed(get_abi(handler)));
    }

    template <typename D>
    Runtime::AsyncActionCompletedHandler consume_IAsyncAction<D>::Completed() const
    {
        Runtime::AsyncActionCompletedHandler handler{};
        check_hresult(WINRT_SHIM(Runtime::IAsyncAction)->get_Completed(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TResult>
    void consume_IAsyncOperation<D, TResult>::Completed(Runtime::AsyncOperationCompletedHandler<TResult> const& handler) const
    {
        check_hresult(WINRT_SHIM(Runtime::IAsyncOperation<TResult>)->put_Completed(get_abi(handler)));
    }

    template <typename D, typename TResult>
    Runtime::AsyncOperationCompletedHandler<TResult> consume_IAsyncOperation<D, TResult>::Completed() const
    {
        Runtime::AsyncOperationCompletedHandler<TResult> temp;
        check_hresult(WINRT_SHIM(Runtime::IAsyncOperation<TResult>)->get_Completed(put_abi(temp)));
        return temp;
    }

    template <typename D, typename TProgress>
    void consume_IAsyncActionWithProgress<D, TProgress>::Progress(Runtime::AsyncActionProgressHandler<TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(Runtime::IAsyncActionWithProgress<TProgress>)->put_Progress(get_abi(handler)));
    }

    template <typename D, typename TProgress>
    Runtime::AsyncActionProgressHandler<TProgress> consume_IAsyncActionWithProgress<D, TProgress>::Progress() const
    {
        Runtime::AsyncActionProgressHandler<TProgress> handler;
        check_hresult(WINRT_SHIM(Runtime::IAsyncActionWithProgress<TProgress>)->get_Progress(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TProgress>
    void consume_IAsyncActionWithProgress<D, TProgress>::Completed(Runtime::AsyncActionWithProgressCompletedHandler<TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(Runtime::IAsyncActionWithProgress<TProgress>)->put_Completed(get_abi(handler)));
    }

    template <typename D, typename TProgress>
    Runtime::AsyncActionWithProgressCompletedHandler<TProgress> consume_IAsyncActionWithProgress<D, TProgress>::Completed() const
    {
        Runtime::AsyncActionWithProgressCompletedHandler<TProgress> handler;
        check_hresult(WINRT_SHIM(Runtime::IAsyncActionWithProgress<TProgress>)->get_Completed(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TResult, typename TProgress>
    void consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Progress(Runtime::AsyncOperationProgressHandler<TResult, TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(Runtime::IAsyncOperationWithProgress<TResult, TProgress>)->put_Progress(get_abi(handler)));
    }

    template <typename D, typename TResult, typename TProgress>
    Runtime::AsyncOperationProgressHandler<TResult, TProgress> consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Progress() const
    {
        Runtime::AsyncOperationProgressHandler<TResult, TProgress> handler;
        check_hresult(WINRT_SHIM(Runtime::IAsyncOperationWithProgress<TResult, TProgress>)->get_Progress(put_abi(handler)));
        return handler;
    }

    template <typename D, typename TResult, typename TProgress>
    void consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Completed(Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const& handler) const
    {
        check_hresult(WINRT_SHIM(Runtime::IAsyncOperationWithProgress<TResult, TProgress>)->put_Completed(get_abi(handler)));
    }

    template <typename D, typename TResult, typename TProgress>
    Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> consume_IAsyncOperationWithProgress<D, TResult, TProgress>::Completed() const
    {
        Runtime::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> handler;
        check_hresult(WINRT_SHIM(Runtime::IAsyncOperationWithProgress<TResult, TProgress>)->get_Completed(put_abi(handler)));
        return handler;
    }
}
