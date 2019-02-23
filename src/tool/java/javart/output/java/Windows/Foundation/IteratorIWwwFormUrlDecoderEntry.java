package Windows.Foundation;

import java.util.Iterator;

public class IteratorIWwwFormUrlDecoderEntry extends Inspectable implements Iterator<IWwwFormUrlDecoderEntry> {

	public IteratorIWwwFormUrlDecoderEntry(long abi) {
		super(abi);
	}
	
	public IteratorIWwwFormUrlDecoderEntry(Inspectable that) {
		super(that);
	}

	@Override
	public boolean hasNext() {
		return abi_hasNext(abi);
	}

	@Override
	public IWwwFormUrlDecoderEntry next() {
		return abi_next(abi);
	}

	// Implementation ...
    private native boolean abi_hasNext(long abi);
    private native IWwwFormUrlDecoderEntry abi_next(long abi);
	
	private static native void Register();

	static {
		System.loadLibrary("Windows.Foundation");
		Register();
	}
}