/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef _KIS_DYNAMIC_BRUSH_H_
#define _KIS_DYNAMIC_BRUSH_H_

#include <kis_types.h>

class KisAutobrushShape;
class KisDynamicColoring;

class KisDynamicShape {
    public:
        KisDynamicShape() {}
        virtual ~KisDynamicShape() {}
    public:
        virtual int width() =0;
        virtual int height() =0;
        /**
         * Call this function to resize the shape.
         * @param xs horizontal scaling
         * @param ys vertical scaling
         */
        virtual void resize(double xs, double ys) = 0;
        /**
         * Call this function to create the stamp to apply on the paint device
         * @param stamp the temporary paint device on which the shape will draw the stamp
         * @param coloringsrc the color source to use for the stamp
         */
        virtual void createStamp(KisPaintDeviceSP stamp, KisDynamicColoring* coloringsrc) =0;
};

struct KisAutoDab {
    enum Shape {
        ShapeCircle, ShapeRectangle
    };
    int width, height, hfade, vfade;
    Shape shape;
};

struct KisDabBrush : public KisDynamicShape {
    KisDabBrush();
    virtual ~KisDabBrush();
    enum DabType {
        DabAlphaMask, DabAuto
    };
    KisQImagemaskSP alphaMask;
    KisAutoDab autoDab;
    DabType type;
    virtual quint8 alphaAt(int x, int y) = 0;
    KisAutobrushShape* m_shape;
};

struct KisAlphaMaskBrush : public KisDabBrush {
    KisAlphaMaskBrush() { type = DabAlphaMask; }
    virtual ~KisAlphaMaskBrush() { }
    virtual quint8 alphaAt(int x, int y);
    virtual void resize(double xs, double ys);
};

struct KisAutoMaskBrush : public KisDabBrush {
    KisAutoMaskBrush()  { type = DabAuto; m_shape = 0; }
    virtual ~KisAutoMaskBrush();
    virtual quint8 alphaAt(int x, int y);
    virtual void resize(double xs, double ys);
    virtual void createStamp(KisPaintDeviceSP stamp, KisDynamicColoring* coloringsrc);
    virtual int width() { return autoDab.width; }
    virtual int height() { return autoDab.height; }
};

#endif
