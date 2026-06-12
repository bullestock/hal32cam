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
    extrude(amount=bot_h)
    with BuildSketch(p.faces().sort_by(Axis.Z).last):
        RectangleRounded(width-2*inset, length-2*inset, 2)
    extrude(amount=-(bot_h-inset), mode=Mode.SUBTRACT)

epilogue(p)
