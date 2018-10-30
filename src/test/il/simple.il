.assembly extern windowsruntime foundation
{
    .ver 255:255:255:255
}

.class interface public abstract auto ansi windowsruntime Simple.IStringable
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
                                                                                       uint8) = ( 01 00 DA EC CC 98 80 62 D5 41 96 EE E0 5F 3D 62
                                                                                                  F5 AE 00 00 )
    .method public hidebysig newslot abstract virtual instance string  ToString() cil managed
    {
    }
}
