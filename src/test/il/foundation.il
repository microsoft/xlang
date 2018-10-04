.assembly windowsruntime foundation
{
  .hash algorithm 0x00008004
  .ver 255:255:255:255
}

.class public auto ansi windowsruntime sealed Windows.Foundation.Metadata.GuidAttribute
       extends [mscorlib]System.Attribute
{
  .method public hidebysig specialname rtspecialname
          instance void  .ctor([in] uint32 a,
                               [in] uint16 b,
                               [in] uint16 c,
                               [in] uint8 d,
                               [in] uint8 e,
                               [in] uint8 f,
                               [in] uint8 g,
                               [in] uint8 h,
                               [in] uint8 i,
                               [in] uint8 j,
                               [in] uint8 k) runtime managed
  {
  }
}

.class public auto ansi windowsruntime sealed Windows.Foundation.PropertyType
       extends [mscorlib]System.Enum
{
  .field private specialname rtspecialname int32 value__
  .field public static literal valuetype Windows.Foundation.PropertyType Empty = int32(0x00000000)
  .field public static literal valuetype Windows.Foundation.PropertyType UInt8 = int32(0x00000001)
  .field public static literal valuetype Windows.Foundation.PropertyType Int16 = int32(0x00000002)
  .field public static literal valuetype Windows.Foundation.PropertyType UInt16 = int32(0x00000003)
  .field public static literal valuetype Windows.Foundation.PropertyType Int32 = int32(0x00000004)
  .field public static literal valuetype Windows.Foundation.PropertyType UInt32 = int32(0x00000005)
  .field public static literal valuetype Windows.Foundation.PropertyType Int64 = int32(0x00000006)
  .field public static literal valuetype Windows.Foundation.PropertyType UInt64 = int32(0x00000007)
  .field public static literal valuetype Windows.Foundation.PropertyType Single = int32(0x00000008)
  .field public static literal valuetype Windows.Foundation.PropertyType Double = int32(0x00000009)
  .field public static literal valuetype Windows.Foundation.PropertyType Char16 = int32(0x0000000A)
  .field public static literal valuetype Windows.Foundation.PropertyType Boolean = int32(0x0000000B)
  .field public static literal valuetype Windows.Foundation.PropertyType String = int32(0x0000000C)
  .field public static literal valuetype Windows.Foundation.PropertyType Inspectable = int32(0x0000000D)
  .field public static literal valuetype Windows.Foundation.PropertyType DateTime = int32(0x0000000E)
  .field public static literal valuetype Windows.Foundation.PropertyType TimeSpan = int32(0x0000000F)
  .field public static literal valuetype Windows.Foundation.PropertyType Guid = int32(0x00000010)
  .field public static literal valuetype Windows.Foundation.PropertyType Point = int32(0x00000011)
  .field public static literal valuetype Windows.Foundation.PropertyType Size = int32(0x00000012)
  .field public static literal valuetype Windows.Foundation.PropertyType Rect = int32(0x00000013)
  .field public static literal valuetype Windows.Foundation.PropertyType OtherType = int32(0x00000014)
  .field public static literal valuetype Windows.Foundation.PropertyType UInt8Array = int32(0x00000401)
  .field public static literal valuetype Windows.Foundation.PropertyType Int16Array = int32(0x00000402)
  .field public static literal valuetype Windows.Foundation.PropertyType UInt16Array = int32(0x00000403)
  .field public static literal valuetype Windows.Foundation.PropertyType Int32Array = int32(0x00000404)
  .field public static literal valuetype Windows.Foundation.PropertyType UInt32Array = int32(0x00000405)
  .field public static literal valuetype Windows.Foundation.PropertyType Int64Array = int32(0x00000406)
  .field public static literal valuetype Windows.Foundation.PropertyType UInt64Array = int32(0x00000407)
  .field public static literal valuetype Windows.Foundation.PropertyType SingleArray = int32(0x00000408)
  .field public static literal valuetype Windows.Foundation.PropertyType DoubleArray = int32(0x00000409)
  .field public static literal valuetype Windows.Foundation.PropertyType Char16Array = int32(0x0000040A)
  .field public static literal valuetype Windows.Foundation.PropertyType BooleanArray = int32(0x0000040B)
  .field public static literal valuetype Windows.Foundation.PropertyType StringArray = int32(0x0000040C)
  .field public static literal valuetype Windows.Foundation.PropertyType InspectableArray = int32(0x0000040D)
  .field public static literal valuetype Windows.Foundation.PropertyType DateTimeArray = int32(0x0000040E)
  .field public static literal valuetype Windows.Foundation.PropertyType TimeSpanArray = int32(0x0000040F)
  .field public static literal valuetype Windows.Foundation.PropertyType GuidArray = int32(0x00000410)
  .field public static literal valuetype Windows.Foundation.PropertyType PointArray = int32(0x00000411)
  .field public static literal valuetype Windows.Foundation.PropertyType SizeArray = int32(0x00000412)
  .field public static literal valuetype Windows.Foundation.PropertyType RectArray = int32(0x00000413)
  .field public static literal valuetype Windows.Foundation.PropertyType OtherTypeArray = int32(0x00000414)
}

