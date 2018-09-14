
static PyObject* init_apartment(PyObject* /*unused*/, PyObject* /*unused*/)
{
    winrt::init_apartment();
    Py_RETURN_NONE;
}

static PyObject* uninit_apartment(PyObject* /*unused*/, PyObject* /*unused*/)
{
    winrt::uninit_apartment();
    Py_RETURN_NONE;
}

static PyMethodDef module_methods[]{
    { "init_apartment", init_apartment, METH_NOARGS, "initialize the apartment" },
    { "uninit_apartment", uninit_apartment, METH_NOARGS, "uninitialize the apartment" },
    { nullptr }
};
