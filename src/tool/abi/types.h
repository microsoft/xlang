#pragma once

#include "meta_reader.h"
#include "sha1.h"
#include "type_names.h"

struct writer;

struct metadata_type
{
    virtual std::string_view clr_full_name() const = 0;
    virtual std::string_view clr_abi_namespace() const = 0;
    virtual std::string_view clr_logical_namespace() const = 0;

    virtual std::string_view cpp_abi_name() const = 0;
    virtual std::string_view cpp_logical_name() const = 0;

    virtual std::string_view mangled_name() const = 0;
    virtual std::string_view generic_param_mangled_name() const = 0;

    virtual void append_signature(sha1& hash) const = 0;

    virtual std::size_t push_contract_guards(writer& w) const = 0;

    virtual void write_cpp_forward_declaration(writer& w) const = 0;
    virtual void write_cpp_generic_param_logical_type(writer& w) const = 0;
    virtual void write_cpp_generic_param_abi_type(writer& w) const = 0;
    virtual void write_cpp_abi_name(writer& w) const = 0;
    virtual void write_cpp_abi_param(writer& w) const = 0;

    virtual void write_c_forward_declaration(writer& w) const = 0;
    virtual void write_c_abi_param(writer& w) const = 0;
};

inline bool operator<(metadata_type const& lhs, metadata_type const& rhs) noexcept
{
    return lhs.clr_full_name() < rhs.clr_full_name();
}

struct element_type final : metadata_type
{
    element_type(
        std::string_view clrName,
        std::string_view logicalName,
        std::string_view abiName,
        std::string_view cppName,
        std::string_view mangledName,
        std::string_view signature) :
        m_clrName(clrName),
        m_logicalName(logicalName),
        m_abiName(abiName),
        m_cppName(cppName),
        m_mangledName(mangledName),
        m_signature(signature)
    {
    }

    static element_type const& from_type(xlang::meta::reader::ElementType type);

    virtual std::string_view clr_abi_namespace() const override
    {
        return {};
    }

    virtual std::string_view clr_logical_namespace() const override
    {
        return {};
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrName;
    }

    virtual std::string_view cpp_abi_name() const override
    {
        return m_abiName;
    }

    virtual std::string_view cpp_logical_name() const override
    {
        return m_logicalName;
    }

    virtual std::string_view mangled_name() const override
    {
        xlang::throw_invalid("ElementType values don't have mangled names");
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        return m_mangledName;
    }

    virtual void append_signature(sha1& hash) const override
    {
        hash.append(m_signature);
    }

    virtual std::size_t push_contract_guards(writer&) const override
    {
        // No contract guards necessary
        return 0;
    }

    virtual void write_cpp_forward_declaration(writer&) const override
    {
        // No forward declaration necessary
    }

    virtual void write_cpp_generic_param_logical_type(writer& w) const override;
    virtual void write_cpp_generic_param_abi_type(writer& w) const override;
    virtual void write_cpp_abi_name(writer& w) const override;

    virtual void write_cpp_abi_param(writer& w) const override
    {
        // For element types, param name == ABI name
        write_cpp_abi_name(w);
    }

    virtual void write_c_forward_declaration(writer&) const override
    {
        // No forward declaration necessary
    }

    virtual void write_c_abi_param(writer& w) const override;

private:

    std::string_view m_clrName;
    std::string_view m_logicalName;
    std::string_view m_abiName;
    std::string_view m_cppName;
    std::string_view m_mangledName;
    std::string_view m_signature;
};

struct system_type final : metadata_type
{
    system_type(std::string_view clrName, std::string_view cppName, std::string_view signature) :
        m_clrName(clrName),
        m_cppName(cppName),
        m_signature(signature)
    {
    }

    static system_type const& from_name(std::string_view typeName);

    virtual std::string_view clr_abi_namespace() const override
    {
        // Currently all mapped types from the System namespace have no namespace
        return {};
    }

    virtual std::string_view clr_logical_namespace() const override
    {
        // Currently all mapped types from the System namespace have no namespace
        return {};
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrName;
    }

