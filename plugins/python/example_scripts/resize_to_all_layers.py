'''
This script is licensed CC 0 1.0, so that you can learn from it.

------ CC 0 1.0 ---------------

The person who associated a work with this deed has dedicated the work to the public domain by waiving all of his or her rights to the work worldwide under copyright law, including all related and neighboring rights, to the extent allowed by law.

You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission.

https://creativecommons.org/publicdomain/zero/1.0/legalcode
'''
#
# This script will iterate over all toplevel nodes, create
# keeping track of the layer boundaries and then resize
# the image to the unity of all boundaries.
#
from krita import *
d = Krita.instance().activeDocument()
w = d.width()
h = d.height()
x = d.xOffset()
y = d.yOffset()

print(x, y, w, h)
r = QRect(x, y, w, h)
print(r)
for n in d.topLevelNodes():
    print (n, n.bounds())
    b = n.bounds()
    r = r.united(b)
        
print (r) 

d.resizeImage(r.x(), r.y(), r.width(), r.height())
