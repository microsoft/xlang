#pragma once

#include <string>
#include <string_view>
#include <variant>

#include "model_ref.h"
#include "meta_reader.h"
#include "base_model.h"

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
        std::shared_ptr<xlang::meta::reader::TypeDef>,
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

        void set_semantic(class_type_semantics const& sem) noexcept
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
            if (std::holds_alternative<std::shared_ptr<xlang::meta::reader::TypeDef>>(sem))
            {
                m_semantic.resolve(std::get<std::shared_ptr<xlang::meta::reader::TypeDef>>(sem));
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

        bool operator==(type_ref const& right_ref)
        {
            //if (m_semantic.is_resolved() && right_ref.get_semantic().is_resolved())
            //{
            //    type_semantics left_type = m_semantic.get_resolved_target();

            //    type_semantics right_type = right_ref.get_semantic().get_resolved_target();
            //    if (std::holds_alternative<simple_type>(left_type) && std::holds_alternative<simple_type>(right_type))
            //    {
            //        return std::get<simple_type>(left_type) == std::get<simple_type>(right_type);
            //    }
            //    if (std::holds_alternative<object_type>(left_type) && std::holds_alternative<object_type>(right_type))
            //    {
            //        return true;
            //    }
            //    if (std::holds_alternative<std::shared_ptr<class_model>>(left_type) && std::holds_alternative<std::shared_ptr<class_model>>(right_type))
            //    {
            //        return std::get<std::shared_ptr<class_model>>(left_type)->get_fully_qualified_id() == std::get<std::shared_ptr<class_model>>(right_type)->get_fully_qualified_id();
            //    }
            //    if (std::holds_alternative<std::shared_ptr<interface_model>>(left_type) && std::holds_alternative<std::shared_ptr<interface_model>>(right_type))
            //    {
            //        return std::get<std::shared_ptr<interface_model>>(left_type)->get_fully_qualified_id() == std::get<std::shared_ptr<interface_model>>(right_type)->get_fully_qualified_id();
            //    }
            //    if (std::holds_alternative<std::shared_ptr<enum_model>>(left_type) && std::holds_alternative<std::shared_ptr<enum_model>>(right_type))
            //    {
            //        return std::get<std::shared_ptr<enum_model>>(left_type)->get_fully_qualified_id() == std::get<std::shared_ptr<enum_model>>(right_type)->get_fully_qualified_id();
            //    }
            //    if (std::holds_alternative<std::shared_ptr<struct_model>>(left_type) && std::holds_alternative<std::shared_ptr<struct_model>>(right_type))
            //    {
            //        return std::get<std::shared_ptr<struct_model>>(left_type)->get_fully_qualified_id() == std::get<std::shared_ptr<struct_model>>(right_type)->get_fully_qualified_id();
            //    }
            //    if (std::holds_alternative<std::shared_ptr<delegate_model>>(left_type) && std::holds_alternative<std::shared_ptr<delegate_model>>(right_type))
            //    {
            //        return std::get<std::shared_ptr<delegate_model>>(left_type)->get_fully_qualified_id() == std::get<std::shared_ptr<delegate_model>>(right_type)->get_fully_qualified_id();
            //    }
            //    if (std::holds_alternative< std::shared_ptr<xlang::meta::reader::TypeDef>>(left_type) && std::holds_alternative<std::shared_ptr<xlang::meta::reader::TypeDef>>(right_type))
            //    {
            //        assert(false);
            //        // TODO check type
            //        return std::get<xlang::meta::reader::TypeDef>(left_type).FullyQualifiedName() && std::get<xlang::meta::reader::TypeDef>(right_type).FullyQualifiedName();
            //    }
            //}
            //return false;
            return false;
        }

    private:
        model_ref<type_semantics> m_semantic;
    };
}
