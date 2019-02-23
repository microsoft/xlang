package com.microsoft.windows.foundation;

import java.util.Iterator;

public class WwwFormUrlDecoderEntry extends Inspectable implements Windows.Foundation.IWwwFormUrlDecoderEntry
{
    public WwwFormUrlDecoderEntry(long abi) {
        super(abi);
    }

    public WwwFormUrlDecoderEntry(WwwFormUrlDecoderEntry that) {
        super(that);
    }



    @Override
    public boolean equals(Object arg0) {
        if (arg0 instanceof WwwFormUrlDecoderEntry) {
            return super.equals(arg0);
        }
        return false;
    }

    public String getName() {
        return abi_get_Name(abi);
    }

    public String getValue() {
        return abi_get_Value(abi);
    }



    // Implementation ...
    private native String abi_get_Name(long abi, );

    private native String abi_get_Value(long abi, );


    private static native void Register();

    static {
        System.loadLibrary("com.microsoft.windows.foundation");
        Register();
    }
};
        