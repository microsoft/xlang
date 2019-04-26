#pragma once

#include <string>
#include <string_view>
#include <variant>

#include "model_ref.h"

namespace xlang::xmeta
{
    struct class_model;
    struct enum_model;
    struct interface_model;
    struct struct_model;

    enum class simple_type
    {
        Boolean,
        String,
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Char16,
        Guid,
        Single,
        Double,
    };

    struct object_type {};

    using type_semantics = model_ref<
        std::shared_ptr<class_model>,
        std::shared_ptr<enum_model>,
        std::shared_ptr<interface_model>,
        std::shared_ptr<struct_model>,
        simple_type,
        object_type>;

    struct type_ref
    {
        type_ref() = delete;
        type_ref(std::string_view const& id) :
            m_semantic{ std::string(id) }
        { }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        void set_semantic(std::shared_ptr<class_model> const& sem) noexcept
        {
            m_semantic.resolve(sem);
        }

        void set_semantic(std::shared_ptr<enum_model> const& sem) noexcept
        {
            m_semantic.resolve(sem);
        }

        void set_semantic(std::shared_ptr<interface_model> const& sem) noexcept
        {
            m_semantic.resolve(sem);
        }

        void set_semantic(std::shared_ptr<struct_model> const& sem) noexcept
        {
            m_semantic.resolve(sem);
        }

        void set_semantic(simple_type st)
        {
            m_semantic.resolve(st);
        }

        void set_semantic(object_type o)
        {
            m_semantic.resolve(o);
        }

    private:
        type_semantics m_semantic;
    };
}
