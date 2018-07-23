
namespace xlang::meta::reader
{
    template <typename T>
    template <typename Row>
    Row index_base<T>::get_row() const
    {
        WINRT_ASSERT(type() == (index_tag_v<T, Row>));
        return get_database().get_table<Row>()[index()];
    }

    inline auto typed_index<TypeDefOrRef>::TypeDef() const
    {
        return get_row<reader::TypeDef>();
    }

    inline auto typed_index<TypeDefOrRef>::TypeRef() const
    {
        return get_row<reader::TypeRef>();
    }

    inline auto typed_index<TypeDefOrRef>::TypeSpec() const
    {
        return get_row<reader::TypeSpec>();
    }

    inline auto typed_index<HasConstant>::Field() const
    {
        return get_row<reader::Field>();
    }

    inline auto typed_index<HasConstant>::Param() const
    {
        return get_row<reader::Param>();
    }

    inline auto typed_index<HasConstant>::Property() const
    {
        return get_row<reader::Property>();
    }

    inline auto typed_index<CustomAttributeType>::MemberRef() const
    {
        return get_row<reader::MemberRef>();
    }

    inline auto typed_index<MemberRefParent>::TypeRef() const
    {
        return get_row<reader::TypeRef>();
    }

    inline bool TypeDef::has_attribute(std::string_view const& type_namespace, std::string_view const& type_name) const
    {
        for (auto&& attribute : CustomAttribute())
        {
            auto type = attribute.Type().MemberRef().Class().TypeRef();

            if (type_name == type.TypeName() && type_namespace == type.TypeNamespace())
            {
                return true;
            }
        }

        return false;
    }
}
