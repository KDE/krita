/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License.
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

#ifndef _KISDYNAMICTRANSFORMATION_H_
#define _KISDYNAMICTRANSFORMATION_H_

#include <KoColor.h>

#include <kis_types.h>

class KisPaintInformation;
class KisAutobrushShape;

// TODO: I don't like the following two classes too much, find a better a way to do it
struct KisColoringSource {
    enum ColoringType {
        ColoringPlainColor, ColoringPaintDevice
    };
    KoColor color;
    KisPaintDeviceSP paintDevice;
    ColoringType type;
};

struct KisAutoDab {
    enum Shape {
        ShapeCircle, ShapeRectangle
    };
    int width, height, hfade, vfade;
    Shape shape;
};


struct KisDabSource {
    KisDabSource();
    virtual ~KisDabSource();
    enum DabType {
        DabAlphaMask, DabAuto
    };
    KisAlphaMaskSP alphaMask;
    KisAutoDab autoDab;
    DabType type;
    virtual quint8 alphaAt(int x, int y) = 0;
    KisAutobrushShape* m_shape;
};

struct KisDabAlphaMaskSource : public KisDabSource {
    KisDabAlphaMaskSource() { type = DabAlphaMask; }
    virtual ~KisDabAlphaMaskSource() { }
    virtual quint8 alphaAt(int x, int y);
};

struct KisDabAutoSource : public KisDabSource {
    KisDabAutoSource()  { type = DabAuto; m_shape = 0; }
    virtual ~KisDabAutoSource();
    virtual quint8 alphaAt(int x, int y);
};

/**
 * This is the base class for transformation.
 * 
 */
class KisDynamicTransformation {
    public:
        KisDynamicTransformation() : m_next(0) {}
        virtual ~KisDynamicTransformation() { if(m_next) delete m_next; }
        virtual void transformDab(KisDabSource& dabsrc, const KisPaintInformation& info) =0;
        virtual void transformColoring(KisColoringSource& dabsrc, const KisPaintInformation& info) =0;
        inline void setNextTransformation(KisDynamicTransformation* n) { m_next = n; }
        inline KisDynamicTransformation* nextTransformation() { return m_next; }
    private:
        KisDynamicTransformation* m_next;
};

#endif
