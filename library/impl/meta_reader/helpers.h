
namespace xlang::meta::reader
{
    template <typename T>
    bool empty(std::pair<T, T> const& range) noexcept
    {
        return range.first == range.second;
    }

    template <typename T>
    std::size_t size(std::pair<T, T> const& range) noexcept
    {
        return range.second - range.first;
    }

    inline auto find(TypeRef const& type)
    {
        return type.get_database().get_cache().find(type.TypeNamespace(), type.TypeName());
    }

    inline bool is_const(ParamSig const& param)
    {
        auto is_type_const = [](auto&& type)
        {
            return type.TypeNamespace() == "System.Runtime.CompilerServices" && type.TypeName() == "IsConst";
        };

        for (auto const& cmod : param.CustomMod())
        {
            auto type = cmod.Type();

            if (type.type() == TypeDefOrRef::TypeDef)
            {
                if (is_type_const(type.TypeDef()))
                {
                    return true;
                }
            }
            else if (type.type() == TypeDefOrRef::TypeRef)
            {
                if (is_type_const(type.TypeRef()))
                {
                    return true;
                }
            }
        }

        return false;
    };

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
        if (enum_mask(type.Flags(), TypeAttributes::Interface) == TypeAttributes::Interface)
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
