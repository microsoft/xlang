import sys
sys.path.append("./generated")

import _pyrt
_pyrt.init_apartment()

def import_ns(ns):
    import importlib.machinery
    import importlib.util
    try:
        module_name = "_pyrt_" + ns.replace('.', '_')
        loader = importlib.machinery.ExtensionFileLoader(module_name, _pyrt.__file__)
        spec = importlib.util.spec_from_loader(module_name, loader)
        module = importlib.util.module_from_spec(spec)
        loader.exec_module(module)
        return module
    except:
        return None
