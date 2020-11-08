# SPDX-License-Identifier: CC0-1.0

"""
This script will iterate over all toplevel nodes, create
keeping track of the layer boundaries and then resize
the image to the unity of all boundaries.
"""

from krita import Krita
from PyQt5.QtCore import QRect

d = Krita.instance().activeDocument()
w = d.width()
h = d.height()
x = d.xOffset()
y = d.yOffset()

print(x, y, w, h)
r = QRect(x, y, w, h)
print(r)
for n in d.topLevelNodes():
    print(n, n.bounds())
    b = n.bounds()
    r = r.united(b)

print(r)

d.resizeImage(r.x(), r.y(), r.width(), r.height())
