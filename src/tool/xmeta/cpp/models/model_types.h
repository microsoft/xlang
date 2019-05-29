#pragma once

#include <string>
#include <string_view>
#include <variant>

#include "base_model.h"
#include "model_ref.h"

namespace xlang::xmeta
{
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

        void set_semantic(type_semantics const& sem) noexcept;

        void set_semantic(class_type_semantics const& sem) noexcept;

        void set_semantic(std::shared_ptr<class_model> const& sem) noexcept;

        void set_semantic(std::shared_ptr<enum_model> const& sem) noexcept;

        void set_semantic(std::shared_ptr<interface_model> const& sem) noexcept;

        void set_semantic(std::shared_ptr<struct_model> const& sem) noexcept;

        void set_semantic(std::shared_ptr<delegate_model> const& sem) noexcept;

        void set_semantic(fundamental_type st);

        void set_semantic(object_type o);

        // To be use for testing purposes only
        bool operator==(type_ref const& right_ref) const;

    private:
        model_ref<type_semantics> m_semantic;
    };
}