.class interface public abstract auto ansi windowsruntime Windows.Foundation.IPropertyValue
{
  .custom instance void Windows.Foundation.Metadata.GuidAttribute::.ctor(uint32,
                                                                         uint16,
                                                                         uint16,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8) = ( 01 00 DD 82 D6 4B 54 75 E9 40 9A 9B 82 65 4E DE
                                                                                    7E 62 00 00 )
  .method public hidebysig newslot specialname abstract virtual
          instance valuetype Windows.Foundation.PropertyType
          get_Type() cil managed
  {
  }
  .method public hidebysig newslot specialname abstract virtual
          instance bool  get_IsNumericScalar() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance uint8  GetUInt8() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance int16  GetInt16() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance uint16  GetUInt16() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance int32  GetInt32() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance uint32  GetUInt32() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance int64  GetInt64() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance uint64  GetUInt64() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance float32  GetSingle() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance float64  GetDouble() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance char  GetChar16() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance bool  GetBoolean() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance string  GetString() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance valuetype [mscorlib]System.Guid
          GetGuid() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance valuetype Windows.Foundation.DateTime
          GetDateTime() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance valuetype Windows.Foundation.TimeSpan
          GetTimeSpan() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance valuetype Windows.Foundation.Point
          GetPoint() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance valuetype Windows.Foundation.Size
          GetSize() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance valuetype Windows.Foundation.Rect
          GetRect() cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetUInt8Array([out] uint8[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetInt16Array([out] int16[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetUInt16Array([out] uint16[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetInt32Array([out] int32[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetUInt32Array([out] uint32[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetInt64Array([out] int64[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetUInt64Array([out] uint64[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetSingleArray([out] float32[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetDoubleArray([out] float64[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetChar16Array([out] char[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetBooleanArray([out] bool[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetStringArray([out] string[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetInspectableArray([out] object[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetGuidArray([out] valuetype [mscorlib]System.Guid[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetDateTimeArray([out] valuetype Windows.Foundation.DateTime[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetTimeSpanArray([out] valuetype Windows.Foundation.TimeSpan[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetPointArray([out] valuetype Windows.Foundation.Point[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetSizeArray([out] valuetype Windows.Foundation.Size[]& 'value') cil managed
  {
  }
  .method public hidebysig newslot abstract virtual
          instance void  GetRectArray([out] valuetype Windows.Foundation.Rect[]& 'value') cil managed
  {
  }
  .property instance bool IsNumericScalar()
  {
    .get instance bool Windows.Foundation.IPropertyValue::get_IsNumericScalar()
  }
  .property instance valuetype Windows.Foundation.PropertyType
          Type()
  {
    .get instance valuetype Windows.Foundation.PropertyType Windows.Foundation.IPropertyValue::get_Type()
  }
}

.class public sequential ansi windowsruntime sealed Windows.Foundation.DateTime
       extends [mscorlib]System.ValueType
{
  .field public int64 UniversalTime
}

.class public sequential ansi windowsruntime sealed Windows.Foundation.TimeSpan
       extends [mscorlib]System.ValueType
{
  .field public int64 Duration
}

.class public sequential ansi windowsruntime sealed Windows.Foundation.Point
       extends [mscorlib]System.ValueType
{
  .field public float32 X
  .field public float32 Y
}

.class public sequential ansi windowsruntime sealed Windows.Foundation.Size
       extends [mscorlib]System.ValueType
{
  .field public float32 Width
  .field public float32 Height
}

.class public sequential ansi windowsruntime sealed Windows.Foundation.Rect
       extends [mscorlib]System.ValueType
{
  .field public float32 X
  .field public float32 Y
  .field public float32 Width
  .field public float32 Height
}

.class public abstract auto ansi windowsruntime sealed Windows.Foundation.PropertyValue
       extends [mscorlib]System.Object
{
  .custom instance void Windows.Foundation.Metadata.StaticAttribute::.ctor(class [mscorlib]System.Type,
                                                                           uint32,
                                                                           string) = ( 01 00 28 57 69 6E 64 6F 77 73 2E 46 6F 75 6E 64
                                                                                       61 74 69 6F 6E 2E 49 50 72 6F 70 65 72 74 79 56
                                                                                       61 6C 75 65 53 74 61 74 69 63 73 00 00 01 00 25
                                                                                       57 69 6E 64 6F 77 73 2E 46 6F 75 6E 64 61 74 69
                                                                                       6F 6E 2E 46 6F 75 6E 64 61 74 69 6F 6E 43 6F 6E
                                                                                       74 72 61 63 74 00 00 )

  .method public hidebysig static object
          CreateEmpty() runtime managed
  {
  }
  .method public hidebysig static object
          CreateUInt8([in] uint8 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateInt16([in] int16 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateUInt16([in] uint16 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateInt32([in] int32 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateUInt32([in] uint32 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateInt64([in] int64 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateUInt64([in] uint64 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateSingle([in] float32 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateDouble([in] float64 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateChar16([in] char 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateBoolean([in] bool 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateString([in] string 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateInspectable([in] object 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateGuid([in] valuetype [mscorlib]System.Guid 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateDateTime([in] valuetype Windows.Foundation.DateTime 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateTimeSpan([in] valuetype Windows.Foundation.TimeSpan 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreatePoint([in] valuetype Windows.Foundation.Point 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateSize([in] valuetype Windows.Foundation.Size 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateRect([in] valuetype Windows.Foundation.Rect 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateUInt8Array([in] uint8[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateInt16Array([in] int16[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateUInt16Array([in] uint16[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateInt32Array([in] int32[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateUInt32Array([in] uint32[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateInt64Array([in] int64[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateUInt64Array([in] uint64[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateSingleArray([in] float32[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateDoubleArray([in] float64[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateChar16Array([in] char[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateBooleanArray([in] bool[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateStringArray([in] string[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateInspectableArray([in] object[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateGuidArray([in] valuetype [mscorlib]System.Guid[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateDateTimeArray([in] valuetype Windows.Foundation.DateTime[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateTimeSpanArray([in] valuetype Windows.Foundation.TimeSpan[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreatePointArray([in] valuetype Windows.Foundation.Point[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateSizeArray([in] valuetype Windows.Foundation.Size[] 'value') runtime managed
  {
  }
  .method public hidebysig static object
          CreateRectArray([in] valuetype Windows.Foundation.Rect[] 'value') runtime managed
  {
  }
}

.class public auto ansi windowsruntime sealed Windows.Foundation.Metadata.StaticAttribute
       extends [mscorlib]System.Attribute
{
  .method public hidebysig specialname rtspecialname
          instance void  .ctor([in] class [mscorlib]System.Type 'type',
                               [in] uint32 version) runtime managed
  {
  }
  .method public hidebysig specialname rtspecialname
          instance void  .ctor([in] class [mscorlib]System.Type 'type',
                               [in] uint32 version,
                               [in] string contractName) runtime managed
  {
  }
}

.class interface private abstract auto ansi windowsruntime Windows.Foundation.IPropertyValueStatics
{
  .custom instance void Windows.Foundation.Metadata.GuidAttribute::.ctor(uint32,
                                                                         uint16,
                                                                         uint16,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8,
                                                                         uint8) = ( 01 00 C8 DB 9B 62 32 D9 F4 4F 96 B9 8D 96 C5 C1
                                                                                    E8 58 00 00 )
  .method public hidebysig newslot abstract virtual 
          instance object  CreateEmpty() cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateUInt8([in] uint8 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateInt16([in] int16 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateUInt16([in] uint16 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateInt32([in] int32 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateUInt32([in] uint32 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateInt64([in] int64 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateUInt64([in] uint64 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateSingle([in] float32 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateDouble([in] float64 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateChar16([in] char 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateBoolean([in] bool 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateString([in] string 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateInspectable([in] object 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateGuid([in] valuetype [mscorlib]System.Guid 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateDateTime([in] valuetype Windows.Foundation.DateTime 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateTimeSpan([in] valuetype Windows.Foundation.TimeSpan 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreatePoint([in] valuetype Windows.Foundation.Point 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateSize([in] valuetype Windows.Foundation.Size 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateRect([in] valuetype Windows.Foundation.Rect 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateUInt8Array([in] uint8[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateInt16Array([in] int16[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateUInt16Array([in] uint16[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateInt32Array([in] int32[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateUInt32Array([in] uint32[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateInt64Array([in] int64[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateUInt64Array([in] uint64[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateSingleArray([in] float32[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateDoubleArray([in] float64[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateChar16Array([in] char[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateBooleanArray([in] bool[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateStringArray([in] string[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateInspectableArray([in] object[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateGuidArray([in] valuetype [mscorlib]System.Guid[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateDateTimeArray([in] valuetype Windows.Foundation.DateTime[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateTimeSpanArray([in] valuetype Windows.Foundation.TimeSpan[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreatePointArray([in] valuetype Windows.Foundation.Point[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateSizeArray([in] valuetype Windows.Foundation.Size[] 'value') cil managed
  {
  }

  .method public hidebysig newslot abstract virtual 
          instance object  CreateRectArray([in] valuetype Windows.Foundation.Rect[] 'value') cil managed
  {
  }
}