    virtual std::string_view cpp_abi_name() const override
    {
        // Currently all mapped types from the System namespace do not have differing ABI/logical names
        return m_cppName;
    }

    virtual std::string_view cpp_logical_name() const override
    {
        return m_cppName;
    }

    virtual std::string_view mangled_name() const override
    {
        xlang::throw_invalid("ElementType values don't have mangled names");
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        // Currently all mangled names from the System namespace match their C++ name (i.e. no '_' etc. in the name)
        return m_cppName;
    }

    virtual void append_signature(sha1& hash) const override
    {
        hash.append(m_signature);
    }

    virtual std::size_t push_contract_guards(writer&) const override
    {
        // No contract guards necessary
        return 0;
    }

    virtual void write_cpp_forward_declaration(writer&) const override
    {
        // No forward declaration necessary
    }

    virtual void write_cpp_generic_param_logical_type(writer& w) const override;
    virtual void write_cpp_generic_param_abi_type(writer& w) const override;
    virtual void write_cpp_abi_name(writer& w) const override;

    virtual void write_cpp_abi_param(writer& w) const override
    {
        // For element types, param name == ABI name
        write_cpp_abi_name(w);
    }

    virtual void write_c_forward_declaration(writer&) const override
    {
        // No forward declaration necessary
    }

    virtual void write_c_abi_param(writer& w) const override;

private:

    std::string_view m_clrName;
    std::string_view m_cppName;
    std::string_view m_signature;
};

struct mapped_type final : metadata_type
{
    mapped_type(
        xlang::meta::reader::TypeDef const& type,
        std::string_view cppName,
        std::string_view mangledName,
        std::string_view signature) :
        m_type(type),
        m_clrFullName(::clr_full_name(type)),
        m_cppName(cppName),
        m_mangledName(mangledName),
        m_signature(signature)
    {
    }

    static mapped_type const* from_typedef(xlang::meta::reader::TypeDef const& type);

    virtual std::string_view clr_abi_namespace() const override
    {
        return m_type.TypeNamespace();
    }

    virtual std::string_view clr_logical_namespace() const override
    {
        // Currently all mapped types are in the global namespace
        return {};
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrFullName;
    }

    virtual std::string_view cpp_abi_name() const override
    {
        // Currently no mapped types have differing ABI/logical names
        return m_cppName;
    }

    virtual std::string_view cpp_logical_name() const override
    {
        return m_cppName;
    }

    virtual std::string_view mangled_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        // Currently no mapped type names have any characters that would make it so that the mangled name would be
        // different than the generic param mangled name (i.e. no underscore, etc.)
        return m_mangledName;
    }

    virtual void append_signature(sha1& hash) const override
    {
        hash.append(m_signature);
    }

    virtual std::size_t push_contract_guards(writer&) const override
    {
        // No contract guards necessary
        return 0;
    }

    virtual void write_cpp_forward_declaration(writer&) const override
    {
        // No forward declaration necessary
    }

    virtual void write_cpp_generic_param_logical_type(writer& w) const override;
    virtual void write_cpp_generic_param_abi_type(writer& w) const override;
    virtual void write_cpp_abi_name(writer& w) const override;
    virtual void write_cpp_abi_param(writer& w) const override;

    virtual void write_c_forward_declaration(writer&) const override
    {
        // No forward declaration necessary
    }

    virtual void write_c_abi_param(writer& w) const override;

private:

    xlang::meta::reader::TypeDef m_type;
    std::string m_clrFullName;
    std::string_view m_cppName;
    std::string_view m_mangledName;
    std::string_view m_signature;
};

struct typedef_base : metadata_type
{
    typedef_base(xlang::meta::reader::TypeDef const& type) :
        m_type(type),
        m_clrFullName(::clr_full_name(type)),
        m_mangledName(::mangled_name<false>(type)),
        m_genericParamMangledName(::mangled_name<true>(type))
    {
    }

    virtual std::string_view clr_abi_namespace() const override
    {
        // Most types don't have a different abi and logical type, so use the logical namespace as an easy default
        return m_type.TypeNamespace();
    }

