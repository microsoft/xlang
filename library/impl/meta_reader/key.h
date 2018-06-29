
namespace xlang::meta::reader
{
    inline auto typed_index<TypeDefOrRef>::TypeDef() const
    {
        WINRT_ASSERT(type() == TypeDefOrRef::TypeDef);
        return get_database().TypeDef[index()];
    }

    inline auto typed_index<TypeDefOrRef>::TypeRef() const
    {
        WINRT_ASSERT(type() == TypeDefOrRef::TypeRef);
        return get_database().TypeRef[index()];
    }

    inline auto typed_index<TypeDefOrRef>::TypeSpec() const
    {
        WINRT_ASSERT(type() == TypeDefOrRef::TypeSpec);
        return get_database().TypeSpec[index()];
    }

    inline auto typed_index<HasConstant>::Field() const
    {
        WINRT_ASSERT(type() == HasConstant::Field);
        return get_database().Field[index()];
    }

    inline auto typed_index<HasConstant>::Param() const
    {
        WINRT_ASSERT(type() == HasConstant::Param);
        return get_database().Param[index()];
    }

    inline auto typed_index<HasConstant>::Property() const
    {
        WINRT_ASSERT(type() == HasConstant::Property);
        return get_database().Property[index()];
    }

    inline auto typed_index<CustomAttributeType>::MemberRef() const
    {
        WINRT_ASSERT(type() == CustomAttributeType::MemberRef);
        return get_database().MemberRef[index()];
    }

    inline auto typed_index<MemberRefParent>::TypeRef() const
    {
        WINRT_ASSERT(type() == MemberRefParent::TypeRef);
        return get_database().TypeRef[index()];
    }
}
