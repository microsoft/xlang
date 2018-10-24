#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "common.h"
#include "generic_arg_stack.h"
#include "meta_reader.h"
#include "namespace_iterator.h"
#include "text_writer.h"

struct console_writer : xlang::text::writer_base<console_writer> {};

template <typename Int>
struct format_hex
{
    static_assert(std::is_unsigned_v<Int>);
    Int value;
};
template <typename Int>
format_hex(Int) -> format_hex<Int>;

struct indent { std::size_t additional_indentation = 0; };

struct writer : xlang::text::writer_base<writer>
{
    writer(
        std::string_view ns,
        abi_configuration const& config,
        xlang::meta::reader::cache const& cache,
        xlang::meta::reader::cache::namespace_members const& members) :
        m_namespace(ns),
        m_config(config),
        m_cache(cache),
        m_members(members)
    {
        initialize_dependencies();
    }

    using writer_base::write;

    abi_configuration const& config() const noexcept
    {
        return m_config;
    }

    void write_code(std::string_view value)
    {
        xlang::text::bind_list("::", namespace_range{ value })(*this);
    }

    static void write_uppercase(writer& w, std::string_view str)
    {
        for (auto ch : str)
        {
            w.write(static_cast<char>(::toupper(ch)));
        }
    }

    static void write_lowercase(writer& w, std::string_view str)
    {
        for (auto ch : str)
        {
            w.write(static_cast<char>(::tolower(ch)));
        }
    }

    template <typename Int>
    void write(format_hex<Int> val)
    {
        write_value(val.value);
    }

    void write(indent value)
    {
        for (std::size_t i = 0; i < (m_indentation + value.additional_indentation); ++i)
        {
            write("    ");
        }
    }

    void write_value(bool value)
    {
        write(value ? "TRUE" : "FALSE");
    }

    void write_value(char16_t value)
    {
        write_printf("%#0hx", value);
    }

    void write_value(int8_t value)
    {
        write_printf("%hhd", value);
    }

    void write_value(uint8_t value)
    {
        write_printf("%#0hhx", value);
    }

    void write_value(int16_t value)
    {
        write_printf("%hd", value);
    }

    void write_value(uint16_t value)
    {
        write_printf("%#0hx", value);
    }

    void write_value(int32_t value)
    {
        write_printf("%d", value);
    }

    void write_value(uint32_t value)
    {
        write_printf("%#0x", value);
    }

    void write_value(int64_t value)
    {
        write_printf("%lld", value);
    }

    void write_value(uint64_t value)
    {
        write_printf("%#0llx", value);
    }

    void write_value(float value)
    {
        write_printf("%f", value);
    }

    void write_value(double value)
    {
        write_printf("%f", value);
    }

    void write_value(std::string_view value)
    {
        write("\"%\"", value);
    }

    void write(xlang::meta::reader::Constant const& value)
    {
        using namespace xlang::meta::reader;
        switch (value.Type())
        {
        case ConstantType::Boolean:
            write_value(value.ValueBoolean());
            break;
        case ConstantType::Char:
            write_value(value.ValueChar());
            break;
        case ConstantType::Int8:
            write_value(value.ValueInt8());
            break;
        case ConstantType::UInt8:
            write_value(value.ValueUInt8());
            break;
        case ConstantType::Int16:
            write_value(value.ValueInt16());
            break;
        case ConstantType::UInt16:
            write_value(value.ValueUInt16());
            break;
        case ConstantType::Int32:
            write_value(value.ValueInt32());
            break;
        case ConstantType::UInt32:
            write_value(value.ValueUInt32());
            break;
        case ConstantType::Int64:
            write_value(value.ValueInt64());
            break;
        case ConstantType::UInt64:
            write_value(value.ValueUInt64());
            break;
        case ConstantType::Float32:
            write_value(value.ValueFloat32());
            break;
        case ConstantType::Float64:
            write_value(value.ValueFloat64());
            break;
        case ConstantType::String:
            write_value(value.ValueString());
            break;
        case ConstantType::Class:
            write("null");
            break;
        }
    }

    void push_namespace(std::string_view ns);
    void push_generic_namespace(std::string_view ns);
    void pop_namespace();
    void pop_generic_namespace();

    std::size_t push_contract_guards(xlang::meta::reader::TypeDef const& type);
    std::size_t push_contract_guards(xlang::meta::reader::TypeRef const& ref);
    std::size_t push_contract_guards(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type);
    std::size_t push_contract_guards(xlang::meta::reader::TypeSig const& type);
    std::size_t push_contract_guards(xlang::meta::reader::GenericTypeInstSig const& type);
    std::size_t push_contract_guards(xlang::meta::reader::Field const& field);
    void pop_contract_guards(std::size_t count);

    std::pair<bool, std::string_view> should_declare(xlang::meta::reader::TypeDef const& type);
    std::pair<bool, std::string_view> should_declare(xlang::meta::reader::GenericTypeInstSig const& type);

    void write_api_contract_definitions();
    void write_includes();
    void write_interface_forward_declarations();
    void write_generics_definitions();
    void write_type_dependencies();
    void write_type_declarations();
    void write_type_definitions();

private:

    void initialize_dependencies();
    void initialize_dependencies(xlang::meta::reader::TypeDef const& type);
    void add_dependency(xlang::meta::reader::TypeSig const& type);
    void add_dependency(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type);
    void add_dependency(xlang::meta::reader::GenericTypeInstSig const& type);

    std::size_t push_contract_guards(contract_version& vers);
    std::pair<bool, std::string_view> should_declare(std::string mangledName);

    void write_generic_definition(xlang::meta::reader::GenericTypeInstSig const& type);

    std::string_view m_namespace;
    abi_configuration const& m_config;
    xlang::meta::reader::cache const& m_cache;
    xlang::meta::reader::cache::namespace_members const& m_members;

    std::size_t m_indentation = 0;
    std::vector<std::string_view> m_namespaceStack;

    std::vector<contract_version> m_contractGuardStack;

    // A set of already declared (or in the case of generics, defined) types
    std::set<std::string> m_typeDeclarations;

    std::set<std::string_view> m_dependentNamespaces;
    std::set<xlang::meta::reader::TypeDef, typename_compare> m_dependencies;
    std::map<type_name, std::vector<xlang::meta::reader::GenericTypeInstSig>> m_genericReferences;

    generic_arg_stack::type m_genericArgStack;
    std::size_t m_currentGenericArgIndex = 0;
};

void write_abi_header(
    std::string_view ns,
    xlang::meta::reader::cache const& c,
    xlang::meta::reader::cache::namespace_members const& members,
    abi_configuration const& config);
