#pragma once

#include "pal_internal.h"
#include "atomic_ref_count.h"

namespace xlang
{
    namespace impl
    {
        struct error_info : xlang_error_info
        {
            explicit error_info(xlang_result result) noexcept
                : m_result(result)
            {}

            int32_t XLANG_CALL QueryInterface(xlang_guid const& id, void** object) noexcept override
            {
                if (id == xlang_unknown_guid)
                {
                    *object = static_cast<xlang_unknown*>(this);
                }
                else if (id == xlang_error_info_guid)
                {
                    *object = static_cast<xlang_error_info*>(this);
                }
                else
                {
                    *object = nullptr;
                    return 0x80004002;
                }
                AddRef();
                return 0;
            }

            uint32_t XLANG_CALL AddRef() noexcept override
            {
                return ++m_count;
            }

            uint32_t XLANG_CALL Release() noexcept override
            {
                auto result = --m_count;
                if (result == 0)
                {
                    delete this;
                }
                return result;
            }

            xlang_result error_code() noexcept override
            {
                return m_result;
            }

        private:
            xlang_result m_result{};
            atomic_ref_count m_count;
        };
    }

    [[noreturn]] inline void throw_result(xlang_result result)
    {
        throw new impl::error_info{ result };
    }

    [[noreturn]] inline void throw_result(xlang_error_info* result)
    {
        throw result;
    }

    // TODO(defaultryan) Replace with xlang projection methods
    inline xlang_error_info* to_result() noexcept
    {
        try
        {
            throw;
        }
        catch (xlang_error_info* e)
        {
            return e;
        }
        catch (std::bad_alloc const&)
        {
            return new impl::error_info{ xlang_error_out_of_memory };
        }
        catch (...)
        {
            std::terminate();
        }
    }
}