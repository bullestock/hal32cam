#! /usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division
import os
import sys
import re

# Assumes SolidPython is in site-packages or elsewhwere in sys.path
from solid import *
from solid.utils import *
from utils import *

SEGMENTS = 16#64

eps = 0.01
rr = 2

wth = 1.5
a = 28
b = a + 4*wth
c = 40.5
d = c + 4*wth
gap = 0.3
# Height of pinheaders above rim
h = 14
#-- Dimple
dd1 = 9
dd2 = 6.75
# - height
dh = 1.5

arm_offset = .75

def dimple(a):
    pad = roundccube(dd1, 3, dd1 + 2, 1)
    return rot(a, 90, 90, cylinder(d1 = dd1, d2 = dd2, h = dh))

def screwhole():
    return cylinder(d = 3.5, h = 2*dd1)

def ridge(extra):
    s = sphere(d = 1.5)
    dd = 0.8
    z = extra + 5
    return hull()(trans(0, dd, -z, s) + trans(0, dd, z, s) + trans(0, -dd, -z, s) + trans(0, -dd, z, s))

def assembly():
    ah = d/2+15
    ah2 = 10 # Extending part
    arm = down(dd1*0.65 + ah2)(roundccube(dd1, dd1*.6, ah + ah2 + 0.15*dd1, rr))
    brace = up(ah - dd1)(roundccube(dd1, b + 6*dh + 2*arm_offset, dd1/2, rr))
    comp = 0.2 # Increase for tighter fit
    dimples = trans(0, b/2 - comp, 0, dimple(180)) + trans(0, -b/2 + comp, 0, dimple(0))
    spacer = rot(0, 90, 90, cylinder(d = dd1, h = dh + arm_offset + comp))
    spacers = trans(0, b/2 - comp, 0, spacer) + trans(0, -(b/2 + dh + arm_offset), 0, spacer)
    arms = trans(0, b/2 + dh*2 + arm_offset, 0, arm) + trans(0, -(b/2 + dh*2 + arm_offset), 0, arm)
    ridge_offset = b/2 + arm_offset + 1
    ridges = trans(0, -ridge_offset, 0, ridge(ah2)) + trans(0, ridge_offset, 0, ridge(ah2))
    return dimples + brace + arms + spacers - up(ah - 2*dd1)(screwhole()) + ridges

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python esp32cammount.py"
# End:
