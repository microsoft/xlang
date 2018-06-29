
namespace xlang::meta::reader
{
    enum class TypeDefOrRef : uint32_t
    {
        TypeDef,
        TypeRef,
        TypeSpec,

        coded_index_bits = 2
    };

    enum class HasConstant : uint32_t
    {
        Field,
        Param,
        Property,

        coded_index_bits = 2
    };

    enum class HasCustomAttribute : uint32_t
    {
        MethodDef,
        Field,
        TypeRef,
        TypeDef,
        Param,
        m_InterfaceImpl,
        MemberRef,
        Module,
        Permission,
        Property,
        Event,
        StandAloneSig,
        ModuleRef,
        TypeSpec,
        Assembly,
        AssemblyRef,
        File,
        ExportedType,
        ManifestResource,
        GenericParam,
        GenericParamConstraint,
        MethodSpec,

        coded_index_bits = 5
    };

    enum class HasFieldMarshal : uint32_t
    {
        Field,
        Param,

        coded_index_bits = 1
    };

    enum class HasDeclSecurity : uint32_t
    {
        TypeDef,
        MethodDef,
        Assembly,

        coded_index_bits = 2
    };

    enum class MemberRefParent : uint32_t
    {
        TypeDef,
        TypeRef,
        ModuleRef,
        MethodDef,
        TypeSpec,

        coded_index_bits = 3
    };

    enum class HasSemantics : uint32_t
    {
        Event,
        Property,

        coded_index_bits = 1
    };

    enum class MethodDefOrRef : uint32_t
    {
        MethodDef,
        MemberRef,

        coded_index_bits = 1
    };

    enum class MemberForwarded : uint32_t
    {
        Field,
        MethodDef,

        coded_index_bits = 1
    };

    enum class Implementation : uint32_t
    {
        File,
        AssemblyRef,
        ExportedType,

        coded_index_bits = 2
    };

    enum class CustomAttributeType : uint32_t
    {
        MethodDef = 2,
        MemberRef,

        coded_index_bits = 3
    };

    enum class ResolutionScope : uint32_t
    {
        Module,
        ModuleRef,
        AssemblyRef,
        TypeRef,

        coded_index_bits = 2
    };

    enum class TypeOrMethodDef : uint32_t
    {
        TypeDef,
        MethodDef,

        coded_index_bits = 1
    };

    enum class TypeAttributes : uint32_t
    {
        // Visibility
        VisibilityMask = 0x00000007,
        NotPublic = 0x00000000,
        Public = 0x00000001,
        NestedPublic = 0x00000002,
        NestedPrivate = 0x00000003,
        NestedFamily = 0x00000004,
        NestedAssembly = 0x00000005,
        NestedFamANDAssem = 0x00000006,
        NestedFamORAssem = 0x00000007,

        // Class layout
        LayoutMask = 0x00000018,
        AutoLayout = 0x00000000,
        SequentialLayout = 0x00000008,
        ExplicitLayout = 0x00000010,

        // Class semantics
        ClassSemanticsMask = 0x00000020,
        Class = 0x00000000,
        Interface = 0x00000020,

        // Special semantics
        Abstract = 0x00000080,
        Sealed = 0x00000100,
        SpecialName = 0x00000400,

        // Implementation
        Import = 0x00001000,
        Serializable = 0x00002000,
        WindowsRuntime = 0x00004000,

        // String formatting
        StringFormatMask = 0x00030000,
        AnsiClass = 0x00000000,
        UnicodeClass = 0x00010000,
        AutoClass = 0x00020000,
        CustomFormatClass = 0x00030000,
        CustomFormatMask = 0x00C00000,

        // Additional
        BeforeFieldInit = 0x00100000,
        RTSpecialName = 0x00000800,
        HasSecurity = 0x00040000,
        IsTypeForwarder = 0x00200000,
        ReservedMask = 0x00040800,
    };

    enum class MethodImplAttributes : uint32_t
    {

    };

    enum class MethodAttributes : uint32_t
    {

    };

    enum class ParamAttributes : uint32_t
    {

    };

    enum class FieldAttribute : uint32_t
    {

    };

    enum class GenericParamAttributes : uint16_t
    {
        VarianceMask = 0x0003,
        None = 0x0000,
        Covariant = 0x0001,
        Contravariant = 0x0002,

        SpecialConstraintMask = 0x001c,
        ReferenceTypeConstraint = 0x0004,
        NotNullableValueTypeConstraint = 0x0008,
        DefaultConstructorConstraint = 0x0010
    };
}
