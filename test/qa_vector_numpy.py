#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2022 Josh Morman
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import unittest
import numpy as np
import pmtf as pmt
import time


class qa_pmt_vector (unittest.TestCase):

    def _test_array(self, x):
        y = pmt.pmt(x)
        z = y()  # get back the numpy type
        self.assertEqual(x.dtype, z.dtype)
        self.assertEqual(x.shape, z.shape)
        self.assertTrue(np.array_equiv(x, z))

    def test_vector(self):
        rng = np.random.default_rng(seed=823474928)
        sz = (1000,)
        x = np.array(rng.normal(size=sz)+1j*rng.normal(size=sz), dtype=np.complex64)
        self._test_array(x)

        x = np.array(rng.normal(size=sz)+1j*rng.normal(size=sz), dtype=np.complex128)
        self._test_array(x)
        
        for d in [np.float32, np.float64]:
            x = np.array(rng.normal(size=sz), dtype=d)
            self._test_array(x)

        for d in [np.uint64, np.uint32, np.uint16, np.uint8, np.int64, np.int32, np.int16, np.int8]:
            x = np.array(rng.integers(np.iinfo(d).min, np.iinfo(d).max/2, size=sz), dtype=d)
            self._test_array(x)


if __name__ == '__main__':
    unittest.main()
