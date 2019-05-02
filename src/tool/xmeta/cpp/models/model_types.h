#pragma once

#include <string>
#include <string_view>
#include <variant>

#include "model_ref.h"

namespace xlang::xmeta
{
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
        Single,
        Double,
    };

    struct object_type {};

    using type_semantics = std::variant<
        std::shared_ptr<class_model>,
        std::shared_ptr<enum_model>,
        std::shared_ptr<interface_model>,
        std::shared_ptr<struct_model>,
        std::shared_ptr<delegate_model>,
        simple_type,
        object_type>;

    struct type_ref // TODO: needs to have decl
    {
        type_ref() = delete;
        type_ref(std::string_view const& id) :
            m_semantic{ std::string(id) }
        { }

        type_ref(type_semantics const& type) :
            m_semantic{ std::string() }
        {
            m_semantic.resolve(type);
        }

        auto const& get_semantic() const noexcept
        {
            return m_semantic;
        }

        void set_semantic(type_semantics const& sem) noexcept
        {
            m_semantic.resolve(sem);
        }

        void set_semantic(std::variant<
            std::shared_ptr<class_model>,
            std::shared_ptr<enum_model>,
            std::shared_ptr<interface_model>,
            std::shared_ptr<struct_model>,
            std::shared_ptr<delegate_model>> const& sem) noexcept
        {
            if (std::holds_alternative<std::shared_ptr<delegate_model>>(sem))
            {
                m_semantic.resolve(std::get<std::shared_ptr<delegate_model>>(sem));
            }
            if (std::holds_alternative<std::shared_ptr<class_model>>(sem))
            {
                m_semantic.resolve(std::get<std::shared_ptr<class_model>>(sem));
            }
            if (std::holds_alternative<std::shared_ptr<interface_model>>(sem))
            {
                m_semantic.resolve(std::get<std::shared_ptr<interface_model>>(sem));
            }
            if (std::holds_alternative<std::shared_ptr<enum_model>>(sem))
            {
                m_semantic.resolve(std::get<std::shared_ptr<enum_model>>(sem));
            }
            if (std::holds_alternative<std::shared_ptr<struct_model>>(sem))
            {
                m_semantic.resolve(std::get<std::shared_ptr<struct_model>>(sem));
            }
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

        void set_semantic(std::shared_ptr<delegate_model> const& sem) noexcept
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
        model_ref<type_semantics> m_semantic;
    };
}
