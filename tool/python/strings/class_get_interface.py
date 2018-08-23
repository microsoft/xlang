def __get_interface(self, iid = None):
    if (iid == None):
        return _ctx.nullcontext(self.__interface)
    else:
        return _rt.querycontext(self.__interface, iid)

