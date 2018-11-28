
namespace winrt::impl
{
    template <typename T>
    auto detach_from(T&& object) noexcept
    {
        return detach_abi(std::forward<T>(object));
    }

    template <typename D> struct produce<D, Windows::Foundation::IActivationFactory> : produce_base<D, Windows::Foundation::IActivationFactory>
    {
        int32_t WINRT_CALL ActivateInstance(void** instance) noexcept final
        {
            try
            {
                *instance = nullptr;
                typename D::abi_guard guard(this->shim());
                *instance = detach_abi(this->shim().ActivateInstance());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D> struct produce<D, Windows::Foundation::IAsyncAction> : produce_base<D, Windows::Foundation::IAsyncAction>
    {
        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Windows::Foundation::AsyncActionCompletedHandler const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Completed(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncActionCompletedHandler>(this->shim().Completed());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetResults() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().GetResults();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D> struct produce<D, Windows::Foundation::IAsyncInfo> : produce_base<D, Windows::Foundation::IAsyncInfo>
    {
        int32_t WINRT_CALL get_Id(uint32_t* id) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *id = this->shim().Id();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Status(winrt::Windows::Foundation::AsyncStatus* status) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *status = this->shim().Status();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_ErrorCode(int32_t* errorCode) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                *errorCode = this->shim().ErrorCode();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL Cancel() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Cancel();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL Close() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Close();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename TProgress> struct produce<D, Windows::Foundation::IAsyncActionWithProgress<TProgress>> : produce_base<D, Windows::Foundation::IAsyncActionWithProgress<TProgress>>
    {
        int32_t WINRT_CALL put_Progress(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Progress(*reinterpret_cast<Windows::Foundation::AsyncActionProgressHandler<TProgress> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Progress(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncActionProgressHandler<TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Completed(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>>(this->shim().Completed());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetResults() noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().GetResults();
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename TResult> struct produce<D, Windows::Foundation::IAsyncOperation<TResult>> : produce_base<D, Windows::Foundation::IAsyncOperation<TResult>>
    {
        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Windows::Foundation::AsyncOperationCompletedHandler<TResult> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Completed(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncOperationCompletedHandler<TResult>>(this->shim().Completed());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetResults(arg_out<TResult> results) noexcept final
        {
            try
            {
                clear_abi(results);
                typename D::abi_guard guard(this->shim());
                *results = detach_from<TResult>(this->shim().GetResults());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <typename D, typename TResult, typename TProgress> struct produce<D, Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>> : produce_base<D, Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress>>
    {
        int32_t WINRT_CALL put_Progress(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Progress(*reinterpret_cast<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Progress(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>>(this->shim().Progress());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL put_Completed(void* handler) noexcept final
        {
            try
            {
                typename D::abi_guard guard(this->shim());
                this->shim().Completed(*reinterpret_cast<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress> const*>(&handler));
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL get_Completed(void** handler) noexcept final
        {
            try
            {
                *handler = nullptr;
                typename D::abi_guard guard(this->shim());
                *handler = detach_from<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>(this->shim().Completed());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }

        int32_t WINRT_CALL GetResults(arg_out<TResult> results) noexcept final
        {
            try
            {
                clear_abi(results);
                typename D::abi_guard guard(this->shim());
                *results = detach_from<TResult>(this->shim().GetResults());
                return error_ok;
            }
            catch (...) { return to_hresult(); }
        }
    };

    template <> struct delegate<Windows::Foundation::AsyncActionCompletedHandler>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncActionCompletedHandler, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncActionCompletedHandler, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* asyncInfo, Windows::Foundation::AsyncStatus asyncStatus) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncAction const*>(&asyncInfo), asyncStatus);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult> struct delegate<Windows::Foundation::AsyncOperationCompletedHandler<TResult>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncOperationCompletedHandler<TResult>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncOperationCompletedHandler<TResult>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, Windows::Foundation::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncOperation<TResult> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<Windows::Foundation::AsyncActionProgressHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncActionProgressHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncActionProgressHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncActionWithProgress<TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TProgress> struct delegate<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncActionWithProgressCompletedHandler<TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, Windows::Foundation::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncActionWithProgress<TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncOperationProgressHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, arg_in<TProgress> args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), *reinterpret_cast<TProgress const*>(&args));
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };

    template <typename TResult, typename TProgress> struct delegate<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>>
    {
        template <typename H>
        struct type final : implements_delegate<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>
        {
            type(H&& handler) : implements_delegate<Windows::Foundation::AsyncOperationWithProgressCompletedHandler<TResult, TProgress>, H>(std::forward<H>(handler)) {}

            int32_t WINRT_CALL Invoke(void* sender, Windows::Foundation::AsyncStatus args) noexcept final
            {
                try
                {
                    (*this)(*reinterpret_cast<Windows::Foundation::IAsyncOperationWithProgress<TResult, TProgress> const*>(&sender), args);
                    return error_ok;
                }
                catch (...) { return to_hresult(); }
            }
        };
    };
}
