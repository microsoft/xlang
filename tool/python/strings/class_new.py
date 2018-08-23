def __new__(cls, interface, *args, **kwargs):
    if not isinstance(interface, _ct.c_void_p):
        raise Exception("Invalid interface type")
    return super().__new__(cls, *args, **kwargs)

