
/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push // ignore warning of external libraries that from this lib-context we do not have any control over
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#ifndef __clang__ // only for GCC, not Clang
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#endif

#include <pybind11/pybind11.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace py = pybind11;

void bind_pmt(py::module&);

// We need this hack because import_array() returns NULL
// for newer Python versions.
// This function is also necessary because it ensures access to the C API
// and removes a warning.
void* init_numpy()
{
    import_array();
    return NULL;
}

PYBIND11_MODULE(pmtv_python, m)
{
    init_numpy();
    // Initialize the numpy C API
    // (otherwise we will see segmentation faults)
    bind_pmt(m);
}