    virtual std::string_view clr_logical_namespace() const override
    {
        return m_type.TypeNamespace();
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrFullName;
    }

    virtual std::string_view cpp_abi_name() const override
    {
        // Most types just use the CLR type name, so use that as the easy default
        return m_type.TypeName();
    }

    virtual std::string_view cpp_logical_name() const override
    {
        // Most types just use the CLR type name, so use that as the easy default
        return m_type.TypeName();
    }

    virtual std::string_view mangled_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        // Only generic instantiations should be used as generic params
        XLANG_ASSERT(!is_generic());
        return m_genericParamMangledName;
    }

    virtual std::size_t push_contract_guards(writer& w) const override;

    virtual void write_cpp_generic_param_logical_type(writer& w) const override
    {
        // For most types, logical type == abi type, so provide that as the easy default
        write_cpp_generic_param_abi_type(w);
    }

    virtual void write_cpp_abi_name(writer& w) const override;

    xlang::meta::reader::TypeDef const& type() const noexcept
    {
        return m_type;
    }

    bool is_generic() const noexcept
    {
        return ::is_generic(m_type);
    }

    auto is_deprecated() const noexcept
    {
        return ::is_deprecated(m_type);
    }

    xlang::meta::reader::category category() const noexcept
    {
        return get_category(m_type);
    }

protected:

    xlang::meta::reader::TypeDef m_type;

    // These strings are initialized by the base class
    std::string m_clrFullName;
    std::string m_mangledName;
    std::string m_genericParamMangledName;
};

struct enum_type final : typedef_base
{
    enum_type(xlang::meta::reader::TypeDef const& type) :
        typedef_base(type)
    {
    }

    virtual void append_signature(sha1& hash) const override
    {
        using namespace std::literals;
        hash.append("enum("sv);
        hash.append(m_clrFullName);
        hash.append(";"sv);
        element_type::from_type(underlying_type()).append_signature(hash);
        hash.append(")"sv);
    }

    virtual void write_cpp_forward_declaration(writer& w) const override;
    virtual void write_cpp_generic_param_abi_type(writer& w) const override;
    virtual void write_cpp_abi_param(writer& w) const override;

    virtual void write_c_forward_declaration(writer& w) const override;
    virtual void write_c_abi_param(writer& w) const override;

    void write_cpp_definition(writer& w) const;
    void write_c_definition(writer& w) const;

    xlang::meta::reader::ElementType underlying_type() const
    {
        return underlying_enum_type(m_type);
    }
};

struct struct_member
{
    xlang::meta::reader::Field field;
    metadata_type const* type;
};

struct struct_type final : typedef_base
{
    struct_type(xlang::meta::reader::TypeDef const& type) :
        typedef_base(type)
    {
    }

    virtual void append_signature(sha1& hash) const override
    {
        using namespace std::literals;
        XLANG_ASSERT(members.size() == static_cast<std::size_t>(distance(m_type.FieldList())));
        hash.append("struct("sv);
        hash.append(m_clrFullName);
        for (auto const& member : members)
        {
            hash.append(";");
            member.type->append_signature(hash);
        }
        hash.append(")"sv);
    }

    virtual void write_cpp_forward_declaration(writer& w) const override;
    virtual void write_cpp_generic_param_abi_type(writer& w) const override;
    virtual void write_cpp_abi_param(writer& w) const override;

    virtual void write_c_forward_declaration(writer& w) const override;
    virtual void write_c_abi_param(writer& w) const override;

    void write_cpp_definition(writer& w) const;
    void write_c_definition(writer& w) const;

    std::vector<struct_member> members;
};

struct function_return_type
{
    xlang::meta::reader::RetTypeSig signature;
    std::string_view name;
    metadata_type const* type;
};

struct function_param
{
    xlang::meta::reader::ParamSig signature;
    std::string_view name;
    metadata_type const* type;
};

struct function_def
{
    xlang::meta::reader::MethodDef def;
    std::optional<function_return_type> return_type;
    std::vector<function_param> params;
};

