import sys
from pathlib import Path
from build123d import *
from ocp_vscode import *


parent_module = sys.modules['.'.join(__name__.split('.')[:-1]) or '__main__']

def epilogue(p):
    show(p, reset_camera=Camera.KEEP)

    export_step(p.part, f'{Path(parent_module.__file__).stem}.step')

def filletx(p, radius):
    fillet(p.edges().filter_by(Axis.X), radius=radius)
    
def fillety(p, radius):
    fillet(p.edges().filter_by(Axis.Y), radius=radius)
    
def filletz(p, radius):
    fillet(p.edges().filter_by(Axis.Z), radius=radius)
    
