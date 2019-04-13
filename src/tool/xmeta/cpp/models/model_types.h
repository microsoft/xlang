#pragma once

#include <string>
#include <string_view>
#include <variant>

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
        Uint8,
        Uint16,
        Uint32,
        Uint64,
        Char16,
        Guid,
        Single,
        Double,
    };

    struct object_type {};

    using type_semantics = std::variant<
        std::shared_ptr<class_model>,
        std::shared_ptr<enum_model>,
        std::shared_ptr<interface_model>,
        std::shared_ptr<struct_model>,
        simple_type,
        object_type>;

    struct type_ref
    {
        type_ref(std::string_view const& id) : m_semantic{ std::string(id) } { }
        type_ref(type_ref&& ref) = default;

        void set_type_semantics(type_semantics const& sem)
        {
            m_semantic = sem;
        }

    private:
        std::variant<std::string, type_semantics> m_semantic;
    };
}