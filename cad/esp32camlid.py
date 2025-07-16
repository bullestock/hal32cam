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

SEGMENTS = 64#128

eps = 0.001
rr = 2

wth = 1.5
a = 28
b = a + 4*wth
c = 40.5
d = c + 4*wth
e = 0
f = 8.5
g = 12
# Height of square part of camera
h = 1.7
j = 8.5
k = 5
m = 2.5
# camhole offset
cho = 10

def ridge():
    s = sphere(r = .8)
    f = 0.8*a/2
    return hull()(trans(0, f, 0, s) + trans(0, -f, 0, s))

def assembly():
    base = roundxycube(d, b, h + e + m, rr)
    rim = roundxycube(c + 2*wth, a + 2*wth, k, rr) - down(0.5)(ccube(c, a, k+1))
    camhole = cylinder(d = f, h = 10)
    camcone = cylinder(d1 = g, d2 = f, h = e)
    cambox = ccube(j, j, m + 1)
    camcutout = camhole + camcone + up(h + e)(cambox)
    cutout = trans(0, 0, 5, roundccube(15, a + 10, 10, 2))
    s1w = 5
    support1 = trans(-(c + s1w)/2 + s1w, 0, h + e + m - eps, ccube(s1w, a, 2))
    support = support1
    ledhole = trans(-(18.5 - 9), 12.25 - 1, -5, roundxycube(3.5, 3.5, 20, 0.5))
    rz = 3 + h + e + m
    rx = wth
    ridges = trans(c/2 + rx, 0, rz, ridge()) + trans(-(c/2 + rx), 0, rz, ridge())
    return base + up(h + e + m - eps)(rim) + support - trans(cho, 0, -2*eps, camcutout) - cutout - hole()(ledhole) - ridges

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python esp32camlid.py"
# End:
