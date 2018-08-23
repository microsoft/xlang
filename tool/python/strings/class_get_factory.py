^@classmethod
def __get_factory(cls, iid):
    try:
        factory = cls.__factory
    except AttributeError:
        factory = cls.__factory = _rt.get_activation_factory('%.%', iid)
    return _rt.querycontext(factory, iid)

