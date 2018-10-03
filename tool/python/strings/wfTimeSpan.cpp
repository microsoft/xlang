winrt::Windows::Foundation::TimeSpan py::converter<winrt::Windows::Foundation::TimeSpan>::convert_to(PyObject* obj)
{
    if (!PyDict_Check(obj)) { throw winrt::hresult_invalid_argument(); }

    PyObject* pyDuration = PyDict_GetItemString(obj, "Duration");
    if (!pyDuration) { throw winrt::hresult_invalid_argument(); }
    auto duration = converter<int64_t>::convert_to(pyDuration);
    return winrt::Windows::Foundation::TimeSpan{ duration };
}

PyObject* TimeSpan_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    if ((PyTuple_Size(args) == 0) && (kwds == nullptr))
    {
        try
        {
            winrt::Windows::Foundation::TimeSpan instance{};
            return py::wrap_struct(instance, type);
        }
        catch (...)
        {
            return py::to_PyErr();
        }
    }

    int64_t _Duration{};
    static char* kwlist[] = { "Duration", nullptr };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "L", kwlist, &_Duration))
    {
        return nullptr;
    }

    try
    {
        winrt::Windows::Foundation::TimeSpan instance{ _Duration };
        return py::wrap_struct(instance, type);
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}

static PyObject* TimeSpan_get_Duration(py::winrt_struct_wrapper<winrt::Windows::Foundation::TimeSpan>* self, void* /*unused*/)
{
    try
    {
        return py::convert(self->obj.count());
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}

static int TimeSpan_set_Duration(py::winrt_struct_wrapper<winrt::Windows::Foundation::TimeSpan>* self, PyObject* value, void* /*unused*/)
{
    if (value == nullptr)
    {
        PyErr_SetString(PyExc_RuntimeError, "property delete not supported");
        return -1;
    }

    try
    {
        self->obj = winrt::Windows::Foundation::TimeSpan{ py::convert_to<int64_t>(value) };
        return 0;
    }
    catch (...)
    {
        return -1;
    }
}
