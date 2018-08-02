
namespace xlang::meta::reader
{
    template <typename T>
    struct coded_index_bits : std::integral_constant<uint32_t, 0> {};

    template <typename T>
    inline constexpr uint32_t coded_index_bits_v = coded_index_bits<T>::value;

    enum class TypeDefOrRef : uint32_t
    {
        TypeDef,
        TypeRef,
        TypeSpec,
    };
    template <>
    struct coded_index_bits<TypeDefOrRef> : std::integral_constant<uint32_t, 2> {};

    enum class HasConstant : uint32_t
    {
        Field,
        Param,
        Property,
    };
    template <>
    struct coded_index_bits<HasConstant> : std::integral_constant<uint32_t, 2> {};

    enum class HasCustomAttribute : uint32_t
    {
        MethodDef,
        Field,
        TypeRef,
        TypeDef,
        Param,
        InterfaceImpl,
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
    };
    template <>
    struct coded_index_bits<HasCustomAttribute> : std::integral_constant<uint32_t, 5> {};

    enum class HasFieldMarshal : uint32_t
    {
        Field,
        Param,
    };
    template <>
    struct coded_index_bits<HasFieldMarshal> : std::integral_constant<uint32_t, 1> {};

    enum class HasDeclSecurity : uint32_t
    {
        TypeDef,
        MethodDef,
        Assembly,
    };
    template <>
    struct coded_index_bits<HasDeclSecurity> : std::integral_constant<uint32_t, 2> {};

    enum class MemberRefParent : uint32_t
    {
        TypeDef,
        TypeRef,
        ModuleRef,
        MethodDef,
        TypeSpec,
    };
    template <>
    struct coded_index_bits<MemberRefParent> : std::integral_constant<uint32_t, 3> {};

    enum class HasSemantics : uint32_t
    {
        Event,
        Property,
    };
    template <>
    struct coded_index_bits<HasSemantics> : std::integral_constant<uint32_t, 1> {};

    enum class MethodDefOrRef : uint32_t
    {
        MethodDef,
        MemberRef,
    };
    template <>
    struct coded_index_bits<MethodDefOrRef> : std::integral_constant<uint32_t, 1> {};

    enum class MemberForwarded : uint32_t
    {
        Field,
        MethodDef,
    };
    template <>
    struct coded_index_bits<MemberForwarded> : std::integral_constant<uint32_t, 1> {};

    enum class Implementation : uint32_t
    {
        File,
        AssemblyRef,
        ExportedType,
    };
    template <>
    struct coded_index_bits<Implementation> : std::integral_constant<uint32_t, 2> {};

    enum class CustomAttributeType : uint32_t
    {
        MethodDef = 2,
        MemberRef,
    };
    template <>
    struct coded_index_bits<CustomAttributeType> : std::integral_constant<uint32_t, 3> {};

    enum class ResolutionScope : uint32_t
    {
        Module,
        ModuleRef,
        AssemblyRef,
        TypeRef,
    };
    template <>
    struct coded_index_bits<ResolutionScope> : std::integral_constant<uint32_t, 2> {};

    enum class TypeOrMethodDef : uint32_t
    {
        TypeDef,
        MethodDef,
    };
    template <>
    struct coded_index_bits<TypeOrMethodDef> : std::integral_constant<uint32_t, 1> {};

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

    enum class MethodImplAttributes : uint16_t
    {
        CodeTypeMask = 0x0003,
        IL = 0x0000,      // Method impl is CIL
        Native = 0x0001,  // Method impl is native
        OPTIL = 0x0002,   // Reserved: shall be zero in conforming implementations
        Runtime = 0x0003, // Method impl is provided by the runtime

        ManagedMask = 0x0004,
        Unmanaged = 0x0004,
        Managed = 0x0000,

        ForwardRef = 0x0010, // Method is defined; used primarily in merge scenarios
        PreserveSig = 0x0080, // Reserved
        InternalCall = 0x1000, // Reserved
        Synchronized = 0x0020, // Method is single threaded through the body
        NoInlining = 0x0008, // Method cannot be inlined
        MaxMethodImplVal = 0xffff, // Range check value
        NoOptimization = 0x0040, // Method will not be optimized when generatinv native code
    };

    enum class MethodAttributes : uint16_t
    {
        MemberAccessMask = 0x0007,
        CompilerControlled = 0x0000, // Member not referenceable
        Private = 0x0001,
        FamAndAssem = 0x0002,        // Accessible by subtypes only in this Assembly
        Assembly = 0x0003,           // Accessible by anyone in this Assembly
        Family = 0x0004,             // aka Protected
        FamOrAssem = 0x0005,         // Accessible by subtypes anywhere, plus anyone in this Assembly
        Public = 0x0006,

        Static = 0x0010,
        Final = 0x0020,
        Virtual = 0x0040,
        HideBySig = 0x0080,

        VtableLayoutMask = 0x0100,
        ReuseSlot = 0x0000, // Method reuses existing slot in a vtable
        NewSlot = 0x0100,   // Method always gets a new slot in the vtable

        Strict = 0x0200,
        Abstract = 0x0400,
        SpecialName = 0x0800,

        PInvokeImpl = 0x2000, // Implementation is forwarded through PInvoke
        UnmanagedExport = 0x0008, // Reerved: shall be zero for conforming implementations

