
static PyObject* _iterator_@(%* self)
{
    return reinterpret_cast<PyObject*>(self);
}

static PyObject* _iterator_next_@(%* self)
{
    return self->obj->dunder_iternext();
}

