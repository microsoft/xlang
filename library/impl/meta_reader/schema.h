
namespace xlang::meta::reader
{
    struct TypeRef : row_base
    {
        using row_base::row_base;

        auto ResolutionScope() const
        {
            return get_coded_index<reader::ResolutionScope>(0);
        }

        auto TypeName() const
        {
            return get_string(1);
        }

        auto TypeNamespace() const
        {
            return get_string(2);
        }
    };

    struct CustomAttribute : row_base
    {
        using row_base::row_base;

        auto Parent() const
        {
            return get_coded_index<HasCustomAttribute>(0);
        }

        auto Type() const
        {
            return get_coded_index<CustomAttributeType>(1);
        }

        auto Value() const
        {
            return get_blob(2);
        }
    };

    struct TypeDef : row_base
    {
        using row_base::row_base;

        auto Flags() const
        {
            return get_value<TypeAttributes>(0);
        }

        auto TypeName() const
        {
            return get_string(1);
        }

        auto TypeNamespace() const
        {
            return get_string(2);
        }

        auto Extends() const
        {
            return get_coded_index<TypeDefOrRef>(3);
        }

        auto CustomAttribute() const;
        auto InterfaceImpl() const;
        auto GenericParam() const;
    };

    struct MethodDef : row_base
    {
        using row_base::row_base;

        auto RVA() const
        {
            return get_value<uint32_t>(0);
        }

        auto ImplFlags() const
        {
            return get_value<MethodImplAttributes>(1);
        }

        auto Flags() const
        {
            return get_value<MethodAttributes>(2);
        }

        auto Name() const
        {
            return get_string(3);
        }

        auto Signature() const
        {
            return get_blob(4);
        }

        auto ParamList() const;
    };

    struct MemberRef : row_base
    {
        using row_base::row_base;

        auto Class() const
        {
            return get_coded_index<MemberRefParent>(0);
        }

        auto Name() const
        {
            return get_string(1);
        }

        MethodDefSig MethodSignature() const
        {
            auto cursor = get_blob(2);
            return{ get_table(), cursor };
        }
    };

    struct Module : row_base
    {
        using row_base::row_base;

        auto Name() const
        {
            return get_string(1);
        }
    };

    struct Field : row_base
    {
        using row_base::row_base;

        auto Flags() const
        {
            return get_value<FieldAttribute>(0);
        }

        auto Name() const
        {
            return get_string(1);
        }

        auto Signature() const
        {
            return get_blob(2);
        }
    };

    struct Param : row_base
    {
        using row_base::row_base;

        auto Flags() const
        {
            return get_value<ParamAttributes>(0);
        }

        auto Sequence() const
        {
            return get_value<uint16_t>(1);
        }

        auto Name() const
        {
            return get_string(2);
        }
    };

    struct InterfaceImpl : row_base
    {
        using row_base::row_base;

        auto Class() const;

        auto Interface() const
        {
            return get_coded_index<TypeDefOrRef>(1);
        }
    };

    struct Constant : row_base
    {
        using row_base::row_base;
    };

    struct FieldMarshal : row_base
    {
        using row_base::row_base;
    };

    struct TypeSpec : row_base
    {
        using row_base::row_base;

        TypeSpecSig Signature() const
        {
            auto cursor = get_blob(0);
            return{ get_table(), cursor };
        }
    };

    struct DeclSecurity : row_base
    {
        using row_base::row_base;
    };

    struct ClassLayout : row_base
    {
        using row_base::row_base;
    };

    struct FieldLayout : row_base
    {
        using row_base::row_base;
    };

    struct StandAloneSig : row_base
    {
        using row_base::row_base;
    };

    struct EventMap : row_base
    {
        using row_base::row_base;
    };

    struct Event : row_base
    {
        using row_base::row_base;
    };

    struct PropertyMap : row_base
    {
        using row_base::row_base;
    };

    struct Property : row_base
    {
        using row_base::row_base;
    };

    struct MethodSemantics : row_base
    {
        using row_base::row_base;
    };

    struct MethodImpl : row_base
    {
        using row_base::row_base;
    };

    struct ModuleRef : row_base
    {
        using row_base::row_base;
    };

    struct ImplMap : row_base
    {
        using row_base::row_base;
    };

    struct FieldRVA : row_base
    {
        using row_base::row_base;
    };

    struct Assembly : row_base
    {
        using row_base::row_base;
    };

    struct AssemblyProcessor : row_base
    {
        using row_base::row_base;
    };

    struct AssemblyOS : row_base
    {
        using row_base::row_base;
    };

    struct AssemblyRef : row_base
    {
        using row_base::row_base;
    };

    struct AssemblyRefProcessor : row_base
    {
        using row_base::row_base;
    };

    struct AssemblyRefOS : row_base
    {
        using row_base::row_base;
    };

    struct File : row_base
    {
        using row_base::row_base;
    };

    struct ExportedType : row_base
    {
        using row_base::row_base;
    };

    struct ManifestResource : row_base
    {
        using row_base::row_base;
    };

    struct NestedClass : row_base
    {
        using row_base::row_base;
    };

    struct GenericParam : row_base
    {
        using row_base::row_base;

        auto Number() const
        {
            return get_value<uint16_t>(0);
        }

        auto Flags() const
        {
            return get_value<ParamAttributes>(1);
        }

        auto Owner() const
        {
            return get_coded_index<TypeOrMethodDef>(2);
        }

        auto Name() const
        {
            return get_string(3);
        }
    };

    struct MethodSpec : row_base
    {
        using row_base::row_base;
    };

    struct GenericParamConstraint : row_base
    {
        using row_base::row_base;
    };

    inline bool operator<(coded_index<HasCustomAttribute> const& left, CustomAttribute const& right) noexcept
    {
        return left < right.Parent();
    }

    inline bool operator<(CustomAttribute const& left, coded_index<HasCustomAttribute> const& right) noexcept
    {
        return left.Parent() < right;
    }


    inline bool operator<(coded_index<TypeOrMethodDef> const& left, GenericParam const& right) noexcept
    {
        return left < right.Owner();
    }

    inline bool operator<(GenericParam const& left, coded_index<TypeOrMethodDef> const& right) noexcept
    {
        return left.Owner() < right;
    }
}
