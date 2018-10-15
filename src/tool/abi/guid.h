#pragma once

#include "common.h"
#include "meta_reader.h"

// Since the GUID type is not cross platform...
struct guid
{
    uint32_t data0;
    uint16_t data1;
    uint16_t data2;
    uint8_t data3[8];

    guid(uint32_t v0, uint16_t v1, uint16_t v2,
        uint8_t v30, uint8_t v31, uint8_t v32, uint8_t v33, uint8_t v34, uint8_t v35, uint8_t v36, uint8_t v37) :
        data0{ v0 },
        data1{ v1 },
        data2{ v2 },
        data3{ v30, v31, v32, v33, v34, v35, v36, v37 }
    {
    }

    static guid from_type(xlang::meta::reader::TypeDef const& type)
    {
        using namespace xlang;
        using namespace xlang::meta::reader;

        auto attr = get_attribute(type, metadata_namespace, guid_attribute);
        if (!attr)
        {
            throw_invalid("'Windows.Foundation.Metadata.GuidAttribute' attribute for type '", type.TypeNamespace(), ".", type.TypeName(), "' not found");
        }

        auto value = attr.Value();
        auto const& args = value.FixedArgs();
        return guid(
            std::get<uint32_t>(std::get<ElemSig>(args[0].value).value),
            std::get<uint16_t>(std::get<ElemSig>(args[1].value).value),
            std::get<uint16_t>(std::get<ElemSig>(args[2].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[3].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[4].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[5].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[6].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[7].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[8].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[9].value).value),
            std::get<uint8_t>(std::get<ElemSig>(args[10].value).value));
    }

    static guid from_type_name(xlang::meta::reader::GenericTypeInstSig const& type)
    {
        // The IID for generics is calculated off the SHA1 hash of the GUID "11F47AD5-7B73-42C0-ABAE-878B1E16ADEE", in
        // network byte order, appended with the string
        std::string hashString = "\x11\xF4\x7A\xD5\x7B\x73\x42\xC0\xAB\xAE\x87\x8B\x1E\x16\xAD\xEE";
    }
};
