PyObject* DateTime_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    if ((PyTuple_Size(args) == 0) && (kwds == nullptr))
    {
        try
        {
            winrt::Windows::Foundation::DateTime instance{};
            return py::wrap_struct(instance, type);
        }
        catch (...)
        {
            return py::to_PyErr();
        }
    }

    int64_t _UniversalTime{};
    static char* kwlist[] = { "UniversalTime", nullptr };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "L", kwlist, &_UniversalTime))
    {
        return nullptr;
    }

    try
    {
        winrt::Windows::Foundation::DateTime instance{ winrt::Windows::Foundation::TimeSpan{ _UniversalTime } };
        return py::wrap_struct(instance, type);
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}

static PyObject* DateTime_get_UniversalTime(py::winrt_struct_wrapper<winrt::Windows::Foundation::DateTime>* self, void* /*unused*/)
{
    try
    {
        return py::convert(self->obj.time_since_epoch().count());
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}

static int DateTime_set_UniversalTime(py::winrt_struct_wrapper<winrt::Windows::Foundation::DateTime>* self, PyObject* value, void* /*unused*/)
{
    if (value == nullptr)
    {
        PyErr_SetString(PyExc_RuntimeError, "property delete not supported");
        return -1;
    }

    try
    {
        self->obj = winrt::Windows::Foundation::DateTime{ winrt::Windows::Foundation::TimeSpan{ py::convert_to<int64_t>(value) } };
        return 0;
    }
    catch (...)
    {
        return -1;
    }
}