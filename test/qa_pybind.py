#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2022 Josh Morman
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import unittest
import numpy as np
import pmtv
from pmtv import pmt
import time


class qa_pybind (unittest.TestCase):

    def _pmt_convert(self, x, compare_type=True):
        p = pmt(x)
        xp = p()
        # if compare_type:
            # self.assertEqual(type(x), type(xp))
        self.assertAlmostEqual(x, xp, 6)

    def _test_array(self, x):
        y = pmt(x)
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

    def test_scalar(self):
        self._pmt_convert(np.int8(17))
        self._pmt_convert(np.int16(17))
        self._pmt_convert(np.int32(17))
        self._pmt_convert(np.int64(17))
        self._pmt_convert(np.uint8(17))
        self._pmt_convert(np.uint16(17))
        self._pmt_convert(np.uint32(17))
        self._pmt_convert(np.uint64(17))

        self._pmt_convert(np.float32(44.4))
        self._pmt_convert(np.float64(44.4))
        self._pmt_convert(complex(44.4, -55.4), compare_type=False)


    def test_map(self):
        m = {'abc': pmt(123)}
        x = pmt(m)
        xp = pmtv.get_map(x) # () operator does not yet work for maps
        self.assertEqual(xp['abc'](),123)

        # Nested Map
        m = {'abc': pmt({'def': pmt(456)})}
        x = pmt(m)
        xp = pmtv.get_map(x) # () operator does not yet work for maps
        self.assertEqual(pmtv.get_map(xp['abc'])['def'](),456)

        # Vector in a Nested Map
        m = {'meta': pmt({'abc': pmt(123)}), 'data': pmt(np.array([1,2,3],dtype=np.float32))}
        x = pmt(m)
        xp = pmtv.get_map(x) 
        self.assertEqual(pmtv.get_map(xp['meta'])['abc'](),123)
        self.assertTrue(np.array_equal(xp['data'](),[1,2,3]))



if __name__ == '__main__':
    unittest.main()
