.assembly extern windowsruntime foundation
{
    .ver 255:255:255:255
}

.class interface private abstract auto ansi windowsruntime Component.IClass
{
    .custom instance void [foundation]Windows.Foundation.Metadata.GuidAttribute::.ctor(uint32,
                                                                                       uint16,
                                                                                       uint16,
                                                                                       uint8,
                                                                                       uint8,
                                                                                       uint8,
                                                                                       uint8,
                                                                                       uint8,
                                                                                       uint8,
                                                                                       uint8,
                                                                                       uint8) = ( 01 00 3C E1 37 C5 C3 9F E5 50 B6 17 BF ED A4 51
                                                                                                  E5 BD 00 00 )

    .method public hidebysig newslot abstract virtual instance string  GetValue() cil managed
    {
    }
}

.class public auto ansi windowsruntime sealed Component.Class
    extends [mscorlib]System.Object
    implements Component.IClass
{
    .custom instance void [foundation]Windows.Foundation.Metadata.ActivatableAttribute::.ctor(uint32) = ( 01 00 01 00 00 00 00 00 ) 
    .interfaceimpl type Component.IClass
    .custom instance void [foundation]Windows.Foundation.Metadata.DefaultAttribute::.ctor() = ( 01 00 00 00 ) 
}