struct delegate_type final : typedef_base
{
    delegate_type(xlang::meta::reader::TypeDef const& type) :
        typedef_base(type)
    {
        m_abiName.reserve(1 + type.TypeName().length());
        details::append_type_prefix(m_abiName, type);
        m_abiName += type.TypeName();
    }

    virtual std::string_view cpp_abi_name() const override
    {
        return m_abiName;
    }

    virtual std::string_view cpp_logical_name() const override
    {
        // Even though the ABI name of delegates is different than their CLR name, the logical name is still the same as
        // the ABI name
        return m_abiName;
    }

    virtual void append_signature(sha1& hash) const override
    {
        using namespace std::literals;
        hash.append("delegate({"sv);
        auto iid = type_iid(m_type);
        hash.append(std::string_view{ iid.data(), iid.size() - 1 });
        hash.append("})"sv);
    }

    virtual void write_cpp_forward_declaration(writer& w) const override;
    virtual void write_cpp_generic_param_abi_type(writer& w) const override;
    virtual void write_cpp_abi_param(writer& w) const override;

    virtual void write_c_forward_declaration(writer& w) const override;
    virtual void write_c_abi_param(writer& w) const override;

    void write_cpp_definition(writer& w) const;
    void write_c_definition(writer& w) const;

    std::vector<function_def> functions;

private:

    std::string m_abiName;
};

struct interface_type final : typedef_base
{
    interface_type(xlang::meta::reader::TypeDef const& type) :
        typedef_base(type)
    {
    }

    virtual void append_signature(sha1& hash) const override
    {
        using namespace std::literals;
        hash.append("{"sv);
        auto iid = type_iid(m_type);
        hash.append(std::string_view{ iid.data(), iid.size() - 1 });
        hash.append("}"sv);
    }

    virtual void write_cpp_forward_declaration(writer& w) const override;
    virtual void write_cpp_generic_param_abi_type(writer& w) const override;
    virtual void write_cpp_abi_param(writer& w) const override;

    virtual void write_c_forward_declaration(writer& w) const override;
    virtual void write_c_abi_param(writer& w) const override;

    void write_cpp_definition(writer& w) const;
    void write_c_definition(writer& w) const;

    std::vector<metadata_type const*> required_interfaces;
    std::vector<function_def> functions;
};

struct fastabi_type;

struct class_type final : typedef_base
{
    class_type(xlang::meta::reader::TypeDef const& type) :
        typedef_base(type)
    {
        using namespace xlang::meta::reader;
        if (auto defaultIface = try_get_default_interface(type))
        {
            ::visit(defaultIface, xlang::visit_overload{
                [&](GenericTypeInstSig const& t)
                {
                    switch (t.GenericType().type())
                    {
                    case TypeDefOrRef::TypeDef:
                        m_abiNamespace = t.GenericType().TypeDef().TypeNamespace();
                        break;

                    case TypeDefOrRef::TypeRef:
                        m_abiNamespace = t.GenericType().TypeRef().TypeNamespace();
                        break;

                    default:
                        XLANG_ASSERT(false);
                    }
                },
                [&](auto const& defOrRef)
                {
                    m_abiNamespace = defOrRef.TypeNamespace();
                }});
        }
    }

    virtual std::string_view clr_abi_namespace() const override
    {
        if (m_abiNamespace.empty())
        {
            xlang::throw_invalid("Class type '", clr_full_name(), "' does not have a default interface and therefore "
                "does not have an ABI type namespace");
        }

        return m_abiNamespace;
    }

    virtual std::string_view cpp_abi_name() const override
    {
        if (!default_interface)
        {
            xlang::throw_invalid("Class type '", clr_full_name(), "' does not have a default interface and therefore "
                "does not have an ABI type name");
        }

        return default_interface->cpp_abi_name();
    }

