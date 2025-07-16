#! /usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division
import os
import sys
import re
import math

# Assumes SolidPython is in site-packages or elsewhwere in sys.path
from solid import *
from solid.utils import *
from utils import *

SEGMENTS = 32

eps = 0.01
rr = 2

wth = 1.5
a = 28
b = a + 4*wth
c = 40.5
d = c + 4*wth
gap = 0.3
# Height of pinheaders above rim
h = 14+5
# Dimple
dd1 = 9
dd2 = 6.75
dh = 1.5

def dimple(a):
    pad = roundccube(dd1, 3, dd1 + 2, 1)
    offset = wth
    if a > 0:
        offset = -offset
    part = hole()(rot(a, 90, 90, cylinder(d1 = dd1, d2 = dd2, h = dh))) + trans(0, offset, -dd1/2, pad)
    r = 10
    y_off = 0.2
    if a > 0:
        y_off = -y_off
    n = 18
    for i in range(0, n):
        a = 2*math.pi*((i+0.5)/n)
        f = 1
        if i == 4:
            f = 0.9
        elif i == 3 or i == 5:
            f = 0.95
        d1 = trans(r * f * math.cos(a), y_off, r * f * math.sin(a), dot())
        part = part + d1
    return part

def dot():
    return sphere(r = 1.2)

def plughole():
    w = 10.5+.4 # was .5
    h = 5+.4 # was .5
    c = ccube(3*wth, w, h)
    return trans(0, 0, 0, c)

def ridge():
    s = sphere(r = .5)
    f = 0.75*a/2
    return hull()(trans(0, f, 0, s) + trans(0, -f, 0, s))

def assembly():
    base = roundxycube(d, b, h + wth, rr)
    cutout1 = roundxycube(c + 2*wth + gap, a + 2*wth + gap, 6, rr/2)
    cutout2 = roundxycube(c, a, h, rr/2)
    dimples = trans(0, b/2 + eps, (h + 2*wth)/2, dimple(180)) + trans(0, -b/2 - eps, (h + 2*wth)/2, dimple(0))
    ch = 5
    cutter = ccube(20, 50, ch)
    rz = 3.3
    rx = 1.1
    ridges = trans((c + wth)/2 + rx, 0, rz, ridge()) + trans(-((c + wth)/2 + rx), 0, rz, ridge())
    return base - down(eps)(cutout1 + cutout2) + dimples - trans(d/2 - wth, 0, h - 5.2, plughole()) - trans(0, 0, h+wth, cutter) - trans(0, 0, -5, cutter) + ridges

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)


# Local Variables:
# compile-command: "python esp32camtop.py"
# End:
