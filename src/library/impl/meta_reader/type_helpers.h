
namespace xlang::meta::reader
{
    inline std::pair<std::string_view, std::string_view> get_base_class_namespace_and_name(TypeDef type)
    {
        auto const extends = type.Extends();
        if (extends.type() == TypeDefOrRef::TypeDef)
        {
            auto const def = extends.TypeDef();
            return { def.TypeNamespace(), def.TypeName() };
        }
        else if (extends.type() == TypeDefOrRef::TypeRef)
        {
            auto const ref = extends.TypeRef();
            return { ref.TypeNamespace(), ref.TypeName() };
        }
        else
        {
            return {};
        }
    }

    inline auto extends_type(TypeDef type, std::string_view typeNamespace, std::string_view typeName)
    {
        return get_base_class_namespace_and_name(type) == std::pair(typeNamespace, typeName);
    }

    enum class category
    {
        interface_type,
        class_type,
        enum_type,
        struct_type,
        delegate_type
    };

    inline category get_category(TypeDef const& type)
    {
        if (type.Flags().Semantics() == TypeSemantics::Interface)
        {
            return category::interface_type;
        }

        auto const& [extends_namespace, extends_name] = get_base_class_namespace_and_name(type);

        if (extends_name == "Enum"sv && extends_namespace == "System"sv)
        {
            return category::enum_type;
        }

        if (extends_name == "ValueType"sv && extends_namespace == "System"sv)
        {
            return category::struct_type;
        }

        if (extends_name == "MulticastDelegate"sv && extends_namespace == "System"sv)
        {
            return category::delegate_type;
        }

        return category::class_type;
    }
}
