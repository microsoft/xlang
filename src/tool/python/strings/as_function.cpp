static PyObject* _as_@(PyObject* /*unused*/, PyObject* arg)
{
    try
    {
        auto return_value = py::convert_to<winrt::Windows::Foundation::IInspectable>(arg);
        return py::convert(return_value.as<%>());
    }
    catch (...)
    {
        return py::to_PyErr();
    }
}