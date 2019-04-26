#include "pal_internal.h"
#include "pal_error.h"
#include "atomic_ref_count.h"
#include <xlang/base.h>

namespace xlang::impl
{
    struct error_info : xlang_error_info
    {
        // Used to construct constant errors used in out of memory scenarios.
        explicit error_info(xlang_result result) noexcept :
            m_result{ result },
            m_modifiable{ false }
        {
        }

        explicit error_info(
            xlang_result result,
            xlang_string message,
            xlang_string projection_identifier,
            xlang_string language_error,
            xlang_unknown* execution_trace,
            xlang_unknown* language_information
        ) noexcept :
            m_result{ result },
            m_message{ message },
            m_projection_identifier{ projection_identifier },
            m_language_error{ language_error }
        {
            m_execution_trace.copy_from(execution_trace);
            m_language_information.copy_from(language_information);
        }

        int32_t XLANG_CALL QueryInterface(xlang_guid const& id, void** object) noexcept final
        {
            if (id == xlang_unknown_guid)
            {
                static_assert(std::is_base_of_v<xlang_unknown, xlang_error_info>, "Can only combine these two cases if this is true.");
                *object = static_cast<xlang_unknown*>(this);
            }
            else if (id == xlang_error_info_guid)
            {
                *object = static_cast<xlang_error_info*>(this);
            }
            else
            {
                *object = nullptr;
                return xlang_hresult_no_interface;
            }
            AddRef();
            return 0;
        }

        uint32_t XLANG_CALL AddRef() noexcept final
        {
            return ++m_count;
        }

        uint32_t XLANG_CALL Release() noexcept final
        {
            auto result = --m_count;
            if (result == 0)
            {
                delete this;
            }
            return result;
        }

        void get_error(xlang_result* error) noexcept override
        {
            *error = m_result;
        }

        void get_message(xlang_string* message) noexcept override
        {
            *message = m_message;
        }

        void get_language_error(xlang_string* language_error) noexcept override
        {
            *language_error = m_language_error;
        }

        void get_execution_trace(xlang_unknown** execution_trace) noexcept override
        {
            m_execution_trace.copy_to(execution_trace);
        }

        void get_projection_identifier(xlang_string* projection_identifier) noexcept override
        {
            *projection_identifier = m_projection_identifier;
        }

        void get_language_information(xlang_unknown** language_information) noexcept override
        {
            m_language_information.copy_to(language_information);
        }

        void get_propagated_error(xlang_error_info** propagated_error) noexcept override
        {
            m_next_propagated_error.copy_to(propagated_error);
        }

        void propagate_error(
            xlang_string projection_identifier,
            xlang_string language_error,
            xlang_unknown* execution_trace,
            xlang_unknown* language_information
        ) noexcept override
        {
            // If the error info can't be modified, don't collect infomration on propagations
            // as this error info can be reused for another error.
            if (!m_modifiable)
            {
                return;
            }

            com_ptr<xlang_error_info> propagated_error;
            propagated_error.attach(
                xlang_originate_error(
                    m_result,
                    m_message,
                    projection_identifier,
                    language_error,
                    execution_trace,
                    language_information));

            error_info* last_propagated_error = this;
            while (last_propagated_error->m_next_propagated_error != nullptr)
            {
                last_propagated_error = static_cast<error_info*>(last_propagated_error->m_next_propagated_error.get());
            }
            last_propagated_error->m_next_propagated_error = propagated_error;
        }

    private:
        xlang_result m_result{};
        xlang_string m_message{ nullptr };
        xlang_string m_language_error{ nullptr };
        com_ptr<xlang_unknown> m_execution_trace;
        xlang_string m_projection_identifier{ nullptr };
        com_ptr<xlang_unknown> m_language_information;
        com_ptr<xlang_error_info> m_next_propagated_error;
        bool m_modifiable{ true };
        atomic_ref_count m_count;
    };

    error_info error_code_errors [] = {
        error_info {xlang_result::xlang_access_denied},
        error_info {xlang_result::xlang_bounds},
        error_info {xlang_result::xlang_fail},
        error_info {xlang_result::xlang_handle},
        error_info {xlang_result::xlang_invalid_arg},
        error_info {xlang_result::xlang_invalid_state},
        error_info {xlang_result::xlang_no_interface},
        error_info {xlang_result::xlang_not_impl},
        error_info {xlang_result::xlang_out_of_memory},
        error_info {xlang_result::xlang_pointer},
        error_info {xlang_result::xlang_type_load}
    };
}

[[nodiscard]] XLANG_PAL_EXPORT xlang_error_info* XLANG_CALL xlang_originate_error(
    xlang_result error,
    xlang_string message,
    xlang_string projection_identifier,
    xlang_string language_error,
    xlang_unknown* execution_trace,
    xlang_unknown* language_information
) XLANG_NOEXCEPT
{
    xlang_error_info* error_info =
        new (std::nothrow) xlang::impl::error_info
    {
        error,
        message,
        projection_identifier,
        language_error,
        execution_trace,
        language_information
    };

    // If failed to construct, use the statically allocated ones.
    if (error_info == nullptr)
    {
        error_info = &xlang::impl::error_code_errors[static_cast<int>(error)];
        error_info->AddRef();
    }

    return error_info;
}