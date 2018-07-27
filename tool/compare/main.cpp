#define NOMINMAX

#include <windows.h>
#include <winrt/base.h>
#include "meta_reader.h"
#include <filesystem>
#include <cor.h>
#include <corerror.h>
#include <corhdr.h>
#include <rometadataapi.h>
#include <rometadata.h>

using namespace std::filesystem;
using namespace winrt;
using namespace xlang;
using namespace xlang::meta::reader;

template <typename T>
void compare(IMetaDataTables2* tables, ULONG const table_index, table<T> const& table)
{
    char const* name{};
    ULONG row_size{};
    ULONG row_count{};
    ULONG column_count{};
    check_hresult(tables->GetTableInfo(table_index, &row_size, &row_count, &column_count, nullptr, &name));

    if (row_count != table.size())
    {
        printf("ERROR: table '%s' row count expected:%d actual:%d\n", name, row_count, table.size());
    }

    if (row_size != table.row_size())
    {
        printf("ERROR: table '%s' row size expected:%d actual:%d\n", name, row_size, table.row_size());

        for (ULONG column{}; column < column_count; ++column)
        {
            ULONG offset{};
            ULONG size{};
            char const* column_name{};
            check_hresult(tables->GetColumnInfo(table_index, column, &offset, &size, nullptr, &column_name));

            printf("column '%s' size expected:%d actual:%d\n", column_name, size, table.column_size(column));
        }
    }
}

void compare(IMetaDataTables2* tables, database const& db)
{
    printf("\nFile: %s\n", c_str(db.path()));
    ULONG count{};
    check_hresult(tables->GetNumTables(&count));

    for (ULONG index{}; index < count; ++index)
    {
        ULONG row_count{};
        char const* name{};

        check_hresult(tables->GetTableInfo(index, nullptr, &row_count, nullptr, nullptr, &name));

        if (name == "Module"sv)
        {
            compare(tables, index, db.Module);
        }
        else if (name == "TypeRef"sv)
        {
            compare(tables, index, db.TypeRef);
        }
        else if (name == "TypeDef"sv)
        {
            compare(tables, index, db.TypeDef);
        }
        else if (name == "Field"sv)
        {
            compare(tables, index, db.Field);
        }
        else if (name == "Param"sv)
        {
            compare(tables, index, db.Param);
        }
        else if (name == "InterfaceImpl"sv)
        {
            compare(tables, index, db.InterfaceImpl);
        }
        else if (name == "MemberRef"sv)
        {
            compare(tables, index, db.MemberRef);
        }
        else if (name == "Constant"sv)
        {
            compare(tables, index, db.Constant);
        }
        else if (name == "CustomAttribute"sv)
        {
            compare(tables, index, db.CustomAttribute);
        }
        else if (name == "FieldMarshal"sv)
        {
            compare(tables, index, db.FieldMarshal);
        }
        else if (name == "StandAloneSig"sv)
        {
            compare(tables, index, db.StandAloneSig);
        }
        else if (name == "EventMap"sv)
        {
            compare(tables, index, db.EventMap);
        }
        else if (name == "Event"sv)
        {
            compare(tables, index, db.Event);
        }
        else if (name == "PropertyMap"sv)
        {
            compare(tables, index, db.PropertyMap);
        }
        else if (name == "Property"sv)
        {
            compare(tables, index, db.Property);
        }
        else if (name == "MethodSemantics"sv)
        {
            compare(tables, index, db.MethodSemantics);
        }
        else if (name == "MethodImpl"sv)
        {
            compare(tables, index, db.MethodImpl);
        }
        else if (name == "ModuleRef"sv)
        {
            compare(tables, index, db.ModuleRef);
        }
        else if (name == "TypeSpec"sv)
        {
            compare(tables, index, db.TypeSpec);
        }
        else if (name == "ImplMap"sv)
        {
            compare(tables, index, db.ImplMap);
        }
        else if (name == "FieldRVA"sv)
        {
            compare(tables, index, db.FieldRVA);
        }
        else if (name == "Assembly"sv)
        {
            compare(tables, index, db.Assembly);
        }
        else if (name == "AssemblyProcessor"sv)
        {
            compare(tables, index, db.AssemblyProcessor);
        }
        else if (name == "AssemblyOS"sv)
        {
            compare(tables, index, db.AssemblyOS);
        }
        else if (name == "AssemblyRef"sv)
        {
            compare(tables, index, db.AssemblyRef);
        }
        else if (name == "AssemblyRefProcessor"sv)
        {
            compare(tables, index, db.AssemblyRefProcessor);
        }
        else if (name == "AssemblyRefOS"sv)
        {
            compare(tables, index, db.AssemblyRefOS);
        }
        else if (name == "File"sv)
        {
            compare(tables, index, db.File);
        }
        else if (name == "DeclSecurity"sv)
        {
            compare(tables, index, db.DeclSecurity);
        }
        else if (name == "ClassLayout"sv)
        {
            compare(tables, index, db.ClassLayout);
        }
        else if (name == "FieldLayout"sv)
        {
            compare(tables, index, db.FieldLayout);
        }
        else if (name == "ExportedType"sv)
        {
            compare(tables, index, db.ExportedType);
        }
        else if (name == "ManifestResource"sv)
        {
            compare(tables, index, db.ManifestResource);
        }
        else if (name == "NestedClass"sv)
        {
            compare(tables, index, db.NestedClass);
        }
        else if (name == "GenericParam"sv)
        {
            compare(tables, index, db.GenericParam);
        }
        else if (name == "MethodSpec"sv)
        {
            compare(tables, index, db.MethodSpec);
        }
        else if (name == "GenericParamConstraint"sv)
        {
            compare(tables, index, db.GenericParamConstraint);
        }
        else if (name == "Method"sv)
        {
            compare(tables, index, db.MethodDef);
        }
        else if (row_count > 0)
        {
            printf("WARNING: unknown table %s (%d)\n", name, row_count);
        }
    }
}

auto directory_vector(std::string_view const& folder)
{
    std::vector<std::string> files;

    for (auto&& file : directory_iterator(folder))
    {
        files.push_back(file.path().string());
    }

    return files;
}

int main(int, char**)
{
    try
    {
        winrt::init_apartment();

        com_ptr<IMetaDataDispenser> dispenser;
        check_hresult(MetaDataGetDispenser(CLSID_CorMetaDataDispenser, IID_IMetaDataDispenser, dispenser.put_void()));

        for (auto&& file : directory_iterator(R"(C:\Windows\System32\WinMetadata)"))
        {
            com_ptr<IMetaDataTables2> tables;
            check_hresult(dispenser->OpenScope(file.path().c_str(), ofRead, IID_IMetaDataTables2, reinterpret_cast<::IUnknown**>(tables.put())));

            compare(tables.get(), database{ file.path().string(), nullptr });
        }
    }
    catch (winrt::hresult_error const& e)
    {
        printf("ERROR: %ls\n", e.message().c_str());
    }
}
