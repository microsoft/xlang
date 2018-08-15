
#ifdef WINRT_DIAGNOSTICS

WINRT_EXPORT namespace winrt
{
    struct factory_diagnostics_info
    {
        bool is_agile{ true };
        uint32_t requests{ 0 };
    };

    struct diagnostics_info
    {
        std::map<std::wstring_view, uint32_t> queries;
        std::map<std::wstring_view, factory_diagnostics_info> factories;
    };
}

namespace winrt::impl
{
    struct diagnostics_info
    {
        template <typename T>
        void add_query()
        {
            slim_lock_guard const guard(m_lock);
            ++m_info.queries[name_of<T>()];
        }

        template <typename T>
        void add_factory()
        {
            slim_lock_guard const guard(m_lock);
            factory_diagnostics_info& factory = m_info.factories[name_of<T>()];
            ++factory.requests;
        }

        template <typename T>
        void non_agile_factory()
        {
            slim_lock_guard const guard(m_lock);
            factory_diagnostics_info& factory = m_info.factories[name_of<T>()];
            factory.is_agile = false;
        }

        auto get()
        {
            slim_lock_guard const guard(m_lock);
            return m_info;
        }

        auto detach()
        {
            slim_lock_guard const guard(m_lock);
            return std::move(m_info);
        }

    private:

        slim_mutex m_lock;
        winrt::diagnostics_info m_info;
    };

    inline diagnostics_info& get_diagnostics_info() noexcept
    {
        static diagnostics_info info;
        return info;
    }
}

WINRT_EXPORT namespace winrt
{
    inline auto get_diagnostics_info()
    {
        return impl::get_diagnostics_info().get();
    }

    inline auto detach_diagnostics_info()
    {
        return impl::get_diagnostics_info().detach();
    }
}

#endif
