
namespace xlang::impl
{
    constexpr int32_t hresult_from_win32(uint32_t const x) noexcept
    {
        return (int32_t)(x) <= 0 ? (int32_t)(x) : (int32_t)(((x) & 0x0000FFFF) | (7 << 16) | 0x80000000);
    }

    constexpr int32_t hresult_from_nt(uint32_t const x) noexcept
    {
        return ((int32_t)((x) | 0x10000000));
    }

    constexpr int32_t hresult_from_xlang_result(xlang_result const result) noexcept
    {
        switch (result)
        {
        case xlang_result::xlang_success:
            return error_ok;
        case xlang_result::xlang_access_denied:
            return error_access_denied;
        case xlang_result::xlang_bounds:
            return error_out_of_bounds;
        case xlang_result::xlang_fail:
            return error_fail;
        case xlang_result::xlang_handle:
            return static_cast<hresult>(0x80070006);
        case xlang_result::xlang_invalid_arg:
            return error_invalid_argument;
        case xlang_result::xlang_invalid_state:
            return error_illegal_state_change;
        case xlang_result::xlang_no_interface:
            return error_no_interface;
        case xlang_result::xlang_not_impl:
            return error_not_implemented;
        case xlang_result::xlang_out_of_memory:
            return error_bad_alloc;
        case xlang_result::xlang_pointer:
            return static_cast<hresult>(0x80004003);
        case xlang_result::xlang_type_load:
            return static_cast<hresult>(0x80131522);
        }

        return error_fail;
    }
}

namespace xlang
{
    struct hresult_error
    {
        using from_abi_t = take_ownership_from_abi_t;
        static constexpr auto from_abi{ take_ownership_from_abi };

        hresult_error() noexcept = default;
        hresult_error(hresult_error&&) = default;
        hresult_error& operator=(hresult_error&&) = default;

        hresult_error(hresult_error const& other) noexcept :
            m_code(other.m_code)
        {
        }

        hresult_error& operator=(hresult_error const& other) noexcept
        {
            m_code = other.m_code;
            return *this;
        }

        explicit hresult_error(hresult const code) noexcept : m_code(code)
        {
            originate(code, nullptr);
        }

        hresult_error(hresult const code, param::hstring const& message) noexcept : m_code(code)
        {
            originate(code, get_abi(message));
        }

        hresult_error(hresult const code, take_ownership_from_abi_t) noexcept : m_code(code)
        {
            originate(code, nullptr);
        }

        hresult code() const noexcept
        {
            return m_code;
        }

        hresult to_abi() const noexcept
        {
            return m_code;
        }

    private:

        void originate(hresult const, void*) noexcept
        {
        }

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif

        uint32_t const m_debug_magic{ 0xAABBCCDD };
        hresult m_code{ impl::error_fail };

#ifdef __clang__
#pragma clang diagnostic pop
#endif
    };

    struct hresult_access_denied : hresult_error
    {
        hresult_access_denied() noexcept : hresult_error(impl::error_access_denied) {}
        hresult_access_denied(param::hstring const& message) noexcept : hresult_error(impl::error_access_denied, message) {}
        hresult_access_denied(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_access_denied, take_ownership_from_abi) {}
    };

    struct hresult_wrong_thread : hresult_error
    {
        hresult_wrong_thread() noexcept : hresult_error(impl::error_wrong_thread) {}
        hresult_wrong_thread(param::hstring const& message) noexcept : hresult_error(impl::error_wrong_thread, message) {}
        hresult_wrong_thread(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_wrong_thread, take_ownership_from_abi) {}
    };

    struct hresult_not_implemented : hresult_error
    {
        hresult_not_implemented() noexcept : hresult_error(impl::error_not_implemented) {}
        hresult_not_implemented(param::hstring const& message) noexcept : hresult_error(impl::error_not_implemented, message) {}
        hresult_not_implemented(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_not_implemented, take_ownership_from_abi) {}
    };

    struct hresult_invalid_argument : hresult_error
    {
        hresult_invalid_argument() noexcept : hresult_error(impl::error_invalid_argument) {}
        hresult_invalid_argument(param::hstring const& message) noexcept : hresult_error(impl::error_invalid_argument, message) {}
        hresult_invalid_argument(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_invalid_argument, take_ownership_from_abi) {}
    };

    struct hresult_out_of_bounds : hresult_error
    {
        hresult_out_of_bounds() noexcept : hresult_error(impl::error_out_of_bounds) {}
        hresult_out_of_bounds(param::hstring const& message) noexcept : hresult_error(impl::error_out_of_bounds, message) {}
        hresult_out_of_bounds(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_out_of_bounds, take_ownership_from_abi) {}
    };

    struct hresult_no_interface : hresult_error
    {
        hresult_no_interface() noexcept : hresult_error(impl::error_no_interface) {}
        hresult_no_interface(param::hstring const& message) noexcept : hresult_error(impl::error_no_interface, message) {}
        hresult_no_interface(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_no_interface, take_ownership_from_abi) {}
    };

    struct hresult_class_not_available : hresult_error
    {
        hresult_class_not_available() noexcept : hresult_error(impl::error_class_not_available) {}
        hresult_class_not_available(param::hstring const& message) noexcept : hresult_error(impl::error_class_not_available, message) {}
        hresult_class_not_available(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_class_not_available, take_ownership_from_abi) {}
    };

