# This file is part of Krita
#
# Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

class Inverter:
    def __init__(self):
        try:
            import krosskritacore
        except:
            raise "Import of the KritaCore module failed."
        doc = krosskritacore.get("KritaDocument")
        script = krosskritacore.get("KritaScript")
        image = doc.getImage()
        layer = image.getActivePaintLayer()
        if(layer.colorSpaceId() != "RGBA" ):
            raise("This script works only for 8bit RGBA layers")
        width = layer.getWidth()
        height = layer.getHeight()
        script.setProgressTotalSteps(width * height)
        layer.beginPainting("invert")
        it = layer.createRectIterator( 0, 0, width, height )
        print "kikoo\n"
        finesh = it.isDone()
        while (not finesh) :
            p = it.getRGBA()
            p[0] = 255 - p[0]
            p[1] = 255 - p[1]
            p[2] = 255 - p[2]
            it.setRGBA(p)
            script.incProgress()
            finesh = it.next()
        layer.endPainting()

Inverter()
