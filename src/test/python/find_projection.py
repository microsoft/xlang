from inspect import currentframe, getframeinfo
from pathlib import Path
import sys

filename = getframeinfo(currentframe()).filename
test_root = Path(filename).resolve().parent

vi = sys.version_info
dirname = "lib.{2}-{0}.{1}".format(vi.major, vi.minor, "win-amd64" if sys.maxsize > 2**32 else "win32")

test_module_path = test_root / ("output/build/" + dirname)
sys.path.append(str(test_module_path))

