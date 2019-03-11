
static PyObject* _from_@(PyObject* /*unused*/, PyObject* arg)
{
    return py::trycatch_invoker([=]() -> PyObject* {
        auto return_value = py::convert_to<winrt::Windows::Foundation::IInspectable>(arg);
        return py::convert(return_value.as<%>());
    });
}
