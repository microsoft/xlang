#pragma once

#include "abi_writer.h"
#include "type_writers.h"

inline void write_contract_macro(writer& w, std::string_view contractNamespace, std::string_view contractTypeName)
{
    using namespace xlang::text;
    w.write("%_%_VERSION",
        bind_list<writer::write_uppercase>("_", namespace_range{ contractNamespace }),
        bind<writer::write_uppercase>(contractTypeName));
}

inline void write_forward_declaration(
    writer& w,
    xlang::meta::reader::TypeDef const& type,
    generic_arg_stack const& genericArgs = generic_arg_stack::empty())
{
    using namespace xlang::meta::reader;
    using namespace xlang::text;

    // Generics should be defined, not declared
    XLANG_ASSERT(distance(type.GenericParam()) == 0);

    auto typeNamespace = type.TypeNamespace();
    auto typeName = type.TypeName();

    // Some types are "projected" and therefore should not get forward declared
    if (typeNamespace == foundation_namespace)
    {
        if ((typeName == async_info) || (typeName == async_status))
        {
            return;
        }
    }

    if (auto [shouldDeclare, mangledName] = w.should_declare(type); shouldDeclare)
    {
        auto typeCategory = get_category(type);

        // Only interfaces (and therefore delegates) need the include guards/typedef
        if ((typeCategory == category::interface_type) || (typeCategory == category::delegate_type))
        {
            w.write(R"^-^(#ifndef __%_FWD_DEFINED__
#define __%_FWD_DEFINED__
)^-^", mangledName, mangledName);
        }

        w.push_namespace(typeNamespace);

        switch (typeCategory)
        {
        case category::interface_type:
            w.write("%interface %;\n", indent{}, typeName);
            break;

        case category::class_type:
            w.write("%class %;\n", indent{}, typeName);
            break;

        case category::enum_type:
            // TODO: More than just int?
            w.write("%enum % : int;\n", indent{}, typeName);
            break;

        case category::struct_type:
            w.write("%struct %;\n", indent{}, typeName);
            break;

        case category::delegate_type:
            w.write("%interface I%;\n", indent{}, typeName);
            break;
        }

        w.pop_namespace();

        if ((typeCategory == category::interface_type) || (typeCategory == category::delegate_type))
        {
            w.write(R"^-^(#define % %

#endif // __%_FWD_DEFINED__
)^-^", mangledName, bind<write_type_def<type_format::cpp>>(type, genericArgs), mangledName);
        }

        w.write("\n");
    }
}
