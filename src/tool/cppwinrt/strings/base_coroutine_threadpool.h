
namespace winrt
{
    [[nodiscard]] inline auto resume_background() noexcept
    {
        struct awaitable
        {
            bool await_ready() const noexcept
            {
                return false;
            }

            void await_resume() const noexcept
            {
            }

            void await_suspend(std::experimental::coroutine_handle<> handle) const
            {
                if (!WINRT_TrySubmitThreadpoolCallback(callback, handle.address(), nullptr))
                {
                    throw_last_error();
                }
            }

        private:

            static void WINRT_CALL callback(void*, void* context) noexcept
            {
                std::experimental::coroutine_handle<>::from_address(context)();
            }
        };

        return awaitable{};
    }

    template <typename T>
    [[nodiscard]] auto resume_background(T const& context) noexcept
    {
        struct awaitable
        {
            awaitable(T const& context) : m_context(context)
            {
            }

            bool await_ready() const noexcept
            {
                return false;
            }

            void await_resume() const noexcept
            {
            }

            void await_suspend(std::experimental::coroutine_handle<> resume)
            {
                m_resume = resume;

                if (!WINRT_TrySubmitThreadpoolCallback(callback, this, nullptr))
                {
                    throw_last_error();
                }
            }

        private:

            static void WINRT_CALL callback(void*, void* context) noexcept
            {
                auto that = static_cast<awaitable*>(context);
                auto guard = that->m_context();
                that->m_resume();
            }

            T const& m_context;
            std::experimental::coroutine_handle<> m_resume{ nullptr };
        };

        return awaitable{ context };
    }

    struct apartment_context
    {
        apartment_context()
        {
            m_context.capture(WINRT_CoGetObjectContext);
        }

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_resume() const noexcept
        {
        }

        void await_suspend(std::experimental::coroutine_handle<> handle) const
        {
            impl::com_callback_args args{};
            args.data = handle.address();
            check_hresult(m_context->ContextCallback(callback, &args, guid_of<impl::ICallbackWithNoReentrancyToApplicationSTA>(), 5, nullptr));
        }

    private:

        static int32_t WINRT_CALL callback(impl::com_callback_args* args) noexcept
        {
            std::experimental::coroutine_handle<>::from_address(args->data)();
            return impl::error_ok;
        }

        com_ptr<impl::IContextCallback> m_context;
    };

    [[nodiscard]] inline auto resume_after(Windows::Foundation::TimeSpan duration) noexcept
    {
        struct awaitable
        {
            explicit awaitable(Windows::Foundation::TimeSpan duration) noexcept :
                m_duration(duration)
            {
            }

            bool await_ready() const noexcept
            {
                return m_duration.count() <= 0;
            }

            void await_suspend(std::experimental::coroutine_handle<> handle)
            {
                m_timer.attach(check_pointer(WINRT_CreateThreadpoolTimer(callback, handle.address(), nullptr)));
                int64_t relative_count = -m_duration.count();
                WINRT_SetThreadpoolTimer(m_timer.get(), &relative_count, 0, 0);
            }

            void await_resume() const noexcept
            {
            }

        private:

            static void WINRT_CALL callback(void*, void* context, void*) noexcept
            {
                std::experimental::coroutine_handle<>::from_address(context)();
            }

            struct timer_traits
            {
                using type = impl::ptp_timer;

                static void close(type value) noexcept
                {
                    WINRT_CloseThreadpoolTimer(value);
                }

                static constexpr type invalid() noexcept
                {
                    return nullptr;
                }
            };

            handle_type<timer_traits> m_timer;
            Windows::Foundation::TimeSpan m_duration;
        };

        return awaitable{ duration };
    }

#ifdef _RESUMABLE_FUNCTIONS_SUPPORTED
    inline auto operator co_await(Windows::Foundation::TimeSpan duration)
    {
        return resume_after(duration);
    }
#endif

