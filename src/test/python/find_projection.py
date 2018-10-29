from inspect import currentframe, getframeinfo
from pathlib import Path
import sys

filename = getframeinfo(currentframe()).filename
test_root = Path(filename).resolve().parent

vi = sys.version_info
dirname = "lib.{2}-{0}.{1}".format(vi.major, vi.minor, "win-amd64" if sys.maxsize > 2**32 else "win32")

test_module_path = test_root / ("output/build/" + dirname)
sys.path.append(str(test_module_path))

def import_winrt_ns(ns):
    import _pyrt

    module_name = "_pyrt_" + ns.replace('.', '_')

    import importlib.machinery
    import importlib.util
    loader = importlib.machinery.ExtensionFileLoader(module_name, _pyrt.__file__)
    spec = importlib.util.spec_from_loader(module_name, loader)
    module = importlib.util.module_from_spec(spec)
    loader.exec_module(module)
    return module