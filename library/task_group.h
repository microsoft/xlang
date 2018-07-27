#pragma once

#include "impl/base.h"

namespace xlang
{
    struct task_group
    {
        task_group(task_group const&) = delete;
        task_group& operator=(task_group const&) = delete;

        task_group()
        {
            InitializeThreadpoolEnvironment(&m_environment);
            m_group = CreateThreadpoolCleanupGroup();

            if (!m_group)
            {
                throw std::bad_alloc();
            }

            SetThreadpoolCallbackCleanupGroup(&m_environment, m_group, [](void* task_ptr, void* group_ptr) noexcept
            {
                auto task = static_cast<task_base*>(task_ptr);
                auto group = static_cast<task_group*>(group_ptr);

                if (group && !group->m_exception)
                {
                    group->m_exception = task->exception;
                }

                delete task;
            });
        }

        ~task_group() noexcept
        {
            // Cancels any pending tasks and waits for any executing tasks before cleaning up.
            CloseThreadpoolCleanupGroupMembers(m_group, true, nullptr);
            CloseThreadpoolCleanupGroup(m_group);
            DestroyThreadpoolEnvironment(&m_environment);
        }

        template <typename T>
        void add(T&& callback)
        {
            // Transfers ownership of the lambda and submits it to the thread pool.
            auto work = CreateThreadpoolWork([](PTP_CALLBACK_INSTANCE, void* task_ptr, PTP_WORK) noexcept
            {
                static_cast<task_base*>(task_ptr)->invoke();
            },
                static_cast<task_base*>(new task<T>(std::forward<T>(callback))),
                &m_environment);

            if (!work)
            {
                throw std::bad_alloc();
            }

            SubmitThreadpoolWork(work);
        }

        void get()
        {
            // Waits for all callbacks to complete and rethrows the first exception that was thrown by a callback.
            CloseThreadpoolCleanupGroupMembers(m_group, false, this);

            if (m_exception)
            {
                std::rethrow_exception(m_exception);
            }
        }

    private:

        struct task_base
        {
            virtual ~task_base() noexcept = default;
            virtual void invoke() noexcept = 0;

            std::exception_ptr exception;
        };

        template <typename T>
        struct task final : task_base, T
        {
            task(T&& callback) : T(std::forward<T>(callback))
            {
            }

            void invoke() noexcept final
            {
                try
                {
                    (*this)();
                }
                catch (...)
                {
                    exception = std::current_exception();
                }
            }
        };

        TP_CALLBACK_ENVIRON m_environment{};
        PTP_CLEANUP_GROUP m_group{};
        std::exception_ptr m_exception{};
    };
}
