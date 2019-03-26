if (_obj.HasCurrent())
{
    auto cur = _obj.Current();
    _obj.MoveNext();
    return py::convert(cur);
}
else
{
    return nullptr;
}
