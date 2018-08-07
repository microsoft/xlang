#pragma once

namespace xlang
{
    struct default_sentinel {};

    template <typename T>
    struct generator
    {
        struct promise_type
        {
            T const* value{};
            std::exception_ptr ex{};

            promise_type& get_return_object() noexcept
            {
                return *this;
            }

            std::experimental::suspend_always initial_suspend() const noexcept
            {
                return {};
            }

            std::experimental::suspend_always final_suspend() const noexcept
            {
                return {};
            }

            std::experimental::suspend_always yield_value(T const& other) noexcept
            {
                value = std::addressof(other);
                return {};
            }

            void return_void() const noexcept
            {
            }

            template <typename Expression>
            Expression&& await_transform(Expression&& expression) const noexcept
            {
                static_assert(sizeof(expression) == 0, "co_await is not supported in coroutines of type generator");
                return std::forward<Expression>(expression);
            }

            void unhandled_exception() noexcept
            {
                ex = std::current_exception();
            }

            void rethrow_if_failed() const
            {
                if (ex)
                {
                    std::rethrow_exception(ex);
                }
            }
        };

        using handle_type = std::experimental::coroutine_handle<promise_type>;

        struct iterator
        {
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T const*;
            using reference = T const&;

            handle_type m_handle{};

            iterator() noexcept = default;

            explicit iterator(handle_type handle_type) noexcept : m_handle(handle_type)
            {
            }

            iterator(default_sentinel) noexcept : iterator{}
            {
            }

            iterator &operator++()
            {
                m_handle.resume();

                if (m_handle.done())
                {
                    promise_type& promise = m_handle.promise();
                    m_handle = {};
                    promise.rethrow_if_failed();
                }

                return *this;
            }

            void operator++(int)
            {
                ++*this;
            }

            bool operator==(iterator const& other) const noexcept
            {
                return m_handle == other.m_handle;
            }

            bool operator!=(iterator const& other) const noexcept
            {
                return !(*this == other);
            }

            friend bool operator==(iterator const& it, default_sentinel) noexcept
            {
                return !it.m_handle;
            }
            friend bool operator==(default_sentinel, iterator const& it) noexcept
            {
                return !it.m_handle;
            }
            friend bool operator!=(iterator const& it, default_sentinel) noexcept
            {
                return !(it == default_sentinel{});
            }
            friend bool operator!=(default_sentinel, iterator const& it) noexcept
            {
                return !(it == default_sentinel{});
            }

            T const& operator*() const noexcept
            {
                return *m_handle.promise().value;
            }

            T const* operator->() const noexcept
            {
                return std::addressof(operator*());
            }
        };

        iterator begin()
        {
            if (m_handle)
            {
                m_handle.resume();

                if (m_handle.done())
                {
                    promise_type& promise = m_handle.promise();
                    m_handle = {};
                    promise.rethrow_if_failed();
                }
            }


            return iterator{m_handle};
        }

        default_sentinel end() noexcept
        {
            return {};
        }

        generator(promise_type& promise) noexcept : m_handle(handle_type::from_promise(promise))
        {
        }

        generator() = default;
        generator(generator const&) = delete;
        generator &operator=(generator const&) = delete;

        generator(generator&& other) noexcept : m_handle(std::exchange(other.m_handle, {}))
        {
        }

        generator &operator=(generator&& other) noexcept
        {
            if (this != &other)
            {
                if (m_handle)
                {
                    m_handle.destroy();
                }

                m_handle = std::exchange(other.m_handle, {});
            }

            return *this;
        }

        ~generator() noexcept
        {
            if (m_handle)
            {
                m_handle.destroy();
            }
        }

    private:

        handle_type m_handle{};
    };
}