    [[nodiscard]] inline auto resume_on_signal(void* handle, Windows::Foundation::TimeSpan timeout = {}) noexcept
    {
        struct awaitable
        {
            awaitable(void* handle, Windows::Foundation::TimeSpan timeout) noexcept :
                m_timeout(timeout),
                m_handle(handle)
            {}

            bool await_ready() const noexcept
            {
                return WINRT_WaitForSingleObject(m_handle, 0) == 0;
            }

            void await_suspend(std::experimental::coroutine_handle<> resume)
            {
                m_resume = resume;
                m_wait.attach(check_pointer(WINRT_CreateThreadpoolWait(callback, this, nullptr)));
                int64_t relative_count = -m_timeout.count();
                int64_t* file_time = relative_count != 0 ? &relative_count : nullptr;
                WINRT_SetThreadpoolWait(m_wait.get(), m_handle, file_time);
            }

            bool await_resume() const noexcept
            {
                return m_result == 0;
            }

        private:

            static void WINRT_CALL callback(void*, void* context, void*, uint32_t result) noexcept
            {
                auto that = static_cast<awaitable*>(context);
                that->m_result = result;
                that->m_resume();
            }

            struct wait_traits
            {
                using type = impl::ptp_wait;

                static void close(type value) noexcept
                {
                    WINRT_CloseThreadpoolWait(value);
                }

                static constexpr type invalid() noexcept
                {
                    return nullptr;
                }
            };

            handle_type<wait_traits> m_wait;
            Windows::Foundation::TimeSpan m_timeout;
            void* m_handle;
            uint32_t m_result{};
            std::experimental::coroutine_handle<> m_resume{ nullptr };
        };

        return awaitable{ handle, timeout };
    }

    struct thread_pool
    {
        thread_pool() :
            m_pool(check_pointer(WINRT_CreateThreadpool(nullptr)))
        {
            m_environment.Pool = m_pool.get();
        }

        void thread_limits(uint32_t const high, uint32_t const low)
        {
            WINRT_SetThreadpoolThreadMaximum(m_pool.get(), high);
            check_bool(WINRT_SetThreadpoolThreadMinimum(m_pool.get(), low));
        }

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_resume() const noexcept
        {
        }

        void await_suspend(std::experimental::coroutine_handle<> handle)
        {
            if (!WINRT_TrySubmitThreadpoolCallback(callback, handle.address(), &m_environment))
            {
                throw_last_error();
            }
        }

    private:

        static void WINRT_CALL callback(void*, void* context) noexcept
        {
            std::experimental::coroutine_handle<>::from_address(context)();
        }

        struct pool_traits
        {
            using type = impl::ptp_pool;

            static void close(type value) noexcept
            {
                WINRT_CloseThreadpool(value);
            }

            static constexpr type invalid() noexcept
            {
                return nullptr;
            }
        };

        struct environment // TP_CALLBACK_ENVIRON
        {
            uint32_t Version{ 3 };
            void* Pool{};
            void* CleanupGroup{};
            void* CleanupGroupCancelCallback{};
            void* RaceDll{};
            void* ActivationContext{};
            void* FinalizationCallback{};
            union
            {
                uint32_t Flags{};
                struct
                {
                    uint32_t LongFunction : 1;
                    uint32_t Persistent : 1;
                    uint32_t Private : 30;
                } s;
            } u;
            int32_t CallbackPriority{ 1 };
            uint32_t Size{ sizeof(environment) };
        };

        handle_type<pool_traits> m_pool;
        environment m_environment;
    };

    struct fire_and_forget {};
}

namespace std::experimental
{
    template <typename... Args>
    struct coroutine_traits<winrt::fire_and_forget, Args...>
    {
        struct promise_type
        {
            winrt::fire_and_forget get_return_object() const noexcept
            {
                return{};
            }

            void return_void() const noexcept
            {
            }

            suspend_never initial_suspend() const noexcept
            {
                return{};
            }

            suspend_never final_suspend() const noexcept
            {
                return{};
            }

            void unhandled_exception() const noexcept
            {
                std::terminate();
            }
        };
    };
}