    struct hresult_changed_state : hresult_error
    {
        hresult_changed_state() noexcept : hresult_error(impl::error_changed_state) {}
        hresult_changed_state(param::hstring const& message) noexcept : hresult_error(impl::error_changed_state, message) {}
        hresult_changed_state(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_changed_state, take_ownership_from_abi) {}
    };

    struct hresult_illegal_method_call : hresult_error
    {
        hresult_illegal_method_call() noexcept : hresult_error(impl::error_illegal_method_call) {}
        hresult_illegal_method_call(param::hstring const& message) noexcept : hresult_error(impl::error_illegal_method_call, message) {}
        hresult_illegal_method_call(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_illegal_method_call, take_ownership_from_abi) {}
    };

    struct hresult_illegal_state_change : hresult_error
    {
        hresult_illegal_state_change() noexcept : hresult_error(impl::error_illegal_state_change) {}
        hresult_illegal_state_change(param::hstring const& message) noexcept : hresult_error(impl::error_illegal_state_change, message) {}
        hresult_illegal_state_change(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_illegal_state_change, take_ownership_from_abi) {}
    };

    struct hresult_illegal_delegate_assignment : hresult_error
    {
        hresult_illegal_delegate_assignment() noexcept : hresult_error(impl::error_illegal_delegate_assignment) {}
        hresult_illegal_delegate_assignment(param::hstring const& message) noexcept : hresult_error(impl::error_illegal_delegate_assignment, message) {}
        hresult_illegal_delegate_assignment(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_illegal_delegate_assignment, take_ownership_from_abi) {}
    };

    struct hresult_canceled : hresult_error
    {
        hresult_canceled() noexcept : hresult_error(impl::error_canceled) {}
        hresult_canceled(param::hstring const& message) noexcept : hresult_error(impl::error_canceled, message) {}
        hresult_canceled(take_ownership_from_abi_t) noexcept : hresult_error(impl::error_canceled, take_ownership_from_abi) {}
    };

    [[noreturn]] inline XLANG_NOINLINE void throw_hresult(hresult const result)
    {
        if (result == impl::error_bad_alloc)
        {
            throw std::bad_alloc();
        }

        if (result == impl::error_access_denied)
        {
            throw hresult_access_denied(take_ownership_from_abi);
        }

        if (result == impl::error_wrong_thread)
        {
            throw hresult_wrong_thread(take_ownership_from_abi);
        }

        if (result == impl::error_not_implemented)
        {
            throw hresult_not_implemented(take_ownership_from_abi);
        }

        if (result == impl::error_invalid_argument)
        {
            throw hresult_invalid_argument(take_ownership_from_abi);
        }

        if (result == impl::error_out_of_bounds)
        {
            throw hresult_out_of_bounds(take_ownership_from_abi);
        }

        if (result == impl::error_no_interface)
        {
            throw hresult_no_interface(take_ownership_from_abi);
        }

        if (result == impl::error_class_not_available)
        {
            throw hresult_class_not_available(take_ownership_from_abi);
        }

        if (result == impl::error_changed_state)
        {
            throw hresult_changed_state(take_ownership_from_abi);
        }

        if (result == impl::error_illegal_method_call)
        {
            throw hresult_illegal_method_call(take_ownership_from_abi);
        }

        if (result == impl::error_illegal_state_change)
        {
            throw hresult_illegal_state_change(take_ownership_from_abi);
        }

        if (result == impl::error_illegal_delegate_assignment)
        {
            throw hresult_illegal_delegate_assignment(take_ownership_from_abi);
        }

        if (result == impl::error_canceled)
        {
            throw hresult_canceled(take_ownership_from_abi);
        }

        throw hresult_error(result, take_ownership_from_abi);
    }

    inline XLANG_NOINLINE hresult to_hresult() noexcept
    {
        try
        {
            throw;
        }
        catch (hresult_error const& e)
        {
            return e.to_abi();
        }
        XLANG_EXTERNAL_CATCH_CLAUSE
        catch (std::bad_alloc const&)
        {
            return impl::error_bad_alloc;
        }
        catch (std::out_of_range const& e)
        {
            return hresult_out_of_bounds(to_hstring(e.what())).to_abi();
        }
        catch (std::invalid_argument const& e)
        {
            return hresult_invalid_argument(to_hstring(e.what())).to_abi();
        }
        catch (std::exception const& e)
        {
            return hresult_error(impl::error_fail, to_hstring(e.what())).to_abi();
        }
        catch (...)
        {
            std::terminate();
        }
    }

    inline void check_hresult(hresult const result)
    {
        if (result < 0)
        {
            throw_hresult(result);
        }
    }

    inline void check_hresult(xlang_error_info* result)
    {
        if (result != nullptr)
        {
            xlang_result error;
            result->GetError(&error);
            throw_hresult(impl::hresult_from_xlang_result(error));
        }
    }

#ifdef _WIN32
    [[noreturn]] inline void throw_last_error()
    {
        throw_hresult(impl::hresult_from_win32(XLANG_GetLastError()));
    }

    template<typename T>
    void check_nt(T result)
    {
        if (result != 0)
        {
            throw_hresult(impl::hresult_from_nt(result));
        }
    }

    template<typename T>
    void check_win32(T result)
    {
        if (result != 0)
        {
            throw_hresult(impl::hresult_from_win32(result));
        }
    }

    template<typename T>
    void check_bool(T result)
    {
        if (!result)
        {
            xlang::throw_last_error();
        }
    }

    template<typename T>
    T* check_pointer(T* pointer)
    {
        if (!pointer)
        {
            throw_last_error();
        }

        return pointer;
    }
#endif
}
