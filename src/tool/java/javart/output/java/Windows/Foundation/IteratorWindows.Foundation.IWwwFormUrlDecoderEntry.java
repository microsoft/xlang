package Windows.Foundation;

import java.util.Iterator;

public class IteratorWindows.Foundation.IWwwFormUrlDecoderEntry extends Inspectable implements Iterator<Windows.Foundation.IWwwFormUrlDecoderEntry> {

	public IteratorWindows.Foundation.IWwwFormUrlDecoderEntry(long abi) {
		super(abi);
	}
	
	public IteratorWindows.Foundation.IWwwFormUrlDecoderEntry(Inspectable that) {
		super(that);
	}

	@Override
	public boolean hasNext() {
		return abi_hasNext(abi);
	}

	@Override
	public Windows.Foundation.IWwwFormUrlDecoderEntry next() {
		return abi_next(abi);
	}

	// Implementation ...
    private native boolean abi_hasNext(long abi);
    private native Windows.Foundation.IWwwFormUrlDecoderEntry abi_next(long abi);
	
	private static native void Register();

	static {
		System.loadLibrary("Windows.Foundation");
		Register();
	}
}