        RTSpecialName = 0x1000,
        HasSecurity = 0x4000,
        RequireSecObject = 0x8000,
    };

    enum class ParamAttributes : uint16_t
    {
        In = 0x0001,
        Out = 0x0002,
        Optional = 0x0010,
        HasDefault = 0x1000, // Param has default value
        HasFieldMarshal = 0x2000,
        Unused = 0xcfe0, // Reserved
    };

    enum class EventAttributes : uint16_t
    {
        SpecialName = 0x0200,
        RTSpecialName = 0x0400,
    };

    enum class MethodSemanticsAttributes : uint16_t
    {
        Setter = 0x0001, // Setter for property
        Getter = 0x0002, // Getter for property
        Other = 0x0004, // Other method for property or event
        AddOn = 0x0008, // AddOn method for event
        RemoveOn = 0x0010, // RemoveOn method for event
        Fire = 0x0020, // Optional fire method for event
    };

    enum class PropertyAttributes : uint16_t
    {
        SpecialName = 0x0200,
        RTSpecialName = 0x0400,
        HasDefault = 0x1000,
        Unused = 0xe9ff,
    };

    enum class FieldAttributes : uint16_t
    {
        FieldAccessMask = 0x0007,
        CompilerControlled = 0x0000, // Member not referenceable
        Private = 0x0001,
        FamAndAssem = 0x0002,        // Accessible by subtypes only in this Assembly
        Assembly = 0x0003,           // Accessible by anyone in this Assembly
        Family = 0x0004,             // aka Protected
        FamOrAssem = 0x0005,         // Accessible by subtypes anywhere, plus anyone in this Assembly
        Public = 0x0006,

        Static = 0x0010,
        InitOnly = 0x0020,
        Literal = 0x0040,
        NotSerialized = 0x0080,
        SpecialName = 0x0200,

        PInvokeImpl = 0x2000,

        RTSpecialName = 0x0400,
        HasFieldMarshall = 0x1000,
        HasDefault = 0x8000,
        HasFieldRVA = 0x0100,
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

    enum class ConstantType : uint16_t
    {
        Boolean = 0x02,
        Char = 0x03,
        Int8 = 0x04,
        UInt8 = 0x05,
        Int16 = 0x06,
        UInt16 = 0x07,
        Int32 = 0x08,
        UInt32 = 0x09,
        Int64 = 0x0a,
        UInt64 = 0x0b,
        Float32 = 0x0c,
        Float64 = 0x0d,
        String = 0x0e,
        Class = 0x12
    };

    enum class ElementType : uint8_t
    {
        End = 0x00, // Sentinel value

        Void = 0x01,
        Boolean = 0x02,
        Char = 0x03,
        I1 = 0x04,
        U1 = 0x05,
        I2 = 0x06,
        U2 = 0x07,
        I4 = 0x08,
        U4 = 0x09,
        I8 = 0x0a,
        U8 = 0x0b,
        R4 = 0x0c,
        R8 = 0x0d,
        String = 0x0e,

        Ptr = 0x0f, // Followed by TypeSig
        ByRef = 0x10, // Followed by TypeSig
        ValueType = 0x11, // Followed by TypeDef or TypeRef
        Class = 0x12, // Followed by TypeDef or TypeRef
        Var = 0x13, // Generic parameter in a type definition, represented as unsigned integer
        Array = 0x14,
        GenericInst = 0x15,
        TypedByRef = 0x16,

        I = 0x18, // System.IntPtr
        U = 0x19, // System.UIntPtr

        FnPtr = 0x1b, // Followed by full method signature
        Object = 0x1c, // System.Object
        SZArray = 0x1d,
        MVar = 0x1e, // Generic parameter in a method definition, represented as unsigned integer
        CModReqd = 0x1f, // Required modifier, followed by a TypeDef or TypeRef
        CModOpt = 0x20, // Optional modifier, followed by a TypeDef or TypeRef
        Internal = 0x21,

        Modifier = 0x40, // Or'd with folowing element types
        Sentinel = 0x41, // Sentinel for vararg method signature

        Pinned = 0x45,

        Type = 0x50, // System.Type
        TaggedObject = 0x51, // Boxed object (in custom attributes)
        Field = 0x53, // Custom attribute field
        Property = 0x54, // Custom attribute property
        Enum = 0x55, // Custom attribute enum
    };

    enum class CallingConvention : uint8_t
    {
        Default = 0x00,
        VarArg = 0x05,
        Field = 0x06,
        LocalSig = 0x07,
        Property = 0x08,
        GenericInst = 0x10,
        Mask = 0x0f,

        HasThis = 0x20,
        ExplicitThis = 0x40,
        Generic = 0x10,
    };

    enum class AssemblyHashAlgorithm : uint32_t
    {
        None = 0x0000,
        Reserved_MD5 = 0x8003,
        SHA1 = 0x8004,
    };

    enum class AssemblyFlags : uint32_t
    {
        PublicKey = 0x0001, // The assembly reference holds the full (unhashed) public key
        Retargetable = 0x01000,
        DisableJITcompileOptimizer = 0x4000,
        EnableJITcompileTracking = 0x8000,
    };

    template <typename T>
    constexpr inline T enum_mask(T value, T mask) noexcept
    {
        static_assert(std::is_enum_v<T>);
        using val = std::underlying_type_t<T>;
        return static_cast<T>(static_cast<val>(value) & static_cast<val>(mask));
    }
}
