// https://www.ecma-international.org/publications/standards/Ecma-335.htm

#define NOMINMAX

#include <windows.h>

#include "meta_reader.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#ifdef DEBUG

#include <Windows.h>

namespace xlang::impl
{
    static_assert(sizeof(image_dos_header) == sizeof(IMAGE_DOS_HEADER));
    static_assert(sizeof(image_file_header) == sizeof(IMAGE_FILE_HEADER));
    static_assert(sizeof(image_data_directory) == sizeof(IMAGE_DATA_DIRECTORY));
    static_assert(sizeof(image_optional_header32) == sizeof(IMAGE_OPTIONAL_HEADER32));
    static_assert(sizeof(image_nt_headers32) == sizeof(IMAGE_NT_HEADERS32));
    static_assert(sizeof(image_section_header) == sizeof(IMAGE_SECTION_HEADER));
    static_assert(sizeof(image_cor20_header) == sizeof(IMAGE_COR20_HEADER));
}

#endif

using namespace xlang;
using namespace xlang::meta::reader;

template <typename T>
using range = std::pair<T, T>;

void print_type_name(coded_index<TypeDefOrRef> const& index, range<GenericParam> const& generic_params);

void print_type_name(TypeRef const& type)
{
    printf("%s.%s",
        c_str(type.TypeNamespace()),
        c_str(type.TypeName()));
}

void print_type_name(TypeDef const& type, range<GenericParam> const& generic_params)
{
    printf("%s.%s",
        c_str(type.TypeNamespace()),
        c_str(type.TypeName()));

    bool found = false;
    for (auto && param : generic_params)
    {
        if (found)
        {
            printf(", %s", c_str(param.Name()));
        }
        else
        {
            found = true;
            printf("<%s", c_str(param.Name()));
        }
    }
    if (found)
    {
        printf(">");
    }
}

void print_type_name(xlang::meta::reader::GenericTypeInstSig const& signature, range<GenericParam> const& generic_params);

template <typename ... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <typename ... Ts> overloaded(Ts...)->overloaded<Ts...>;

void print_type_name(xlang::meta::reader::TypeSig type, range<GenericParam> const& generic_params)
{
    std::visit(overloaded{
        [](ElementType type)
    {
        if (type <= ElementType::String)
        {
            static constexpr const char* primitives[] = {
                "End",
                "Void",
                "Boolean",
                "Char",
                "Int8",
                "UInt8",
                "Int16",
                "UInt16",
                "Int32",
                "UInt32",
                "Int64",
                "UInt64",
                "Float32",
                "Float64",
                "String"
            };
            printf("%s", primitives[static_cast<uint32_t>(type)]);
        }
        else if (type == ElementType::Object)
        {
            printf("Object");
        }
    },
        [&generic_params](GenericTypeIndex var)
    {
        printf("%s", c_str(begin(generic_params)[var.index].Name()));
    },
        [&generic_params](auto&& type)
    {
        print_type_name(type, generic_params);
    },
        }, type.Type());
}

void print_value(bool value)
{
    printf("%s", value ? "true" : "false");
}

void print_value(char16_t value)
{
    printf("%#0hx", value);
}

void print_value(int8_t value)
{
    printf("%hhd", value);
}

void print_value(uint8_t value)
{
    printf("%#0hhx", value);
}

void print_value(int16_t value)
{
    printf("%hd", value);
}

void print_value(uint16_t value)
{
    printf("%#0hx", value);
}

void print_value(int32_t value)
{
    printf("%d", value);
}

void print_value(uint32_t value)
{
    printf("%#0x", value);
}

void print_value(int64_t value)
{
    printf("%lld", value);
}

void print_value(uint64_t value)
{
    printf("%#0llx", value);
}

void print_value(float value)
{
    printf("%f", value);
}

void print_value(double value)
{
    printf("%f", value);
}

void print_value(std::string_view value)
{
    printf("%s", c_str(value));
}

void print_constant(Constant const& value)
{
    switch (value.Type())
    {
    case ConstantType::Boolean:
        print_value(value.ValueBoolean());
        break;
    case ConstantType::Char:
        print_value(value.ValueChar());
        break;
    case ConstantType::Int8:
        print_value(value.ValueInt8());
        break;
    case ConstantType::UInt8:
        print_value(value.ValueUInt8());
        break;
    case ConstantType::Int16:
        print_value(value.ValueInt16());
        break;
    case ConstantType::UInt16:
        print_value(value.ValueUInt16());
        break;
    case ConstantType::Int32:
        print_value(value.ValueInt32());
        break;
    case ConstantType::UInt32:
        print_value(value.ValueUInt32());
        break;
    case ConstantType::Int64:
        print_value(value.ValueInt64());
        break;
    case ConstantType::UInt64:
        print_value(value.ValueUInt64());
        break;
    case ConstantType::Float32:
        print_value(value.ValueFloat32());
        break;
    case ConstantType::Float64:
        print_value(value.ValueFloat64());
        break;
    case ConstantType::String:
        print_value(value.ValueString());
        break;
    case ConstantType::Class:
        printf("null");
        break;
    }
}

void print_type_name(xlang::meta::reader::GenericTypeInstSig const& signature, range<GenericParam> const& generic_params)
{
    print_type_name(signature.GenericType(), generic_params);
    XLANG_ASSERT(signature.GenericArgCount() >= 1);

    printf("<");
    bool first = true;
    for (auto && arg : signature.GenericArgs())
    {
        if (first)
        {
            first = false;
            print_type_name(arg, generic_params);
        }
        else
        {
            printf(", ");
            print_type_name(arg, generic_params);
        }
    }
    printf(">");
}

