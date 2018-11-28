
WINRT_EXPORT namespace winrt::Windows::Foundation
{
    struct WINRT_EBO IAsyncInfo :
        IInspectable,
        impl::consume_t<IAsyncInfo>
    {
        IAsyncInfo(std::nullptr_t = nullptr) noexcept {}
        IAsyncInfo(void* ptr, take_ownership_from_abi_t) noexcept : IInspectable(ptr, take_ownership_from_abi) {}
    };

    struct WINRT_EBO IAsyncAction :
        IInspectable,
        impl::consume_t<IAsyncAction>,
        impl::require<IAsyncAction, IAsyncInfo>
    {
        IAsyncAction(std::nullptr_t = nullptr) noexcept {}
        IAsyncAction(void* ptr, take_ownership_from_abi_t) noexcept : IInspectable(ptr, take_ownership_from_abi) {}
    };

    template <typename TProgress>
    struct WINRT_EBO IAsyncActionWithProgress :
        IInspectable,
        impl::consume_t<IAsyncActionWithProgress<TProgress>>,
        impl::require<IAsyncActionWithProgress<TProgress>, IAsyncInfo>
    {
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        IAsyncActionWithProgress(std::nullptr_t = nullptr) noexcept {}
        IAsyncActionWithProgress(void* ptr, take_ownership_from_abi_t) noexcept : IInspectable(ptr, take_ownership_from_abi) {}
    };

    template <typename TResult>
    struct WINRT_EBO IAsyncOperation :
        IInspectable,
        impl::consume_t<IAsyncOperation<TResult>>,
        impl::require<IAsyncOperation<TResult>, IAsyncInfo>
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        IAsyncOperation(std::nullptr_t = nullptr) noexcept {}
        IAsyncOperation(void* ptr, take_ownership_from_abi_t) noexcept : IInspectable(ptr, take_ownership_from_abi) {}
    };

    template <typename TResult, typename TProgress>
    struct WINRT_EBO IAsyncOperationWithProgress :
        IInspectable,
        impl::consume_t<IAsyncOperationWithProgress<TResult, TProgress>>,
        impl::require<IAsyncOperationWithProgress<TResult, TProgress>, IAsyncInfo>
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        IAsyncOperationWithProgress(std::nullptr_t = nullptr) noexcept {}
        IAsyncOperationWithProgress(void* ptr, take_ownership_from_abi_t) noexcept : IInspectable(ptr, take_ownership_from_abi) {}
    };

    struct AsyncActionCompletedHandler : IUnknown
    {
        AsyncActionCompletedHandler(std::nullptr_t = nullptr) noexcept {}
        AsyncActionCompletedHandler(void* ptr, take_ownership_from_abi_t) noexcept : IUnknown(ptr, take_ownership_from_abi) {}

        template <typename L>
        AsyncActionCompletedHandler(L handler) :
            AsyncActionCompletedHandler(impl::make_delegate<AsyncActionCompletedHandler>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncActionCompletedHandler(F* handler) :
            AsyncActionCompletedHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncActionCompletedHandler(O* object, M method) :
            AsyncActionCompletedHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionCompletedHandler(com_ptr<O>&& object, M method) :
            AsyncActionCompletedHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionCompletedHandler(weak_ref<O>&& object, M method) :
            AsyncActionCompletedHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncAction const& sender, AsyncStatus args) const
        {
            check_hresult((*(impl::abi_t<AsyncActionCompletedHandler>**)this)->Invoke(get_abi(sender), args));
        }
    };

    template <typename TProgress>
    struct WINRT_EBO AsyncActionProgressHandler : IUnknown
    {
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        AsyncActionProgressHandler(std::nullptr_t = nullptr) noexcept {}
        AsyncActionProgressHandler(void* ptr, take_ownership_from_abi_t) noexcept : IUnknown(ptr, take_ownership_from_abi) {}

        template <typename L>
        AsyncActionProgressHandler(L handler) :
            AsyncActionProgressHandler(impl::make_delegate<AsyncActionProgressHandler<TProgress>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncActionProgressHandler(F* handler) :
            AsyncActionProgressHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncActionProgressHandler(O* object, M method) :
            AsyncActionProgressHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionProgressHandler(com_ptr<O>&& object, M method) :
            AsyncActionProgressHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionProgressHandler(weak_ref<O>&& object, M method) :
            AsyncActionProgressHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncActionWithProgress<TProgress> const& sender, TProgress const& args) const
        {
            check_hresult((*(impl::abi_t<AsyncActionProgressHandler<TProgress>>**)this)->Invoke(get_abi(sender), get_abi(args)));
        }
    };

    template <typename TProgress>
    struct WINRT_EBO AsyncActionWithProgressCompletedHandler : IUnknown
    {
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        AsyncActionWithProgressCompletedHandler(std::nullptr_t = nullptr) noexcept {}
        AsyncActionWithProgressCompletedHandler(void* ptr, take_ownership_from_abi_t) noexcept : IUnknown(ptr, take_ownership_from_abi) {}

        template <typename L>
        AsyncActionWithProgressCompletedHandler(L handler) :
            AsyncActionWithProgressCompletedHandler(impl::make_delegate<AsyncActionWithProgressCompletedHandler<TProgress>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncActionWithProgressCompletedHandler(F* handler) :
            AsyncActionWithProgressCompletedHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncActionWithProgressCompletedHandler(O* object, M method) :
            AsyncActionWithProgressCompletedHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionWithProgressCompletedHandler(com_ptr<O>&& object, M method) :
            AsyncActionWithProgressCompletedHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncActionWithProgressCompletedHandler(weak_ref<O>&& object, M method) :
            AsyncActionWithProgressCompletedHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncActionWithProgress<TProgress> const& sender, AsyncStatus const args) const
        {
            check_hresult((*(impl::abi_t<AsyncActionWithProgressCompletedHandler<TProgress>>**)this)->Invoke(get_abi(sender), args));
        }
    };

    template <typename TResult, typename TProgress>
    struct WINRT_EBO AsyncOperationProgressHandler : IUnknown
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        AsyncOperationProgressHandler(std::nullptr_t = nullptr) noexcept {}
        AsyncOperationProgressHandler(void* ptr, take_ownership_from_abi_t) noexcept : IUnknown(ptr, take_ownership_from_abi) {}

        template <typename L>
        AsyncOperationProgressHandler(L handler) :
            AsyncOperationProgressHandler(impl::make_delegate<AsyncOperationProgressHandler<TResult, TProgress>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncOperationProgressHandler(F* handler) :
            AsyncOperationProgressHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncOperationProgressHandler(O* object, M method) :
            AsyncOperationProgressHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationProgressHandler(com_ptr<O>&& object, M method) :
            AsyncOperationProgressHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationProgressHandler(weak_ref<O>&& object, M method) :
            AsyncOperationProgressHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncOperationWithProgress<TResult, TProgress> const& sender, TProgress const& args) const
        {
            check_hresult((*(impl::abi_t<AsyncOperationProgressHandler<TResult, TProgress>>**)this)->Invoke(get_abi(sender), get_abi(args)));
        }
    };

    template <typename TResult, typename TProgress>
    struct WINRT_EBO AsyncOperationWithProgressCompletedHandler : IUnknown
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        static_assert(impl::has_category_v<TProgress>, "TProgress must be WinRT type.");
        AsyncOperationWithProgressCompletedHandler(std::nullptr_t = nullptr) noexcept {}
        AsyncOperationWithProgressCompletedHandler(void* ptr, take_ownership_from_abi_t) noexcept : IUnknown(ptr, take_ownership_from_abi) {}

        template <typename L>
        AsyncOperationWithProgressCompletedHandler(L handler) :
            AsyncOperationWithProgressCompletedHandler(impl::make_delegate<AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncOperationWithProgressCompletedHandler(F* handler) :
            AsyncOperationWithProgressCompletedHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncOperationWithProgressCompletedHandler(O* object, M method) :
            AsyncOperationWithProgressCompletedHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationWithProgressCompletedHandler(com_ptr<O>&& object, M method) :
            AsyncOperationWithProgressCompletedHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationWithProgressCompletedHandler(weak_ref<O>&& object, M method) :
            AsyncOperationWithProgressCompletedHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncOperationWithProgress<TResult, TProgress> const& sender, AsyncStatus const args) const
        {
            check_hresult((*(impl::abi_t<AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>**)this)->Invoke(get_abi(sender), args));
        }
    };

    template <typename TResult>
    struct WINRT_EBO AsyncOperationCompletedHandler : IUnknown
    {
        static_assert(impl::has_category_v<TResult>, "TResult must be WinRT type.");
        AsyncOperationCompletedHandler(std::nullptr_t = nullptr) noexcept {}
        AsyncOperationCompletedHandler(void* ptr, take_ownership_from_abi_t) noexcept : IUnknown(ptr, take_ownership_from_abi) {}

        template <typename L>
        AsyncOperationCompletedHandler(L handler) :
            AsyncOperationCompletedHandler(impl::make_delegate<AsyncOperationCompletedHandler<TResult>>(std::forward<L>(handler)))
        {}

        template <typename F> AsyncOperationCompletedHandler(F* handler) :
            AsyncOperationCompletedHandler([=](auto&&... args) { handler(args...); })
        {}

        template <typename O, typename M> AsyncOperationCompletedHandler(O* object, M method) :
            AsyncOperationCompletedHandler([=](auto&&... args) { ((*object).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationCompletedHandler(com_ptr<O>&& object, M method) :
            AsyncOperationCompletedHandler([o = std::move(object), method](auto&&... args) { ((*o).*(method))(args...); })
        {}

        template <typename O, typename M> AsyncOperationCompletedHandler(weak_ref<O>&& object, M method) :
            AsyncOperationCompletedHandler([o = std::move(object), method](auto&&... args) { if (auto s = o.get()) { ((*s).*(method))(args...); } })
        {}

        void operator()(IAsyncOperation<TResult> const& sender, AsyncStatus args) const
        {
            check_hresult((*(impl::abi_t<AsyncOperationCompletedHandler<TResult>>**)this)->Invoke(get_abi(sender), args));
        }
    };
}
