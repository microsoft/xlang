winrt::Windows::Foundation::DateTime py::converter<winrt::Windows::Foundation::DateTime>::convert_to(PyObject* obj)
{
    if (!PyDict_Check(obj)) { throw winrt::hresult_invalid_argument(); }

    PyObject* pyUniversalTime = PyDict_GetItemString(obj, "UniversalTime");
    if (!pyUniversalTime) { throw winrt::hresult_invalid_argument(); }
    auto universal_time = converter<int64_t>::convert_to(pyUniversalTime);
    return winrt::Windows::Foundation::DateTime{ winrt::Windows::Foundation::TimeSpan{ universal_time } };
}

PyObject* DateTime_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    auto tuple_size = PyTuple_Size(args);
    if ((tuple_size == 0) && (kwds == nullptr))
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

    if ((tuple_size == 1) && (kwds == nullptr))
    {
        auto arg = PyTuple_GetItem(args, 0);
        if (PyDict_Check(arg))
        {
            try
            {
                auto instance = py::converter<winrt::Windows::Foundation::DateTime>::convert_to(arg);
                return py::wrap_struct(instance, type);
            }
            catch (...)
            {
                return py::to_PyErr();
            }
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