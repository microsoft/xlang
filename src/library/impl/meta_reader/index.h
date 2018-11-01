
namespace xlang::meta::reader
{
    template <> struct typed_index<HasConstant> : index_base<HasConstant>
    {
        using index_base<HasConstant>::index_base;

        auto Field() const;
        auto Param() const;
        auto Property() const;
    };

    template <> struct typed_index<TypeDefOrRef> : index_base<TypeDefOrRef>
    {
        using index_base<TypeDefOrRef>::index_base;

        auto TypeDef() const;
        auto TypeRef() const;
        auto TypeSpec() const;
        auto CustomAttribute() const;
    };

    template <> struct typed_index<CustomAttributeType> : index_base<CustomAttributeType>
    {
        using index_base<CustomAttributeType>::index_base;

        auto MemberRef() const;
        auto MethodDef() const;
    };

    template <> struct typed_index<HasSemantics> : index_base<HasSemantics>
    {
        using index_base<HasSemantics>::index_base;

        auto Property() const;
        auto Event() const;
    };
}
