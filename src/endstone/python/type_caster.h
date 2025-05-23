// Copyright (c) 2024, The Endstone Project. (https://endstone.dev) All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <pybind11/pybind11.h>

#include "endstone/endstone.hpp"

namespace pybind11::detail {
template <>
class type_caster<endstone::UUID> {
public:
    // Python -> C++
    bool load(handle src, bool)
    {
        /* Extract PyObject from handle */
        PyObject *source = src.ptr();

        /* Check if the Python Object is a UUID instance */
        PyObject *uuid_module = PyImport_ImportModule("uuid");
        PyObject *uuid_class = PyObject_GetAttrString(uuid_module, "UUID");
        if (!PyObject_IsInstance(source, uuid_class)) {
            PyErr_SetString(PyExc_TypeError, "Object is not an instance of UUID");
            Py_XDECREF(uuid_module);
            Py_XDECREF(uuid_class);
            return false;
        }

        /* Now try to convert into C++ object */
        PyObject *bytes = PyObject_GetAttrString(source, "bytes");
        if (PyBytes_GET_SIZE(bytes) != 16) {
            PyErr_SetString(PyExc_ValueError, "UUID bytes size must be 16");
            Py_XDECREF(bytes);
            Py_XDECREF(uuid_module);
            Py_XDECREF(uuid_class);
            return false;
        }

        const char *bytes_data = PyBytes_AS_STRING(bytes);
        for (int i = 0; i < 16; i++) {
            value.data[i] = bytes_data[i];
        }

        /* Clean up */
        Py_XDECREF(bytes);
        Py_XDECREF(uuid_module);
        Py_XDECREF(uuid_class);
        return PyErr_Occurred() == nullptr;
    }

    // C++ -> Python
    static handle cast(endstone::UUID src, return_value_policy /* policy */, handle /* parent */)
    {
        PyObject *uuid_module = PyImport_ImportModule("uuid");
        PyObject *uuid_class = PyObject_GetAttrString(uuid_module, "UUID");

        // Create Python bytes object from the UUID struct data.
        PyObject *py_bytes = PyBytes_FromStringAndSize(reinterpret_cast<const char *>(src.data), 16);

        // Create a dictionary to hold keyword arguments for the UUID constructor.
        PyObject *kwargs = PyDict_New();
        PyDict_SetItemString(kwargs, "bytes", py_bytes);

        PyObject *res = PyObject_Call(uuid_class, PyTuple_New(0), kwargs);

        Py_XDECREF(py_bytes);
        Py_XDECREF(kwargs);
        Py_XDECREF(uuid_module);
        Py_XDECREF(uuid_class);

        return {res};
    }

    PYBIND11_TYPE_CASTER(endstone::UUID, const_name("uuid.UUID"));
};

template <typename Value>
class type_caster<endstone::Result<Value>> {
public:
    using value_conv = make_caster<Value>;

    template <typename T>
    static handle cast(T &&src, return_value_policy policy, handle parent)
    {
        if (!src) {
            throw std::runtime_error(src.error());
        }
        if (!std::is_lvalue_reference<T>::value) {
            policy = return_value_policy_override<Value>::policy(policy);
        }
        return value_conv::cast(*std::forward<T>(src), policy, parent);
    }

    PYBIND11_TYPE_CASTER(endstone::Result<Value>, value_conv::name);
};

template <>
class type_caster<endstone::Result<void>> {
public:
    template <typename T>
    static handle cast(T &&src, return_value_policy policy, handle parent)
    {
        if (!src) {
            throw std::runtime_error(src.error());
        }
        return none().release();
    }
    PYBIND11_TYPE_CASTER(endstone::Result<void>, const_name("None"));
};

}  // namespace pybind11::detail
