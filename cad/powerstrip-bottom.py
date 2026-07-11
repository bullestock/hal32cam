# A box with a row of USB-C female connectors - lower part

# %%
from build123d import *
from ocp_vscode import *
from epilogue import *

from powerstrip_defs import *

ADD_FLANGES = True

flange_w = 10
flange_l = 10
hole_d = 4.5

width = N * p_cc

length = p_len + extra_len


with BuildPart() as p:
    with BuildSketch():
        RectangleRounded(width, length, 2)
    extrude(amount=bot_h)
    with BuildSketch(p.faces().sort_by(Axis.Z).last):
        RectangleRounded(width-2*inset, length-2*inset, 2)
    extrude(amount=-(bot_h-inset), mode=Mode.SUBTRACT)
    if ADD_FLANGES:
        with BuildSketch():
            RectangleRounded(width + 2*flange_l, flange_w, 2)
        extrude(amount=inset)
        with BuildSketch():
            with GridLocations(width + flange_l, 1, 2, 1):
                Circle(hole_d/2)
        extrude(amount=inset, mode=Mode.SUBTRACT)

epilogue(p)