    virtual void append_signature(sha1& hash) const override
    {
        using namespace std::literals;
        if (!default_interface)
        {
            xlang::throw_invalid("Class type '", clr_full_name(), "' does not have a default interface and therefore "
                "does not have a signature");
        }

        hash.append("rc("sv);
        hash.append(m_clrFullName);
        hash.append(";"sv);
        default_interface->append_signature(hash);
        hash.append(")"sv);
    }

    virtual void write_cpp_forward_declaration(writer& w) const override;
    virtual void write_cpp_generic_param_logical_type(writer& w) const override;
    virtual void write_cpp_generic_param_abi_type(writer& w) const override;
    virtual void write_cpp_abi_param(writer& w) const override;

    virtual void write_c_forward_declaration(writer& w) const override;
    virtual void write_c_abi_param(writer& w) const override;

    void write_cpp_definition(writer& w) const;
    void write_c_definition(writer& w) const;

    std::vector<metadata_type const*> required_interfaces;
    metadata_type const* default_interface = nullptr;
    std::vector<fastabi_type> fastabi_interfaces;

private:

    std::string_view m_abiNamespace;
};

struct generic_inst final : metadata_type
{
    generic_inst(typedef_base const* genericType, std::vector<metadata_type const*> genericParams) :
        m_genericType(genericType),
        m_genericParams(std::move(genericParams))
    {
        m_clrFullName = genericType->clr_full_name();
        m_clrFullName.push_back('<');

        m_mangledName = genericType->mangled_name();

        std::string_view prefix;
        for (auto param : m_genericParams)
        {
            m_clrFullName += prefix;
            m_clrFullName += param->clr_full_name();

            m_mangledName.push_back('_');
            m_mangledName += param->generic_param_mangled_name();

            prefix = ", ";
        }

        m_clrFullName.push_back('>');
    }

    virtual std::string_view clr_abi_namespace() const override
    {
        return m_genericType->clr_abi_namespace();
    }

    virtual std::string_view clr_logical_namespace() const override
    {
        return m_genericType->clr_logical_namespace();
    }

    virtual std::string_view clr_full_name() const override
    {
        return m_clrFullName;
    }

    virtual std::string_view cpp_abi_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view cpp_logical_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view mangled_name() const override
    {
        return m_mangledName;
    }

    virtual std::string_view generic_param_mangled_name() const override
    {
        return m_mangledName;
    }

    virtual void append_signature(sha1& hash) const override
    {
        using namespace std::literals;
        hash.append("pinterface({"sv);
        auto iid = type_iid(m_genericType->type());
        hash.append(std::string_view{ iid.data(), iid.size() - 1 });
        hash.append("}"sv);
        for (auto param : m_genericParams)
        {
            hash.append(";"sv);
            param->append_signature(hash);
        }
        hash.append(")"sv);
    }

    virtual std::size_t push_contract_guards(writer& w) const override;

    virtual void write_cpp_forward_declaration(writer& w) const override;
    virtual void write_cpp_generic_param_logical_type(writer& w) const override;
    virtual void write_cpp_generic_param_abi_type(writer& w) const override;
    virtual void write_cpp_abi_name(writer& w) const override;
    virtual void write_cpp_abi_param(writer& w) const override;

    virtual void write_c_forward_declaration(writer& w) const override;
    virtual void write_c_abi_param(writer& w) const override;

    typedef_base const* generic_type() const noexcept
    {
        return m_genericType;
    }

    xlang::meta::reader::category category() const noexcept
    {
        return m_genericType->category();
    }

    auto is_deprecated() const noexcept
    {
        return m_genericType->is_deprecated();
    }

    std::string_view generic_type_abi_name() const noexcept
    {
        // Generic type CLR names end with "`N" where 'N' is the number of generic parameters
        auto result = m_genericType->cpp_abi_name();
        auto tickPos = result.rfind('`');
        XLANG_ASSERT(tickPos != std::string_view::npos);
        return result.substr(0, tickPos);
    }

    std::vector<metadata_type const*> const& generic_params() const noexcept
    {
        return m_genericParams;
    }

    std::vector<generic_inst const*> dependencies;
    std::vector<function_def> functions;

private:

