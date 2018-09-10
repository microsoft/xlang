
namespace py
{
    PyObject* to_PyErr() noexcept
    {
        try
        {
            throw;
        }
        catch (winrt::hresult_error const& e)
        {
            PyErr_SetString(PyExc_RuntimeError, winrt::to_string(e.message()).c_str());
        }
        catch (std::bad_alloc const&)
        {
            return PyErr_NoMemory();
        }
        catch (std::out_of_range const& e)
        {
            PyErr_SetString(PyExc_IndexError, e.what());
        }
        catch (std::invalid_argument const& e)
        {
            PyErr_SetString(PyExc_TypeError, e.what());
        }
        catch (std::exception const& e)
        {
            PyErr_SetString(PyExc_RuntimeError, e.what());
        }

        return nullptr;
    }

    template <typename T>
    struct py_type
    {
        using type = T;
    };

    template <typename T>
    using py_type_t = typename py_type<T>::type;

    template<typename T>
    PyObject* wrap(T instance)
    {
        auto pyinstance = PyObject_New(py_type_t<T>, reinterpret_cast<PyTypeObject *>(py_type_t<T>::python_type));

        if (!pyinstance) 
        { 
            return nullptr; 
        }

        pyinstance->obj = instance;
        return reinterpret_cast<PyObject*>(pyinstance);
    }

    template<typename T>
    T unwrap(PyObject* arg)
    {
        if (arg == nullptr) 
        { 
            throw winrt::hresult_invalid_argument(); 
        }

        // for runtime classes, verify the python type passed in matches the python type of the runtime class
        if constexpr (std::is_same_v<winrt::impl::category_t<T>, winrt::impl::class_category>)
        {
            if (!PyObject_TypeCheck(arg, reinterpret_cast<PyTypeObject *>(py_type_t<T>::python_type)))
            {
                PyErr_SetString(PyExc_RuntimeError, "incorrect type");
                return nullptr;
            }
        }

        // TODO: introduce a python base class for WinRT interface wrappers so that they can be detected 
        //       independently of their specific runtime class type as per above. If the passed in Python 
        //       object is a winrt class or interface, call QI to retrieve the requested interface and return 
        //       if successful.

        return reinterpret_cast<py_type_t<T> *>(arg)->obj;
    }

    template<typename T>
    T unwrap(PyObject* args, int index)
    {
        return unwrap<T>(PyTuple_GetItem(args, index));
    }

    template <typename T>
    struct converter
    {
        static PyObject* convert_to(T value) noexcept
        {
            PyErr_SetNone(PyExc_NotImplementedError);
            return nullptr;
        }

        static T convert_from(PyObject* obj)
        {
            winrt::hresult_not_implemented;
        }
    };

    template <>
    struct converter<bool>
    {
        static PyObject* convert_to(bool value) noexcept
        {
            return PyBool_FromLong(value ? 1 : 0);
        }

        static bool convert_from(PyObject* obj)
        {
            auto result = PyObject_IsTrue(obj);

            if (result == -1) 
            { 
                throw winrt::hresult_invalid_argument();
            }

            return result > 0;
        }
    };

    template <>
    struct converter<int32_t>
    {
        static PyObject* convert_to(int32_t value) noexcept
        {
            return PyLong_FromLong(value);
        }

        static int32_t convert_from(PyObject* obj)
        {
            auto result = PyLong_AsLong(obj);

            if (result == -1 && PyErr_Occurred())
            {
                throw winrt::hresult_invalid_argument();
            }

            return result;
        }
    };

    template <>
    struct converter<uint32_t>
    {
        static PyObject* convert_to(uint32_t value) noexcept
        {
            return PyLong_FromUnsignedLong(value);
        }

        static uint32_t convert_from(PyObject* obj)
        {
            auto result = PyLong_AsUnsignedLong(obj);

            if (result == -1 && PyErr_Occurred())
            {
                throw winrt::hresult_invalid_argument();
            }

            return result;
        }
    };

    template <>
    struct converter<double>
    {
        static PyObject* convert_to(double value) noexcept
        {
            return PyFloat_FromDouble(value);
        }

        static double convert_from(PyObject* obj)
        {
            auto result = PyFloat_AsDouble(obj);

            if (result == -1 && PyErr_Occurred())
            {
                throw winrt::hresult_invalid_argument();
            }

            return result;
        }
    };

    template <>
    struct converter<winrt::hstring>
    {
        static PyObject* convert_to(winrt::hstring value) noexcept
        {
            return PyUnicode_FromWideChar(value.c_str(), value.size());
        }

        static winrt::hstring convert_from(PyObject* obj)
        {
            // TODO: fast pass string support

            Py_ssize_t size;
            auto buffer = PyUnicode_AsWideCharString(obj, &size);

            if (!buffer)
            {
                throw winrt::hresult_invalid_argument();
            }

            // TODO: need to check to see if size is out of range
            winrt::hstring result{ buffer, static_cast<winrt::hstring::size_type>(size) };
            // TODO: can we do this w/o making another copy?
            PyMem_Free(buffer);
            return result;
        }
    };

    template<typename T>
    PyObject* convert_to(T value) noexcept
    {
        return converter<T>::convert_to(value);
    }

    template<typename T>
    PyObject* convert_enum_to(T instance)
    {
        using enum_type = std::underlying_type_t<T>;
        return convert_to<enum_type>(static_cast<enum_type>(instance));
    }

    template<typename T>
    T convert_from(PyObject* arg)
    {
        if (!arg)
        {
            throw winrt::hresult_invalid_argument();
        }

        return converter<T>::convert_from(arg);
    }

    template<typename T>
    T convert_from(PyObject* args, int index)
    {
        return convert_from<T>(PyTuple_GetItem(args, index));
    }
}