void print_type_name(xlang::meta::reader::TypeSpec const& type, range<GenericParam> const& generic_params)
{
    print_type_name(type.Signature().GenericTypeInst(), generic_params);
}

void print_type_name(xlang::meta::reader::coded_index<TypeDefOrRef> const& index, range<GenericParam> const& generic_params)
{
    switch (index.type())
    {
    case TypeDefOrRef::TypeDef:
        print_type_name(index.TypeDef(), generic_params);
        break;

    case TypeDefOrRef::TypeRef:
        print_type_name(index.TypeRef());
        break;

    case TypeDefOrRef::TypeSpec:
        print_type_name(index.TypeSpec(), generic_params);
        break;
    }
}

void print(xlang::meta::reader::ElemSig const& arg)
{
    std::visit(overloaded{
        [](ElemSig::SystemType arg)
    {
        print_value(arg.name);
    },
        [](ElemSig::EnumValue arg)
    {
        std::visit([](auto&& value) { print_value(value); }, arg.value);
    },
        [](auto&& arg)
    {
        print_value(arg);
    },
        }, arg.value);
}

void print(std::vector<xlang::meta::reader::ElemSig> const& arg)
{
    printf("[ ");
    bool first = true;
    for (auto const& elem : arg)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            printf(", ");
        }
        print(elem);
    }
    printf(" ]");
}

int main(int, char* argv[])
{
    auto const metadata_file = "Windows.Foundation.winmd";
    auto path = fs::canonical(argv[0]).remove_filename();

    while (!fs::exists(path / metadata_file))
    {
        if (path == path.root_path())
        {
            printf("Could not locate %s. Exiting", metadata_file);
            return -1;
        }

        path = path.remove_filename();
    }

    path /= metadata_file;

    try
    {
        printf("Dumping %s\n\n", path.string().c_str());

        cache cache{ {path.string()} };

        for (auto&& type : cache.databases().begin()->TypeDef)
        {
            const auto& generic_params = type.GenericParam();
            print_type_name(type, generic_params);
            printf("\n");

            auto extends = type.Extends();

            if (extends)
            {
                if (extends.type() == TypeDefOrRef::TypeRef)
                {
                    printf("  extends ");
                    print_type_name(extends.TypeRef());
                    printf("\n");
                }
            }

            for (auto&& impl : type.InterfaceImpl())
            {
                assert(impl.Class() == type);
                auto require = impl.Interface();

                printf("  require ");
                if (require.type() == TypeDefOrRef::TypeRef)
                {
                    print_type_name(require.TypeRef());
                }
                else if (require.type() == TypeDefOrRef::TypeSpec)
                {
                    print_type_name(require.TypeSpec(), generic_params);
                }
                printf("\n");
            }

            for (auto&& attribute : type.CustomAttribute())
            {
                auto attribute_index = attribute.Type();

                if (attribute_index.type() == CustomAttributeType::MemberRef)
                {
                    auto class_index = attribute_index.MemberRef().Class();

                    if (class_index.type() == MemberRefParent::TypeRef)
                    {
                        printf("  attribute ");
                        print_type_name(class_index.TypeRef());

                        auto const& signature = attribute_index.MemberRef().MethodSignature();
                        XLANG_ASSERT(!signature.ReturnType().Type());

                        auto const& blob = attribute.Value();
                        auto fixed_arg_iterator = blob.FixedArgs().cbegin();

                        printf("(");
                        bool found = false;
                        for (auto&& param : signature.Params())
                        {
                            if (found)
                            {
                                printf(", ");
                            }
                            else
                            {
                                found = true;
                            }
                            print_type_name(param.Type(), std::pair{ GenericParam{}, GenericParam{} });
                            printf(" ");
                            std::visit([](auto&& arg) { print(arg); }, fixed_arg_iterator->value);
                            ++fixed_arg_iterator;
                        }
                        printf(")");

                        printf("\n");
                    }
                }
            }

            for (auto&& field : type.FieldList())
            {
                printf("  field ");
                if (field.is_literal())
                {
                    printf("literal ");
                }
                else if (field.is_static())
                {
                    printf("static ");
                }
                print_type_name(field.Signature().Type(), generic_params);
                printf(" %s", c_str(field.Name()));
                auto const& constant = field.Constant();
                if (constant)
                {
                    printf(" = ");
                    print_constant(*constant);
                }
                printf("\n");
            }

            for (auto && method : type.MethodList())
            {
                method.Signature();
            }

            for (auto && property : type.PropertyList())
            {
                printf("  property ");
                print_type_name(property.Type().Type(), generic_params);
                printf(" %s", c_str(property.Name()));
                printf("\n");
                XLANG_ASSERT(property.Parent() == type);
                for (auto && semantic : property.MethodSemantic())
                {
                    if (enum_mask(semantic.Semantic(), MethodSemanticsAttributes::Getter) == MethodSemanticsAttributes::Getter)
                    {
                        printf("    getter ");
                    }
                    else if (enum_mask(semantic.Semantic(), MethodSemanticsAttributes::Setter) == MethodSemanticsAttributes::Setter)
                    {
                        printf("    setter ");
                    }
                    else
                    {
                        XLANG_ASSERT(false);
                    }
                    printf(" %s", c_str(semantic.Method().Name()));
                    printf("\n");
                }
            }

            puts("");
        }
    }
    catch (std::exception const& e)
    {
        puts(e.what());
    }
}
