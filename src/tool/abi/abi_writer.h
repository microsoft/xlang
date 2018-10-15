#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "common.h"
#include "generic_arg_stack.h"
#include "guid.h"
#include "meta_reader.h"
#include "namespace_iterator.h"
#include "text_writer.h"

struct console_writer : xlang::text::writer_base<console_writer> {};

template <typename Int>
struct format_hex
{
    Int value;
};
template <typename Int>
format_hex(Int) -> format_hex<Int>;

struct indent {};

struct type_name
{
    std::string_view ns;
    std::string_view name;

    bool operator<(type_name const& other) const
    {
        if (auto cmp = ns.compare(other.ns); cmp != 0)
        {
            return cmp < 0;
        }
        else
        {
            // Same namespace
            return name < other.name;
        }
    }
};

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
        using unsigned_type = std::make_unsigned_t<Int>;

        auto shift = (8 * sizeof(Int)) - 4;
        auto mask = static_cast<unsigned_type>(0xF) << shift;

        write("0x");
        bool printZeros = false;
        while (mask)
        {
            constexpr char digits[] = "0123456789ABCDEF";

            auto value = static_cast<unsigned_type>(val.value & mask) >> shift;
            if (value || printZeros)
            {
                write(digits[value]);
                printZeros = true;
            }

            mask >>= 4;
            shift -= 4;
        }

        if (!printZeros)
        {
            XLANG_ASSERT(val.value == 0);
            write("0");
        }
    }

    void write(indent)
    {
        for (std::size_t i = 0; i < m_indentation; ++i)
        {
            write("    ");
        }
    }

    void push_namespace(std::string_view ns);
    void push_generic_namespace(std::string_view ns);
    void pop_namespace();
    void pop_generic_namespace();

    void push_contract_guard(std::string_view contractTypeName, uint32_t version);
    std::size_t push_contract_guards(xlang::meta::reader::TypeDef const& type);
    std::size_t push_contract_guards(xlang::meta::reader::TypeRef const& ref);
    std::size_t push_contract_guards(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type);
    std::size_t push_contract_guards(xlang::meta::reader::TypeSig const& type);
    std::size_t push_contract_guards(xlang::meta::reader::GenericTypeInstSig const& type);
    void pop_contract_guard(std::size_t count);

    std::pair<bool, std::string_view> should_declare(xlang::meta::reader::TypeDef const& type);
    std::pair<bool, std::string_view> should_declare(xlang::meta::reader::GenericTypeInstSig const& type);

    void write_api_contract_definitions();
    void write_includes();
    void write_interface_forward_declarations();
    void write_generics_definitions();

private:

    void initialize_dependencies();
    void initialize_dependencies(xlang::meta::reader::TypeDef const& type);
    void add_dependency(xlang::meta::reader::TypeSig const& type);
    void add_dependency(xlang::meta::reader::coded_index<xlang::meta::reader::TypeDefOrRef> const& type);
    void add_dependency(xlang::meta::reader::GenericTypeInstSig const& type);

    void write_generic_definition(xlang::meta::reader::GenericTypeInstSig const& type);

    std::string_view m_namespace;
    abi_configuration const& m_config;
    xlang::meta::reader::cache const& m_cache;
    xlang::meta::reader::cache::namespace_members const& m_members;

    std::size_t m_indentation = 0;
    std::vector<std::string_view> m_namespaceStack;

    std::vector<std::pair<std::string_view, uint32_t>> m_contractGuardStack;

    // A set of already declared (or in the case of generics, defined) types
    std::set<std::string> m_typeDeclarations;

    std::set<std::string_view> m_dependentNamespaces;
    std::map<type_name, std::vector<xlang::meta::reader::GenericTypeInstSig>> m_genericReferences;

    generic_arg_stack::type m_genericArgStack;
    std::size_t m_currentGenericArgIndex = 0;
};

void write_abi_header(
    std::string_view ns,
    xlang::meta::reader::cache const& c,
    xlang::meta::reader::cache::namespace_members const& members,
    abi_configuration const& config);
