// https://www.ecma-international.org/publications/standards/Ecma-335.htm

#include "meta_reader.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#if WINDOWS_PLATFORM_WINDOWS
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
#endif

using namespace xlang;
using namespace xlang::meta::reader;

void print_type_name(coded_index<TypeDefOrRef> const& index, range<row_iterator<GenericParam>> const& generic_params);

void print_type_name(TypeRef const& type)
{
    printf("%s.%s",
        c_str(type.TypeNamespace()),
        c_str(type.TypeName()));
}

void print_type_name(TypeDef const& type, range<row_iterator<GenericParam>> const& generic_params)
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

void print_type_name(xlang::meta::reader::GenericTypeInstSig const& signature, range<row_iterator<GenericParam>> const& generic_params);

template <typename ... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <typename ... Ts> overloaded(Ts...)->overloaded<Ts...>;

void print_type_name(xlang::meta::reader::TypeSig type, range<row_iterator<GenericParam>> const& generic_params)
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
                "I1",
                "U1",
                "I2",
                "U2",
                "I4",
                "U4",
                "I8",
                "U8",
                "R4",
                "R8",
                "String"
            };
            printf("%s", primitives[static_cast<uint32_t>(type)]);
        }
        else if (type == ElementType::Object)
        {
            printf("Object");
        }
    },
        [&generic_params](uint32_t var)
    {
        printf("%s", c_str(generic_params.begin()[var].Name()));
    },
        [&generic_params](auto&& type)
    {
        print_type_name(type, generic_params);
    },
        }, type.Type());
}

void print_type_name(xlang::meta::reader::GenericTypeInstSig const& signature, range<row_iterator<GenericParam>> const& generic_params)
{
    print_type_name(signature.GenericType(), generic_params);
    WINRT_ASSERT(signature.GenericArgCount() >= 1);

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

void print_type_name(xlang::meta::reader::TypeSpec const& type, range<row_iterator<GenericParam>> const& generic_params)
{
    print_type_name(type.Signature().GenericTypeInst(), generic_params);
}

void print_type_name(xlang::meta::reader::coded_index<TypeDefOrRef> const& index, range<row_iterator<GenericParam>> const& generic_params)
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

int main(int, char* argv[])
{
    winrt::init_apartment();

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

        database const db{ path.string() };

        for (auto&& type : db.TypeDef)
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
                        WINRT_ASSERT(signature.ReturnType().IsVoid());

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
                            print_type_name(param.Type(), range{ std::pair{ row_iterator<GenericParam>{}, row_iterator<GenericParam>{} } });
                        }
                        printf(")");

                        printf("\n");
                    }

                }
            }

            puts("");
        }
    }
    catch (winrt::hresult_error const& e)
    {
        printf("%ls\n", reinterpret_cast<const wchar_t*>(e.message().c_str()));
    }
}
