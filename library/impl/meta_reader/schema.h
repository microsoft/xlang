
namespace xlang::meta::reader
{
    struct TypeRef : row_base<TypeRef>
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

    struct CustomAttribute : row_base<CustomAttribute>
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

        auto Value() const;
    };

    struct TypeDef : row_base<TypeDef>
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

        auto FieldList() const;
        auto MethodList() const;

        auto CustomAttribute() const;
        auto InterfaceImpl() const;
        auto GenericParam() const;
        auto PropertyList() const;

        bool has_attribute(std::string_view const& type_namespace, std::string_view const& type_name) const;
        bool is_enum() const;
        auto get_enum_definition() const;
    };

    struct MethodDef : row_base<MethodDef>
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

        MethodDefSig Signature() const
        {
            auto cursor = get_blob(4);
            return{ get_table(), cursor };
        }

        auto ParamList() const;
    };

    struct MemberRef : row_base<MemberRef>
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

    struct Module : row_base<Module>
    {
        using row_base::row_base;

        auto Name() const
        {
            return get_string(1);
        }
    };

    struct Field : row_base<Field>
    {
        using row_base::row_base;

        auto Flags() const
        {
            return get_value<FieldAttributes>(0);
        }

        auto Name() const
        {
            return get_string(1);
        }

        auto Signature() const
        {
            auto cursor = get_blob(2);
            return FieldSig{ get_table(), cursor };
        }

        auto Constant() const;
        bool is_static() const
        {
            return enum_mask(Flags(), FieldAttributes::Static) == FieldAttributes::Static;
        }

        bool is_literal() const
        {
            return enum_mask(Flags(), FieldAttributes::Literal) == FieldAttributes::Literal;
        }
    };

    struct Param : row_base<Param>
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

    struct InterfaceImpl : row_base<InterfaceImpl>
    {
        using row_base::row_base;

        auto Class() const;

        auto Interface() const
        {
            return get_coded_index<TypeDefOrRef>(1);
        }
    };

    struct Constant : row_base<Constant>
    {
        using row_base::row_base;

        using value_type = std::variant<bool, char16_t, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t, float, double, std::string_view, std::nullptr_t>;

        auto Type() const
        {
            return get_value<ConstantType>(0);
        }

        auto Parent() const
        {
            return get_coded_index<HasConstant>(1);
        }

        auto ValueBoolean() const;
        auto ValueChar() const;
        auto ValueInt8() const;
        auto ValueUInt8() const;
        auto ValueInt16() const;
        auto ValueUInt16() const;
        auto ValueInt32() const;
        auto ValueUInt32() const;
        auto ValueInt64() const;
        auto ValueUInt64() const;
        auto ValueFloat32() const;
        auto ValueFloat64() const;
        auto ValueString() const;
        auto ValueClass() const;

        value_type Value() const;
    };

    struct FieldMarshal : row_base<FieldMarshal>
    {
        using row_base::row_base;
    };

    struct TypeSpec : row_base<TypeSpec>
    {
        using row_base::row_base;

        TypeSpecSig Signature() const
        {
            auto cursor = get_blob(0);
            return{ get_table(), cursor };
        }
    };

    struct DeclSecurity : row_base<DeclSecurity>
    {
        using row_base::row_base;
    };

    struct ClassLayout : row_base<ClassLayout>
    {
        using row_base::row_base;
    };

    struct FieldLayout : row_base<FieldLayout>
    {
        using row_base::row_base;
    };

    struct StandAloneSig : row_base<StandAloneSig>
    {
        using row_base::row_base;
    };

    struct EventMap : row_base<EventMap>
    {
        using row_base::row_base;
    };

    struct Event : row_base<Event>
    {
        using row_base::row_base;

        auto Name() const
        {
            return get_string(1);
        }
    };

    struct PropertyMap : row_base<PropertyMap>
    {
        using row_base::row_base;

        auto Parent() const;
        auto PropertyList() const;
    };

    struct Property : row_base<Property>
    {
        using row_base::row_base;

        auto Flags() const
        {
            return get_value<PropertyAttributes>(0);
        }

        auto Name() const
        {
            return get_string(1);
        }

        PropertySig Type() const
        {
            auto cursor = get_blob(2);
            return{ get_table(), cursor };
        }

        auto MethodSemantic() const;
        auto Parent() const;
    };

    struct MethodSemantics : row_base<MethodSemantics>
    {
        using row_base::row_base;

        auto Semantic() const
        {
            return get_value<MethodSemanticsAttributes>(0);
        }

        auto Method() const;

        auto Association() const
        {
            return get_coded_index<HasSemantics>(2);
        }
    };

    struct MethodImpl : row_base<MethodImpl>
    {
        using row_base::row_base;
    };

    struct ModuleRef : row_base<ModuleRef>
    {
        using row_base::row_base;
    };

    struct ImplMap : row_base<ImplMap>
    {
        using row_base::row_base;
    };

    struct FieldRVA : row_base<FieldRVA>
    {
        using row_base::row_base;
    };

    struct Assembly : row_base<Assembly>
    {
        using row_base::row_base;

        auto Name() const
        {
            return get_string(4);
        }
    };

    struct AssemblyProcessor : row_base<AssemblyProcessor>
    {
        using row_base::row_base;
    };

    struct AssemblyOS : row_base<AssemblyOS>
    {
        using row_base::row_base;
    };

    struct AssemblyRef : row_base<AssemblyRef>
    {
        using row_base::row_base;

        auto Name() const
        {
            return get_string(3);
        }
    };

    struct AssemblyRefProcessor : row_base<AssemblyRefProcessor>
    {
        using row_base::row_base;
    };

    struct AssemblyRefOS : row_base<AssemblyRefOS>
    {
        using row_base::row_base;
    };

    struct File : row_base<File>
    {
        using row_base::row_base;
    };

    struct ExportedType : row_base<ExportedType>
    {
        using row_base::row_base;
    };

    struct ManifestResource : row_base<ManifestResource>
    {
        using row_base::row_base;
    };

    struct NestedClass : row_base<NestedClass>
    {
        using row_base::row_base;
    };

    struct GenericParam : row_base<GenericParam>
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

    struct MethodSpec : row_base<MethodSpec>
    {
        using row_base::row_base;
    };

    struct GenericParamConstraint : row_base<GenericParamConstraint>
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

    inline bool operator<(coded_index<HasConstant> const& left, Constant const& right) noexcept
    {
        return left < right.Parent();
    }

    inline bool operator<(Constant const& left, coded_index<HasConstant> const& right) noexcept
    {
        return left.Parent() < right;
    }

    inline bool operator<(coded_index<HasSemantics> const& left, MethodSemantics const& right) noexcept
    {
        return left < right.Association();
    }

    inline bool operator<(MethodSemantics const& left, coded_index<HasSemantics> const& right) noexcept
    {
        return left.Association() < right;
    }
}
