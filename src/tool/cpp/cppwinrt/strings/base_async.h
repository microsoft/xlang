
namespace winrt::impl
{
    inline bool is_sta() noexcept
    {
        int32_t aptType;
        int32_t aptTypeQualifier;
        return (error_ok == WINRT_CoGetApartmentType(&aptType, &aptTypeQualifier)) && ((aptType == 0 /*APTTYPE_STA*/) || (aptType == 3 /*APTTYPE_MAINSTA*/));
    }

    template <typename Async>
    void blocking_suspend(Async const& async)
    {
        WINRT_ASSERT(!is_sta());

        slim_mutex m;
        slim_condition_variable cv;
        bool completed = false;
        async.Completed([&](auto && ...)
            {
                {
                    slim_lock_guard const guard(m);
                    completed = true;
                }
                cv.notify_one();
            });

        slim_lock_guard guard(m);
        cv.wait(m, [&] { return completed; });
    }
}
