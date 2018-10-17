"use strict";

function initializeScript()
{
    return [
        new host.typeSignatureRegistration(__DatabaseVisualizer, "xlang::meta::reader::database"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("undefined"), "xlang::meta::reader::table_base"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("undefined"), "xlang::meta::reader::table<*>"),

        // row_base derived types
        new host.typeSignatureRegistration(__TypeDefVisualizer, "xlang::meta::reader::TypeDef"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__TypeDefVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::TypeDef>"),

        new host.typeSignatureRegistration(__TypeRefVisualizer, "xlang::meta::reader::TypeRef"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__TypeRefVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::TypeRef>"),

        new host.typeSignatureRegistration(__TypeSpecVisualizer, "xlang::meta::reader::TypeSpec"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__TypeSpecVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::TypeSpec>"),

        new host.typeSignatureRegistration(__CustomAttributeVisualizer, "xlang::meta::reader::CustomAttribute"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__CustomAttributeVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::CustomAttribute>"),

        new host.typeSignatureRegistration(__MethodDefVisualizer, "xlang::meta::reader::MethodDef"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__MethodDefVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::MethodDef>"),

        new host.typeSignatureRegistration(__MemberRefVisualizer, "xlang::meta::reader::MemberRef"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__MemberRefVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::MemberRef>"),

        new host.typeSignatureRegistration(__FieldVisualizer, "xlang::meta::reader::Field"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__FieldVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::Field>"),

        new host.typeSignatureRegistration(__ParamVisualizer, "xlang::meta::reader::Param"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__ParamVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::Param>"),

        new host.typeSignatureRegistration(__InterfaceImplVisualizer, "xlang::meta::reader::InterfaceImpl"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__InterfaceImplVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::InterfaceImpl>"),

        new host.typeSignatureRegistration(__ModuleVisualizer, "xlang::meta::reader::Module"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__ModuleVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::Module>"),

        // TODO? Permission

        new host.typeSignatureRegistration(__PropertyVisualizer, "xlang::meta::reader::Property"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__PropertyVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::Property>"),

        new host.typeSignatureRegistration(__EventVisualizer, "xlang::meta::reader::Event"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__EventVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::Event>"),

        new host.typeSignatureRegistration(__StandAloneSigVisualizer, "xlang::meta::reader::StandAloneSig"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__StandAloneSigVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::StandAloneSig>"),

        new host.typeSignatureRegistration(__ModuleRefVisualizer, "xlang::meta::reader::ModuleRef"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__ModuleRefVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::ModuleRef>"),

        new host.typeSignatureRegistration(__AssemblyVisualizer, "xlang::meta::reader::Assembly"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__AssemblyVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::Assembly>"),

        new host.typeSignatureRegistration(__AssemblyRefVisualizer, "xlang::meta::reader::AssemblyRef"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__AssemblyRefVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::AssemblyRef>"),

        new host.typeSignatureRegistration(__FileVisualizer, "xlang::meta::reader::File"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__FileVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::File>"),

        new host.typeSignatureRegistration(__ExportedTypeVisualizer, "xlang::meta::reader::ExportedType"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__ExportedTypeVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::ExportedType>"),

        new host.typeSignatureRegistration(__ManifestResourceVisualizer, "xlang::meta::reader::ManifestResource"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__ManifestResourceVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::ManifestResource>"),

        new host.typeSignatureRegistration(__GenericParamVisualizer, "xlang::meta::reader::GenericParam"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__GenericParamVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::GenericParam>"),

        new host.typeSignatureRegistration(__GenericParamConstraintVisualizer, "xlang::meta::reader::GenericParamConstraint"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__GenericParamConstraintVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::GenericParamConstraint>"),

        new host.typeSignatureRegistration(__MethodSpecVisualizer, "xlang::meta::reader::MethodSpec"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__MethodSpecVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::MethodSpec>"),

        new host.typeSignatureRegistration(__MethodImplVisualizer, "xlang::meta::reader::MethodImpl"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__MethodImplVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::MethodImpl>"),

        new host.typeSignatureRegistration(__ConstantVisualizer, "xlang::meta::reader::Constant"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__ConstantVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::Constant>"),

        new host.typeSignatureRegistration(__MethodSemanticsVisualizer, "xlang::meta::reader::MethodSemantics"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__MethodSemanticsVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::MethodSemantics>"),

        new host.typeSignatureRegistration(__PropertyMapVisualizer, "xlang::meta::reader::PropertyMap"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__PropertyMapVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::PropertyMap>"),

        new host.typeSignatureRegistration(__EventMapVisualizer, "xlang::meta::reader::EventMap"),
        new host.typeSignatureRegistration(__MakeTableVisualizer("__EventMapVisualizer"), "xlang::meta::reader::table<xlang::meta::reader::EventMap>"),
    ]
}

function __processFromContext(context)
{
    return host.namespace.Debugger.Sessions.getValueAt(context).Processes.getValueAt(context);
}

function __stripPath(moduleName)
{
    var index = moduleName.lastIndexOf("\\");
    if (index > 0)
    {
        return moduleName.substring(index + 1);
    }

    return moduleName;
}

function __modulePathFromModule(debuggerModule)
{
    return __stripPath(debuggerModule.Name);
}

function __typeFromAddress(type, address, context)
{
    // Try all modules
    var process = __processFromContext(context);
    for (var mod of process.Modules)
    {
        try
        {
            var result = host.createTypedObject(address, __modulePathFromModule(mod), type, context);
            return result;
        }
        catch (exception)
        {
        }
    }

    throw new Error("Type '" + type + "' does not exist in any loaded module");
}

function __enumToString(value, enumValues)
{
    for (var property in enumValues)
    {
        if (enumValues.hasOwnProperty(property) && (value === enumValues[property]))
        {
            return property;
        }
    }

    return "UNKNOWN (" + value + ")";
}

var __TypeDefOrRef = Object.freeze({
    TypeDef: 0,
    TypeRef: 1,
    TypeSpec: 2,

    Values: [
        {
            Valid: true,
            Table: "TypeDef",
        },
        {
            Valid: true,
            Table: "TypeRef",
        },
        {
            Valid: true,
            Table: "TypeSpec",
        },
    ],

    Name: "TypeDefOrRef",
    CodedIndexBits: 2,
});

var __HasCustomAttribute = Object.freeze({
    MethodDef: 0,
    Field: 1,
    TypeRef: 2,
    TypeDef: 3,
    Param: 4,
    InterfaceImpl: 5,
    MemberRef: 6,
    Module: 7,
    Permission: 8,
    Property: 9,
    Event: 10,
    StandAloneSig: 11,
    ModuleRef: 12,
    TypeSpec: 13,
    Assembly: 14,
    AssemblyRef: 15,
    File: 16,
    ExportedType: 17,
    ManifestResource: 18,
    GenericParam: 19,
    GenericParamConstraint: 20,
    MethodSpec: 21,

    Values: [
        {
            Valid: true,
            Table: "MethodDef",
        },
        {
            Valid: true,
            Table: "Field",
        },
        {
            Valid: true,
            Table: "TypeRef",
        },
        {
            Valid: true,
            Table: "TypeDef",
        },
        {
            Valid: true,
            Table: "Param",
        },
        {
            Valid: true,
            Table: "InterfaceImpl",
        },
        {
            Valid: true,
            Table: "MemberRef",
        },
        {
            Valid: true,
            Table: "Module",
        },
        {
            Valid: true,
            Table: "Permission",
        },
        {
            Valid: true,
            Table: "Property",
        },
        {
            Valid: true,
            Table: "Event",
        },
        {
            Valid: true,
            Table: "StandAloneSig",
        },
        {
            Valid: true,
            Table: "ModuleRef",
        },
        {
            Valid: true,
            Table: "TypeSpec",
        },
        {
            Valid: true,
            Table: "Assembly",
        },
        {
            Valid: true,
            Table: "AssemblyRef",
        },
        {
            Valid: true,
            Table: "File",
        },
        {
            Valid: true,
            Table: "ExportedType",
        },
        {
            Valid: true,
            Table: "ManifestResource",
        },
        {
            Valid: true,
            Table: "GenericParam",
        },
        {
            Valid: true,
            Table: "GenericParamConstraint",
        },
        {
            Valid: true,
            Table: "MethodSpec",
        },
    ],

    Name: "HasCustomAttribute",
    CodedIndexBits: 5,
});

var __CustomAttributeType = Object.freeze({
    MethodDef: 2,
    MemberRef: 3,

    Values: [
        {
            Valid: false,
        },
        {
            Valid: false,
        },
        {
            Valid: true,
            Table: "MethodDef",
        },
        {
            Valid: true,
            Table: "MemberRef",
        },
    ],

    Name: "CustomAttributeType",
    CodedIndexBits: 3,
});

var __MemberRefParent = Object.freeze({
    TypeDef: 0,
    TypeRef: 1,
    ModuleRef: 2,
    MethodDef: 3,
    TypeSpec: 4,

    Values: [
        {
            Valid: true,
            Table: "TypeDef",
        },
        {
            Valid: true,
            Table: "TypeRef",
        },
        {
            Valid: true,
            Table: "ModuleRef",
        },
        {
            Valid: true,
            Table: "MethodDef",
        },
        {
            Valid: true,
            Table: "TypeSpec",
        },
    ],

    Name: "MemberRef",
    CodedIndexBits: 3,
});

var __TypeOrMethodDef = Object.freeze({
    TypeDef: 0,
    MethodDef: 1,

    Values: [
        {
            Valid: true,
            Table: "TypeDef",
        },
        {
            Valid: true,
            Table: "MethodDef",
        },
    ],

    Name: "TypeOrMethodDef",
    CodedIndexBits: 1,
});

var __MethodDefOrRef = Object.freeze({
    MethodDef: 0,
    MemberRef: 1,

    Values: [
        {
            Valid: true,
            Table: "MethodDef",
        },
        {
            Valid: true,
            Table: "MemberRef",
        },
    ],

    Name: "MethodDefOrRef",
    CodedIndexBits: 1,
});

var __HasConstant = Object.freeze({
    Field: 0,
    Param: 1,
    Property: 2,

    Values: [
        {
            Valid: true,
            Table: "Field",
        },
        {
            Valid: true,
            Table: "Param",
        },
        {
            Valid: true,
            Table: "Property",
        },
    ],

    Name: "HasConstant",
    CodedIndexBits: 2,
});

var __HasSemantics = Object.freeze({
    Event: 0,
    Property: 1,

    Values: [
        {
            Valid: true,
            Table: "Event",
        },
        {
            Valid: true,
            Table: "Property",
        },
    ],

    Name: "HasSemantics",
    CodedIndexBits: 1,
});

function __codedValueType(value, codedEnum)
{
    return value & ((1 << codedEnum.CodedIndexBits) - 1);
}

function __codedValueIndex(value, codedEnum)
{
    return (value >> codedEnum.CodedIndexBits) - 1;
}

function __makeCodedValue(index, type, codedEnum)
{
    return ((index + 1) << codedEnum.CodedIndexBits) | type;
}

class __Blob
{
    constructor(data, size)
    {
        this.__data = data;
        this.__size = size;
    }

    toString()
    {
        var data = this.ReadValues(this.__size);
        var result = "";
        var prefix = "";
        for (var value of data)
        {
            result += prefix;
            prefix = " ";

            var strValue = value.toString(16);
            for (var i = 0; i < 2 - strValue.length; ++i)
            {
                result += "0";
            }
            result += strValue;
        }

        return result;
    }

    ReadValue(size, offset)
    {
        return this.ReadValues(1, size, offset)[0];
    }

    ReadValues(count, dataSize, offset)
    {
        dataSize = dataSize || 1;
        offset = offset || 0;
        return host.memory.readMemoryValues(this.__data.add(offset).address, count, dataSize);
    }

    ReadString(size, offset)
    {
        offset = offset || 0;
        size = size || (this.__size - offset);
        return host.memory.readString(this.__data.add(offset).address, size);
    }
}

class __BlobStream
{
    constructor(blob, initialPos)
    {
        this.__blob = blob;
        this.__cursor = initialPos || 0;
    }

    copy(offset)
    {
        return new __BlobStream(this.__blob, this.__cursor + initialPos);
    }

    seek(delta)
    {
        this.__cursor += delta;
    }

    consumeValue(dataSize)
    {
        var result = this.peekValue(dataSize);
        this.__cursor += dataSize;
        return result;
    }

    peekValue(dataSize)
    {
        if (this.__cursor + dataSize > this.__blob.Size)
        {
            throw new RangeError("Not enough data remaining in blob");
        }

        return this.__blob.ReadValue(dataSize, this.__cursor);
    }

    consumeUnsigned()
    {
        var result = this.peekUnsigned();
        this.__cursor += result.length;
        return result.value;
    }

    peekUnsigned()
    {
        var initialByte = this.__blob.ReadValue(1, this.__cursor);
        var length;
        if ((initialByte & 0x80) == 0)
        {
            length = 0;
        }
        else if ((initialByte & 0xC0) == 0x80)
        {
            length = 2;
            initialByte = initialByte & 0x3F;
        }
        else if ((initialByte & 0xE0) == 0xC0)
        {
            length = 4;
            initialByte = initialByte & 0x1F;
        }
        else
        {
            throw new Error("Invalid compressed integer in blob");
        }

        for (var i = 1; i < length; ++i)
        {
            initialByte = (initialByte << 8) | this.__blob.ReadValue(1, this.__cursor + i);
        }

        return {
            value: initialByte,
            length: length,
        }
    }
}

class __DatabaseVisualizer
{
    getString(index)
    {
        if (index == 0)
        {
            return null;
        }

        var ptr = this.m_strings.m_first.add(index);
        return host.memory.readString(ptr.address);
    }

    getBlob(index)
    {
        var ptr = this.m_blobs.m_first.add(index);
        var initialByte = host.memory.readMemoryValues(ptr.address, 1, 1)[0];

        // High bits indicate how many bytes are required to represent the blob size
        var blobSizeBytes;
        var blobSize;
        switch (initialByte >> 5)
        {
            case 0:
            case 1:
            case 2:
            case 3:
                blobSizeBytes = 1;
                blobSize = initialByte & 0x7F;
                break;

            case 4:
            case 5:
                blobSizeBytes = 2;
                blobSize = initialByte & 0x3F;
                break;

            case 6:
                blobSizeBytes = 4;
                blobSize = initialByte & 0x1F;
                break;

            default:
                throw new RangeError("Invalid blob encoding");
        }

        for (var i = 1; i < blobSizeBytes; ++i)
        {
            ptr = ptr.add(1);
            blobSize = (blobSize << 8) | host.memory.readMemoryValues(ptr.address, 1, 1)[0]
        }

        return new __Blob(ptr.add(1), blobSize);
    }

    get TypeRef()
    {
        return this.TypeRef;
    }

    get GenericParamConstraint()
    {
        return this.GenericParamConstraint;
    }

    get TypeSpec()
    {
        return this.TypeSpec;
    }

    get TypeDef()
    {
        return this.TypeDef;
    }

    get CustomAttribute()
    {
        return this.CustomAttribute;
    }

    get MethodDef()
    {
        return this.MethodDef;
    }

    get MemberRef()
    {
        return this.MemberRef;
    }

    get Module()
    {
        return this.Module;
    }

    get Param()
    {
        return this.Param;
    }

    get InterfaceImpl()
    {
        return this.InterfaceImpl;
    }

    get Constant()
    {
        return this.Constant;
    }

    get Field()
    {
        return this.Field;
    }

    get FieldMarshal()
    {
        return this.FieldMarshal; // TODO: Visualize
    }

    get DeclSecurity()
    {
        return this.DeclSecurity; // TODO: Visualize
    }

    get ClassLayout()
    {
        return this.ClassLayout; // TODO: Visualize
    }

    get FieldLayout()
    {
        return this.FieldLayout; // TODO: Visualize
    }

    get StandAloneSig()
    {
        return this.StandAloneSig;
    }

    get EventMap()
    {
        return this.EventMap;
    }

    get Event()
    {
        return this.Event;
    }

    get PropertyMap()
    {
        return this.PropertyMap;
    }

    get Property()
    {
        return this.Property;
    }

    get MethodSemantics()
    {
        return this.MethodSemantics;
    }

    get MethodImpl()
    {
        return this.MethodImpl;
    }

    get ModuleRef()
    {
        return this.ModuleRef;
    }

    get ImplMap()
    {
        return this.ImplMap; // TODO: Visualize
    }

    get FieldRVA()
    {
        return this.FieldRVA; // TODO: Visualize
    }

    get Assembly()
    {
        return this.Assembly;
    }

    get AssemblyProcessor()
    {
        return this.AssemblyProcessor; // TODO: Visualize
    }

    get AssemblyOS()
    {
        return this.AssemblyOS; // TODO: Visualize
    }

    get AssemblyRef()
    {
        return this.AssemblyRef;
    }

    get AssemblyRefProcessor()
    {
        return this.AssemblyRefProcessor; // TODO: Visualize
    }

    get AssemblyRefOS()
    {
        return this.AssemblyRefOS; // TODO: Visualize
    }

    get File()
    {
        return this.File;
    }

    get ExportedType()
    {
        return this.ExportedType;
    }

    get ManifestResource()
    {
        return this.ManifestResource;
    }

    get NestedClass()
    {
        return this.NestedClass; // TODO: Visualize
    }

    get GenericParam()
    {
        return this.GenericParam;
    }

    get MethodSpec()
    {
        return this.MethodSpec;
    }
}

function __MakeTableVisualizer(type)
{
    class __TableVisualizer
    {
        get Database()
        {
            return this.m_database;
        }

        get Size()
        {
            return this.m_row_count;
        }

        getDimensionality()
        {
            // TODO: Seems like we can't do this? Nor would we want to?
            // return (type == "undefined") ? this.m_columns.Count() : 1;
            return 1;
        }

        getValueAt(index)
        {
            if (type != "undefined")
            {
                return eval("new " + type + "(this, index)");
            }

            // For an untyped table, return values as arrays
            var result = new Array();
            var ptr = this.m_data.add(index * this.m_row_size);
            for (var i = 0; i < this.m_columns.Count(); ++i)
            {
                var dataSize = this.m_columns[index].size;
                var dataPtr = ptr.add(this.m_columns[i].offset);
                var value = host.memory.readMemoryValues(dataPtr.address, 1, dataSize)[0];
                result.push(value);
            }

            return result;
        }

        *[Symbol.iterator]()
        {
            var ptr = this.m_data;
            for (var i = 0; i < this.m_row_count; ++i)
            {
                yield new host.indexedValue(this.getValueAt(i), [i]);
            }
        }

        getValue(row, col)
        {
            if (row == 0)
            {
                return null;
            }
            else if (row >= this.m_row_count)
            {
                throw new RangeError("Row index out of range: " + row);
            }
            else if (col >= this.m_columns.Count())
            {
                throw new RangeError("Column index out of range: " + col);
            }

            // TODO: Probably best not to do it this way since we specialize 'getValueAt' depending on whether or not
            //       we know the underlying type
            //return this.getValueAt(row)[col];

            var dataSize = this.m_columns[col].size;
            var ptr = this.m_data.add(row * this.m_row_size + this.m_columns[col].offset);
            return host.memory.readMemoryValues(ptr.address, 1, dataSize)[0];
        }

        __lowerBound(col, value, min, max)
        {
            // First value that is not less than value
            min = min || 0;
            max = max || this.Size;

            while (min < max)
            {
                var mid = Math.floor((min + max) / 2);
                var testValue = this.getValue(mid, col);
                if (testValue < value)
                {
                    min = mid + 1;
                }
                else
                {
                    max = mid;
                }
            }

            return min;
        }

        __upperBound(col, value, min, max)
        {
            // First value that is greater than value
            min = min || 0;
            max = max || this.Size;

            while (min < max)
            {
                var mid = Math.floor((min + max) / 2);
                var testValue = this.getValue(mid, col);
                if (testValue <= value)
                {
                    min = mid + 1;
                }
                else
                {
                    max = mid;
                }
            }

            return min;
        }

        __equalRange(col, value, min, max)
        {
            var begin = this.__lowerBound(col, value, min, max);
            var end = this.__upperBound(col, value, begin, max);
            return new __TableRange(this, begin, end);
        }

        __fromCodedIndex(codedIndex, codedEnum)
        {
            var typeIndex = __codedValueType(codedIndex, codedEnum);
            if (typeIndex >= codedEnum.Values.length)
            {
                throw new RangeError("Invalid " + codedEnum.Name + " coded index: " + typeIndex);
            }

            var target = codedEnum.Values[typeIndex];
            if (!target.Valid)
            {
                throw new RangeError("Invalid " + codedEnum.Name + " coded index: " + typeIndex);
            }

            var table = this.Database[target.Table];
            var row = __codedValueIndex(codedIndex, codedEnum);
            if (row == -1)
            {
                return null;
            }

            return eval("new __" + target.Table + "Visualizer(table, row)");
        }

        __getCodedIndex(row, col, codedEnum)
        {
            return this.__fromCodedIndex(this.getValue(row, col), codedEnum);
        }

        __getList(tableName, row, col)
        {
            // Lists are stored as a single index in the current row. This marks the beginning of the list. The next row in
            // the current table is the index of one past our end
            var table = this.Database[tableName];
            var begin = this.getValue(row, col);
            var end = table.Size;
            if (row + 1 < this.Size)
            {
                end = this.getValue(row + 1, col);
            }

            return new __TableRange(table, begin - 1, end - 1);
        }

        __getTargetRow(tableName, row, col)
        {
            return this.Database[tableName][this.getValue(row, col) - 1];
        }

        __getParentRow(tableName, row, col)
        {
            // The parent references the first row in this table that belongs to its list. Thus, we need to look for the
            // first value in the parent's table that references a later value in this table and then choose the previous
            var table = this.Database[tableName];
            var index = table.__upperBound(col, row + 1) - 1;
            return table[index];
        }
    }

    return __TableVisualizer;
}

class __TableRange
{
    constructor(table, begin, end)
    {
        this.__table = table;
        this.__begin = begin;
        this.__end = end;
    }

    get Size()
    {
        return this.__end - this.__begin;
    }

    getDimensionality()
    {
        return 1;
    }

    getValueAt(index)
    {
        var pos = this.__begin + index;
        if ((pos < this.__begin) || (pos >= this.__end))
        {
            throw new RangeError("Index out of range: " + index);
        }

        return this.__table[this.__begin + index];
    }

    *[Symbol.iterator]()
    {
        for (var i = this.__begin; i < this.__end; ++i)
        {
            yield new host.indexedValue(this.__table[i], [i - this.__begin]);
        }
    }
}

class __RowBase
{
    constructor(table, index)
    {
        this.__table = table;
        this.__index = index;
    }

    get __Table()
    {
        return this.m_table || this.__table;
    }

    get __Index()
    {
        return this.m_index || this.__index;
    }

    __getValue(col)
    {
        return this.__Table.getValue(this.__Index, col);
    }

    __getString(col)
    {
        return this.__Table.Database.getString(this.__getValue(col));
    }

    __getBlob(col)
    {
        return this.__Table.Database.getBlob(this.__getValue(col));
    }

    __getCodedIndex(col, codedEnum)
    {
        return this.__Table.__getCodedIndex(this.__Index, col, codedEnum);
    }

    __getBlob(col)
    {
        return this.__Table.Database.getBlob(this.__getValue(col));
    }

    __equalRange(tableName, col, codedEnum)
    {
        // We're looking for range of elements in a different table that correspond to the row in this table. This is
        // done by inspecting coded index values in one of the table's columns to find the range that corresponds to the
        // current row
        var table = this.__Table.Database[tableName];
        var codedValue;
        if (codedEnum)
        {
            codedValue = __makeCodedValue(this.__Index, codedEnum[this.TableName], codedEnum);
        }
        else
        {
            codedValue = this.__Index + 1;
        }

        return table.__equalRange(col, codedValue);
    }

    __getList(tableName, col)
    {
        return this.__Table.__getList(tableName, this.__Index, col);
    }

    __getTargetRow(tableName, col)
    {
        return this.__Table.__getTargetRow(tableName, this.__Index, col);
    }

    __getParentRow(tableName, col)
    {
        return this.__Table.__getParentRow(tableName, this.__Index, col);
    }

    __findConstant()
    {
        var range = this.__equalRange("Constant", 1, __HasConstant);
        if (range.Size == 0)
        {
            return null;
        }
        else if (range.Size != 1)
        {
            throw new RangeError("Expected only one constant value, found: " + range.Size);
        }

        return range.getValueAt(0);
    }
}

// Table Row Types

class __TypeDefVisualizer extends __RowBase
{
    toString()
    {
        return this.TypeNamespace + "." + this.TypeName;
    }

    get TableName()
    {
        return "TypeDef";
    }

    get Flags()
    {
        return this.__getValue(0); // TODO: Type support?
    }

    get TypeName()
    {
        return this.__getString(1);
    }

    get TypeNamespace()
    {
        return this.__getString(2);
    }

    get Extends()
    {
        return this.__getCodedIndex(3, __TypeDefOrRef);
    }

    get FieldList()
    {
        return this.__getList("Field", 4);
    }

    get MethodList()
    {
        return this.__getList("MethodDef", 5);
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }

    get InterfaceImpl()
    {
        return this.__equalRange("InterfaceImpl", 0);
    }

    get GenericParam()
    {
        return this.__equalRange("GenericParam", 2, __TypeOrMethodDef);
    }

    get PropertyList()
    {
        var table = this.__Table.Database.PropertyMap;
        var index = this.__Index + 1;
        var i = 0;
        for (; i < table.Size; ++i)
        {
            if (table.getValue(i, 0) == index)
            {
                break;
            }
        }

        if (i == table.Size)
        {
            return new __TableRange(table, i, i);
        }

        return table[i].PropertyList;
    }

    get EventList()
    {
        var table = this.__Table.Database.EventMap;
        var index = this.__Index + 1;
        var i = 0;
        for (; i < table.Size; ++i)
        {
            if (table.getValue(i, 0) == index)
            {
                break;
            }
        }

        if (i == table.Size)
        {
            return new __TableRange(table, i, i);
        }

        return table[i].EventList;
    }

    get MethodImplList()
    {
        return this.__equalRange("MethodImpl", 0);
    }
}

class __TypeRefVisualizer extends __RowBase
{
    toString()
    {
        return this.TypeNamespace + "." + this.TypeName;
    }

    get TableName()
    {
        return "TypeRef";
    }

    get ResolutionScope()
    {
        return this.__getValue(0); // TODO: Type support?
    }

    get TypeName()
    {
        return this.__getString(1);
    }

    get TypeNamespace()
    {
        return this.__getString(2);
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }

    FindDefinition()
    {
        var typeNamespace = this.TypeNamespace;
        var typeName = this.TypeName;

        // Search for a TypeDef instance of the same name
        var table = this.__Table.Database.TypeDef;
        var min = 0;
        var max = table.Size;
        while (min < max)
        {
            var mid = Math.floor((min + max) / 2);
            var def = table.getValueAt(mid);
            if ((def.TypeNamespace < typeNamespace))
            {
                min = mid + 1;
            }
            else if (def.TypeNamespace == typeNamespace)
            {
                if (def.TypeName < typeName)
                {
                    min = mid + 1;
                }
                else if (def.TypeName == typeName)
                {
                    // We've found it
                    return def;
                }
                else
                {
                    max = mid;
                }
            }
            else
            {
                max = mid;
            }
        }

        // Not found
        return null;
    }
}

class __TypeSpecVisualizer extends __RowBase
{
    // toString()

    get TableName()
    {
        return "TypeSpec";
    }

    get Signature()
    {
        return new __TypeSpecSignature(this.__Table, new __BlobStream(this.__getBlob(0)));
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __CustomAttributeVisualizer extends __RowBase
{
    toString()
    {
        return this.Type.toString();
    }

    get TableName()
    {
        return "CustomAttribute";
    }

    get Parent()
    {
        return this.__getCodedIndex(0, __HasCustomAttribute);
    }

    get Type()
    {
        return this.__getCodedIndex(1, __CustomAttributeType);
    }

    // Value
}

class __MethodDefVisualizer extends __RowBase
{
    toString()
    {
        return this.Name;
    }

    get TableName()
    {
        return "MethodDef";
    }

    get RVA()
    {
        return this.__getValue(0);
    }

    // ImplFlags
    // Flags

    get Name()
    {
        return this.__getString(3);
    }

    // Signature

    get ParamList()
    {
        return this.__getList("Param", 5);
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }

    get Parent()
    {
        return this.__getParentRow("TypeDef", 5);
    }
}

class __MemberRefVisualizer extends __RowBase
{
    toString()
    {
        return this.Class.toString();
    }

    get TableName()
    {
        return "MemberRef";
    }

    get Class()
    {
        return this.__getCodedIndex(0, __MemberRefParent);
    }

    get Name()
    {
        return this.__getString(1);
    }

    // MethodSignature

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __FieldVisualizer extends __RowBase
{
    toString()
    {
        if (this.Constant == null)
        {
            return this.Name;
        }
        else
        {
            return this.Name + " = " + this.Constant.toString();
        }
    }

    get TableName()
    {
        return "Field";
    }

    // Flags

    get Name()
    {
        return this.__getString(1);
    }

    get Signature()
    {
        return new __FieldSig(this.__Table, new __BlobStream(this.__getBlob(2)));
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }

    get Constant()
    {
        return this.__findConstant();
    }

    get Parent()
    {
        return this.__getParentRow("TypeDef", 4);
    }
}

class __ParamVisualizer extends __RowBase
{
    toString()
    {
        return this.Name;
    }

    get TableName()
    {
        return "Param";
    }

    // Flags

    get Sequence()
    {
        return this.__getValue(1);
    }

    get Name()
    {
        return this.__getString(2);
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }

    get Constant()
    {
        return this.__findConstant();
    }
}

class __InterfaceImplVisualizer extends __RowBase
{
    toString()
    {
        return this.Interface.toString();
    }

    get TableName()
    {
        return "InterfaceImpl";
    }

    get Class()
    {
        return this.__getTargetRow("TypeDef", 0);
    }

    get Interface()
    {
        return this.__getCodedIndex(1, __TypeDefOrRef);
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __ModuleVisualizer extends __RowBase
{
    get TableName()
    {
        return "Module";
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __PermissionVisualizer extends __RowBase
{
    get TableName()
    {
        return "Permission";
    }

    // TODO?
}

class __PropertyVisualizer extends __RowBase
{
    toString()
    {
        return this.Name;
    }

    get TableName()
    {
        return "Property";
    }

    // Flags

    get Name()
    {
        return this.__getString(1);
    }

    // Type

    get MethodSemantic()
    {
        return this.__equalRange("MethodSemantics", 2, __HasSemantics);
    }

    get Parent()
    {
        return this.__getParentRow("PropertyMap", 1); // TODO: .Parent()
    }

    get Constant()
    {
        return this.__findConstant();
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __EventVisualizer extends __RowBase
{
    toString()
    {
        return this.Name;
    }

    get TableName()
    {
        return "Event";
    }

    // EventFlags

    get Name()
    {
        return this.__getString(1);
    }

    get EventType()
    {
        return this.__getCodedIndex(2, __TypeDefOrRef);
    }

    get MethodSemantic()
    {
        return this.__equalRange("MethodSemantics", 2, __HasSemantics);
    }

    get Parent()
    {
        return this.__getParentRow("EventMap", 1); // TODO: .Parent()
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __StandAloneSigVisualizer extends __RowBase
{
    get TableName()
    {
        return "StandAloneSig";
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __ModuleRefVisualizer extends __RowBase
{
    get TableName()
    {
        return "ModuleRef";
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __AssemblyVersion
{
    constructor(value)
    {
        // 64-bit value
        this.MajorVersion = value.bitwiseAnd(0xFFFF).asNumber();
        this.MinorVersion = value.bitwiseShiftRight(16).bitwiseAnd(0xFFFF).asNumber();
        this.BuildNumber = value.bitwiseShiftRight(32).bitwiseAnd(0xFFFF).asNumber();
        this.RevisionNumber = value.bitwiseShiftRight(48).bitwiseAnd(0xFFFF).asNumber();
    }

    toString()
    {
        return this.MajorVersion + "." + this.MinorVersion + "." + this.BuildNumber + "." + this.RevisionNumber;
    }
}

class __AssemblyVisualizer extends __RowBase
{
    toString()
    {
        return this.Name;
    }

    get TableName()
    {
        return "Assembly";
    }

    // HashAlgId

    get Version()
    {
        return new __AssemblyVersion(this.__getValue(1));
    }

    // Flags

    get PublicKey()
    {
        this.__getBlob(3);
    }

    get Name()
    {
        return this.__getString(4);
    }

    get Culture()
    {
        return this.__getString(5);
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __AssemblyRefVisualizer extends __RowBase
{
    toString()
    {
        return this.Name;
    }

    get TableName()
    {
        return "AssemblyRef";
    }

    get Version()
    {
        return new __AssemblyVersion(this.__getValue(1));
    }

    // Flags

    get PublicKeyOrToken()
    {
        return this.__getBlob(2);
    }

    get Name()
    {
        return this.__getString(3);
    }

    get Culture()
    {
        return this.__getString(4);
    }

    get HashValue()
    {
        return this.__getString(5);
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __FileVisualizer extends __RowBase
{
    get TableName()
    {
        return "File";
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __ExportedTypeVisualizer extends __RowBase
{
    get TableName()
    {
        return "ExportedType";
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __ManifestResourceVisualizer extends __RowBase
{
    get TableName()
    {
        return "ManifestResource";
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __GenericParamVisualizer extends __RowBase
{
    toString()
    {
        return this.Name;
    }

    get TableName()
    {
        return "GenericParam";
    }

    get Number()
    {
        return this.__getValue(0);
    }

    // Flags

    get Owner()
    {
        return this.__getCodedIndex(2, __TypeOrMethodDef);
    }

    get Name()
    {
        return this.__getString(3);
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __GenericParamConstraintVisualizer extends __RowBase
{
    get TableName()
    {
        return "GenericParamConstraint";
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __MethodSpecVisualizer extends __RowBase
{
    get TableName()
    {
        return "MethodSpec";
    }

    get CustomAttribute()
    {
        return this.__equalRange("CustomAttribute", 0, __HasCustomAttribute);
    }
}

class __MethodImplVisualizer extends __RowBase
{
    toString()
    {
        return this.MethodBody.toString();
    }

    get TableName()
    {
        return "MethodImpl";
    }

    get Class()
    {
        return this.__getTargetRow("TypeDef", 0);
    }

    get MethodBody()
    {
        return this.__getCodedIndex(1, __MethodDefOrRef);
    }

    get MethodDeclaration()
    {
        return this.__getCodedIndex(2, __MethodDefOrRef);
    }
}

class __ConstantVisualizer extends __RowBase
{
    toString()
    {
        return this.Type + "(" + this.Value + ")";
    }

    get TableName()
    {
        return "Constant";
    }

    get Type()
    {
        switch (this.__getValue(0))
        {
        case 2: return "Boolean";
        case 3: return "Char";
        case 4: return "Int8";
        case 5: return "UInt8";
        case 6: return "Int16";
        case 7: return "UInt16";
        case 8: return "Int32";
        case 9: return "UInt32";
        case 10: return "Int64";
        case 11: return "UInt64";
        case 12: return "Float32";
        case 13: return "Float64";
        case 14: return "String";
        case 18: return "Class";
        default: throw new Error("Unknown type: " + this.__getValue(0));
        }
    }

    get Parent()
    {
        return this.__getCodedIndex(1, __HasConstant);
    }

    get Value()
    {
        var blob = this.__getBlob(2);
        switch (this.__getValue(0))
        {
            case 2: return blob.ReadValue(1) == 1 ? true : false;
            case 3: return 'a' + blob.ReadValue(2) - 'a';
            case 4:
            case 5: return blob.ReadValue(1);
            case 6:
            case 7: return blob.ReadValue(2);
            case 8:
            case 9: return blob.ReadValue(4);
            case 10:
            case 11: return blob.ReadValue(8);
            case 12:
            case 13: return "Floating Point; not yet supported";
            case 14: return blob.ReadString();
            case 18: return "Class; not yet supported";
            default: throw new Error("Unknown type");
        }
    }
}

class __MethodSemanticsVisualizer extends __RowBase
{
    toString()
    {
        return this.Method.toString();
    }

    get TableName()
    {
        return "MethodSemantics";
    }

    // Semantic

    get Method()
    {
        return this.__getTargetRow("MethodDef", 1);
    }

    get Association()
    {
        return this.__getCodedIndex(2, __HasSemantics);
    }
}

class __PropertyMapVisualizer extends __RowBase
{
    get TableName()
    {
        return "PropertyMap";
    }

    get Parent()
    {
        return this.__getTargetRow("TypeDef", 0);
    }

    get PropertyList()
    {
        return this.__getList("Property", 1);
    }
}

class __EventMapVisualizer extends __RowBase
{
    get TableName()
    {
        return "EventMap";
    }

    get Parent()
    {
        return this.__getTargetRow("TypeDef", 0);
    }

    get EventList()
    {
        return this.__getList("Event", 1);
    }
}

// Blob Storage Types

var __ElementType = Object.freeze({ // ELEMENT_TYPE enum; one byte in size
    End: 0x00,
    Void: 0x01,
    Boolean: 0x02,
    Char: 0x03,
    I1: 0x04,
    U1: 0x05,
    I2: 0x06,
    U2: 0x07,
    I4: 0x08,
    U4: 0x09,
    I8: 0x0a,
    U8: 0x0b,
    R4: 0x0c,
    R8: 0x0d,
    String: 0x0e,
    Ptr: 0x0f,
    ByRef: 0x10,
    ValueType: 0x11,
    Class: 0x12,
    Var: 0x13,
    Array: 0x14,
    GenericInst: 0x15,
    TypedByRef: 0x16,
    I: 0x18,
    U: 0x19,
    FnPtr: 0x1b,
    Object: 0x1c,
    SZArray: 0x1d,
    MVar: 0x1e,
    CModReqd: 0x1f,
    CModOpt: 0x20,
    Internal: 0x21,
    Modifier: 0x40,
    Sentinel: 0x41,
    Pinned: 0x45,
    Type: 0x50,
    TaggedObject: 0x51,
    Field: 0x53,
    Property: 0x54,
    Enum: 0x55,
});
function __consumeElementType(stream)
{
    return stream.consumeValue(1);
}
function __peekElementType(stream)
{
    return stream.peekValue(1);
}

var __CallingConvention = Object.freeze({
    Default: 0x00,
    VarArg: 0x05,
    Field: 0x06,
    LocalSig: 0x07,
    Property: 0x08,
    GenericInst: 0x10,
    Mask: 0x0f,

    HasThis: 0x20,
    ExplicitThis: 0x40,
    Generic: 0x10,
});
function __consumeCallingConvention(stream)
{
    return stream.consumeValue(1);
}
function __callingConventionToString(value)
{
    // TODO: Others?
    return __enumToString(value & __CallingConvention.Mask, __CallingConvention);
}

function __consumeCodedIndex(stream, table, codedEnum)
{
    return table.__fromCodedIndex(stream.consumeUnsigned(), codedEnum);
}

function __consumeSzArray(stream)
{
    if (__peekElementType(stream) == __ElementType.SZArray)
    {
        stream.seek(1);
        return true;
    }

    return false;
}

class __CustomModSig
{
    constructor(table, stream)
    {
        this.__elementType = __consumeElementType(stream);
        if ((this.__elementType != __ElementType.CModOpt) || (this.__elementType != __ElementType.CModReqd))
        {
            throw new Error("Invalid CustomMod signature element type: " + __enumToString(this.__elementType, __ElementType));
        }

        this.__type = __consumeCodedIndex(stream, table, __TypeDefOrRef);
    }

    toString()
    {
        return this.Type.toString();
    }

    get Type()
    {
        return this.__type;
    }

    get ElementType()
    {
        return __enumToString(this.__elementType, __ElementType);
    }
}

function __consumeCustomMods(table, stream)
{
    var result = new Array();

    // Continue until we read something other than ELEMENT_TYPE_CMOD_OPT or ELEMENT_TYPE_CMOD_REQD
    for (var type = __peekElementType(stream); (type == __ElementType.CModOpt) || (type == __ElementType.CModReqd); type = __peekElementType(stream))
    {
        result.concat(new __CustomModSig(table, stream));
    }

    return result;
}

class __TypeSig
{
    constructor(table, stream)
    {
        this.__isSzArray = __consumeSzArray(stream);
        this.__customMods = __consumeCustomMods(table, stream);

        var type = __consumeElementType(stream);
        switch (type)
        {
        case __ElementType.Boolean:
        case __ElementType.Char:
        case __ElementType.I1:
        case __ElementType.U1:
        case __ElementType.I2:
        case __ElementType.U2:
        case __ElementType.I4:
        case __ElementType.U4:
        case __ElementType.I8:
        case __ElementType.U8:
        case __ElementType.R4:
        case __ElementType.R8:
        case __ElementType.String:
        case __ElementType.Object:
        case __ElementType.U:
        case __ElementType.I:
            this.__type = __enumToString(type, __ElementType);
            break;

        case __ElementType.Class:
        case __ElementType.ValueType:
            this.__type = __consumeCodedIndex(stream, table, __TypeDefOrRef);
            break;

        case __ElementType.GenericInst:
            this.__type = __GenericTypeInstSig(table, stream);
            break;

        case ElementType.Var:
            this.__type = __GenericTypeIndex(stream);
            break;

        default:
            throw new Error("Unknown or invalid ELEMENT_TYPE: " + type);
        }
    }

    toString()
    {
        return this.Type.toString() + (this.IsSzArray ? "[]" : "");
    }

    get Type()
    {
        return this.__type;
    }

    get IsSzArray()
    {
        return this.__isSzArray;
    }

    get CustomMod()
    {
        return this.__customMods;
    }
}

class __FieldSig
{
    constructor(table, stream)
    {
        this.__callingConvention = stream.consumeValue(1);
        if (this.__callingConvention & __CallingConvention.Mask != __CallingConvention.Field)
        {
            throw new Error("Invalid Field signature calling convention: " + this.__callingConvention);
        }

        this.__customMods = __consumeCustomMods(table, stream);
        this.__type = new __TypeSig(table, stream);
    }

    toString()
    {
        return this.Type.toString();
    }

    get Type()
    {
        return this.__type;
    }

    get CallingConvention()
    {
        return __callingConventionToString(this.__callingConvention);
    }

    get CustomMod()
    {
        return this.__customMods;
    }
}












class __GenericTypeIndex
{
    constructor(stream)
    {
        this.__index = stream.consumeUnsigned();
    }

    toString()
    {
        return "GenericTypeIndex: " + this.__index;
    }

    get Index()
    {
        return this.__index;
    }
}

class __TypeSpecSignature
{
    constructor(table, stream)
    {
        var type = stream.consumeValue(1);
        switch (type)
        {
        case __ElementType.Ptr:
            throw new Error("ELEMENT_TYPE_PTR is not currently supported");

        case __ElementType.FnPtr:
            throw new Error("ELEMENT_TYPE_FNPTR is not currently supported");

        case __ElementType.Array:
            throw new Error("ELEMENT_TYPE_ARRAY is not currently supported");

        case __ElementType.SZArray:
            throw new Error("ELEMENT_TYPE_SZARRAY is not currently supported");

        case __ElementType.GenericInst:
            this.__genericInst = new __GenericTypeInstSig(table, stream);
            Object.defineProperty(this, "GenericTypeInstSig", {
                get: function()
                {
                    return this.__genericInst;
                }
            });
            break;

        default:
            throw new Error("Unexpected or unknown ELEMENT_TYPE: " + type);
        }
    }

    toString()
    {
        if (this.__genericInst)
        {
            return this.__genericInst.toString();
        }
    }
}

class __GenericTypeInstSig // TODO
{
    constructor(table, stream)
    {
        this.__type = stream.consumeValue(1);
        if ((this.__type != __ElementType.Class) && (this.__type != __ElementType.ValueType))
        {
            throw new Error("Generic type instance signatures must begin with either ELEMENT_TYPE_CLASS or ELEMENT_TYPE_VALUE");
        }

        var typeDefOrRef = stream.ConsumeValue(4);
    }

    get ElementType()
    {
        return this.__type;
    }

    get GenericType()
    {

    }
}