    typedef_base const* m_genericType;
    std::vector<metadata_type const*> m_genericParams;
    std::string m_clrFullName;
    std::string m_mangledName;
};

struct fastabi_type : metadata_type
{
    fastabi_type(metadata_type const& base, interface_type const& type) :
        m_base(&base),
        m_type(&type)
    {
        // Synthesize an interface name
        m_typeName = "fastabi_";
        m_typeName += m_type->cpp_abi_name();

        m_mangledName = mangled_namespace<false>(m_type->clr_abi_namespace()) + "_C";
        details::append_mangled_name<false>(m_mangledName, m_typeName);
    }

    [[noreturn]]
    virtual std::string_view clr_full_name() const override
    {
        // Fast ABI types are generated from metadata/attributes; they don't actually exist in metadata and therefore
        // have no CLR name, nor is there a reason we should ever try and reference it
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attempt to reference a fast ABI type's CLR name");
    }

    virtual std::string_view clr_abi_namespace() const override
    {
        return m_type->clr_abi_namespace();
    }

    [[noreturn]]
    virtual std::string_view clr_logical_namespace() const override
    {
        // Fast ABI types exist solely at the ABI and therefore we should never try and reference their 'logical' name
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attempt to reference a fast ABI type's logical namespace");
    }

    virtual std::string_view cpp_abi_name() const override
    {
        return m_typeName;
    }

    [[noreturn]]
    virtual std::string_view cpp_logical_name() const override
    {
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attempt to reference a fast ABI type's logical name");
    }

    virtual std::string_view mangled_name() const override
    {
        return m_mangledName;
    }

    [[noreturn]]
    virtual std::string_view generic_param_mangled_name() const override
    {
        // Fast ABI types don't exist in metadata and therefore should never appear as generic parameters
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attempt to use a fast ABI type as a generic parameter");
    }

    virtual void append_signature(sha1& hash) const override
    {
        hash.append("fastabi(");
        m_type->append_signature(hash);
        hash.append(")");
    }

    virtual std::size_t push_contract_guards(writer& w) const override
    {
        auto result = m_base->push_contract_guards(w);
        result += m_type->push_contract_guards(w);
        return result;
    }

    [[noreturn]]
    virtual void write_cpp_forward_declaration(writer&) const override
    {
        // Fast ABI types should only ever be used as base classes, which need to be defined, so we should never find
        // ourselves in a scenario where we are trying to forward declare one
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attemptint to forward declare a fast ABI type");
    }

    [[noreturn]]
    virtual void write_cpp_generic_param_logical_type(writer&) const override
    {
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attempt to use a fast ABI type as a generic parameter");
    }

    [[noreturn]]
    virtual void write_cpp_generic_param_abi_type(writer&) const override
    {
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attempt to use a fast ABI type as a generic parameter");
    }

    virtual void write_cpp_abi_name(writer& w) const override;

    [[noreturn]]
    virtual void write_cpp_abi_param(writer&) const override
    {
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attempting to use a fast ABI type as a function parameter or struct member");
    }

    [[noreturn]]
    virtual void write_c_forward_declaration(writer&) const override
    {
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attempting to forward declare a fast ABI type");
    }

    [[noreturn]]
    virtual void write_c_abi_param(writer&) const override
    {
        XLANG_ASSERT(false);
        xlang::throw_invalid("Attempting to use a fast ABI type as a function parameter or struct member");
    }
    xlang::meta::reader::TypeDef const& type() const noexcept
    {
        return m_type->type();
    }

    interface_type const& current_interface() const noexcept
    {
        return *m_type;
    }

    metadata_type const& base() const noexcept
    {
        return *m_base;
    }

    auto is_deprecated() const noexcept
    {
        // TODO: Should this propagate? E.g. if IFoo2 is deprecated, should we infer that IFoo3 is deprecated?
        return m_type->is_deprecated();
    }

    void write_cpp_definition(writer& w) const;
    void write_c_definition(writer& w) const;

private:

    metadata_type const* m_base;
    interface_type const* m_type;
    std::string m_typeName;
    std::string m_mangledName;
};
