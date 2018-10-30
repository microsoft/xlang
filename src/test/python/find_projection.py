from inspect import currentframe, getframeinfo
from pathlib import Path
import sys

script_filename = getframeinfo(currentframe()).filename
generated_path = (Path(script_filename).resolve().parent) / "generated"

sys.path.append(str(generated_path))
sys.path.append(str(generated_path / "build"))
