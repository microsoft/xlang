
namespace xlang::meta::reader
{
    inline auto extends_type(TypeDef type, std::string_view typeNamespace, std::string_view typeName)
    {
        auto const extends = type.Extends().TypeRef();
        return (extends.TypeNamespace() == typeNamespace && extends.TypeName() == typeName);
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

        auto const extends = type.Extends().TypeRef();
        auto extends_name = extends.TypeName();
        auto extends_namespace = extends.TypeNamespace();

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
