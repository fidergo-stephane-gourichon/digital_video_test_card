#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# Generate RGB <-> YCbCr conversion with 16.16 fixed point math. Uses
# limited range but not hard to modify for full range.
#
# Works with Python 2 and 3. Requires numpy 1.8.0 or later.
#
# Copyright (C) 2016 Väinö Helminen
#

from __future__ import division, print_function

from numpy import matrix, linalg

# ITU-R BT.601 (SDTV)
#kb, kr = 0.114, 0.299

# ITU-R BT.709 (HDTV)
kb, kr = 0.0722, 0.2126;

A = matrix([
    [ kr, (1 - kr - kb), kb ],
    [ 0.5 * (-kr) / (1 - kb), 0.5 * (-(1 - kr - kb)) / (1 - kb), 0.5 * (1 - kb) / (1 - kb)],
    [ 0.5 * (1.0 - kr) / (1 - kr), 0.5 * (-(1 - kr -kb)) / (1 - kr), 0.5 * (-kb) / (1 - kr)]
])
B = linalg.inv(A)


A[0] *= 65536 * 219 / 255
A[1:] *= 65536 * 224 / 255
A = A.round().tolist()
A[0].insert(0, 16*65536 + 32768)
for i in range(1,3):
    A[i].insert(0, 128*65536 + 32768)
print()
print('RGB to YCbCr:')
print('(%d + %d*r + %d*g + %d*b)>>16'%tuple(map(int, A[0])))
print('(%d + %d*r + %d*g + %d*b)>>16'%tuple(map(int, A[1])))
print('(%d + %d*r + %d*g + %d*b)>>16'%tuple(map(int, A[2])))

B[0] *= 65536 * 255 / 219
B[1:] *= 65536 * 255 / 224
B = B.round().tolist()
for i in range(0,3):
    B[i].insert(0, 32768)
print()
print('YCbCr to RGB:')
print('y -= 16')
print('cb -= 128')
print('cr -= 128')
print('(%d + %d*y + %d*cb + %d*cr)>>16'%tuple(map(int, B[0])))
print('(%d + %d*y + %d*cb + %d*cr)>>16'%tuple(map(int, B[1])))
print('(%d + %d*y + %d*cb + %d*cr)>>16'%tuple(map(int, B[2])))
