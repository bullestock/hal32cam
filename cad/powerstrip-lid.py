# A box with a row of USB-C female connectors - upper part

# %%
from build123d import *
from ocp_vscode import *
from epilogue import *

from powerstrip_defs import *

width = N * p_cc
length = p_len + extra_len


with BuildPart() as p:
    with BuildSketch():
        RectangleRounded(width, length, 2)
    extrude(amount=2*ph_d)
    with BuildSketch(p.faces().sort_by(Axis.Z).first):
        RectangleRounded(width-2*inset, length-2*inset, 2)
    extrude(amount=ph_d)
    with BuildSketch(p.faces().sort_by(Axis.Z).last):
        with GridLocations(p_cc, 1, N, 1):
            RectangleRounded(p_w, p_len, p_w/2-0.01)
    extrude(amount=-ph_d, mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Z).last):
        with GridLocations(p_cc, 1, N, 1):
            RectangleRounded(co_w, co_len, 1)
    extrude(amount=-3*ph_d, mode=Mode.SUBTRACT)
    with BuildSketch(p.faces().sort_by(Axis.Z).last):
        with GridLocations(p_cc, p_h_cc, N, 2):
            Circle(hole_d/2)
    extrude(amount=-3*ph_d, mode=Mode.SUBTRACT)

epilogue(p)
