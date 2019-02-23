package com.microsoft.windows.foundation;

import java.util.Iterator;

public class WwwFormUrlDecoder extends Inspectable implements java.lang.Iterable<WwwFormUrlDecoderEntry>, Windows.Foundation.Collections.IVectorView<Windows.Foundation.IWwwFormUrlDecoderEntry>
{
    public WwwFormUrlDecoder(long abi) {
        super(abi);
    }

    public WwwFormUrlDecoder(WwwFormUrlDecoder that) {
        super(that);
    }

    public WwwFormUrlDecoder(String query) {
        this(ConstructG(query));
    }



    @Override
    public boolean equals(Object arg0) {
        if (arg0 instanceof WwwFormUrlDecoder) {
            return super.equals(arg0);
        }
        return false;
    }

    public String getFirstValueByName(String name) {
        return abi_GetFirstValueByNameG(abi, name);
    }

    public Windows.Foundation.Collections.IIterator<Windows.Foundation.IWwwFormUrlDecoderEntry> first() {
        return abi_First(abi);
    }

    public Windows.Foundation.IWwwFormUrlDecoderEntry getAt(int index) {
        return abi_GetAtI(abi, index);
    }

    public int getSize() {
        return abi_get_Size(abi);
    }

    public boolean indexOf(Windows.Foundation.IWwwFormUrlDecoderEntry value, int index) {
        return abi_IndexOfWindows.Foundation.IWwwFormUrlDecoderEntryI(abi, value, index);
    }

    public int getMany(int startIndex, Windows.Foundation.IWwwFormUrlDecoderEntry items) {
        return abi_GetManyIWindows.Foundation.IWwwFormUrlDecoderEntry(abi, startIndex, items);
    }



    // Implementation ...
    private static native void abi_Construct(String query);

    private native String abi_GetFirstValueByName(long abi, String name);

    private native Windows.Foundation.Collections.IIterator<Windows.Foundation.IWwwFormUrlDecoderEntry> abi_First(long abi, );

    private native Windows.Foundation.IWwwFormUrlDecoderEntry abi_GetAt(long abi, int index);

    private native int abi_get_Size(long abi, );

    private native boolean abi_IndexOf(long abi, Windows.Foundation.IWwwFormUrlDecoderEntry value, int index);

    private native int abi_GetMany(long abi, int startIndex, Windows.Foundation.IWwwFormUrlDecoderEntry items);


    private static native void Register();

    static {
        System.loadLibrary("com.microsoft.windows.foundation");
        Register();
    }
};
        