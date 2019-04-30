from . import _%
import enum

class ApartmentType(enum.IntEnum):
    SINGLE_THREADED = 0
    MULTI_THREADED = 1

init_apartment = _%.init_apartment
uninit_apartment = _%.uninit_apartment

def _import_ns_module(ns):
    import importlib.machinery
    import importlib.util

    try:
        module_name = "_%_" + ns.replace('.', '_')

        loader = importlib.machinery.ExtensionFileLoader(module_name, _%.__file__)
        spec = importlib.util.spec_from_loader(module_name, loader)
        module = importlib.util.module_from_spec(spec)
        loader.exec_module(module)
        return module
    except:
        return None
