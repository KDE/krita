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


#ifndef _KIS_DAB_SHAPE_H_
#define _KIS_DAB_SHAPE_H_

#include "dynamicbrush_export.h"

#include <kis_types.h>
#include <QRect>

class KisAutobrushShape;
class KisDynamicColoring;
class KisPaintInformation;

#include "kis_dynamic_shape.h"

struct KisDabShape : public KisDynamicShape {
    KisDabShape();
    virtual ~KisDabShape();
    virtual quint8 alphaAt(int x, int y) = 0;
    virtual void paintAt(const QPointF &pos, const KisPaintInformation& info, KisDynamicColoring* coloringsrc);
    /**
      * Call this function to create the stamp to apply on the paint device
      * @param stamp the temporary paint device on which the shape will draw the stamp
      * @param coloringsrc the color source to use for the stamp
      */
    virtual void createStamp(KisPaintDeviceSP stamp, KisDynamicColoring* coloringsrc,const QPointF &pos, const KisPaintInformation& info) =0;
    KisPaintDeviceSP m_dab;
};

struct KisAlphaMaskShape : public KisDabShape {
    KisAlphaMaskShape();
    virtual ~KisAlphaMaskShape();
    virtual quint8 alphaAt(int x, int y);
    virtual void resize(double xs, double ys);
    KisQImagemaskSP alphaMask;
};

class KisAutoMaskShape : public KisDabShape {
  public:
        struct KisAutoDab {
            enum Shape {
                ShapeCircle, ShapeRectangle
            };
            int width, height, hfade, vfade;
            Shape shape;
        };
    public:
        KisAutoMaskShape()  {  m_shape = 0; }
        virtual ~KisAutoMaskShape();
        virtual KisDynamicShape* clone() const;
        virtual quint8 alphaAt(int x, int y);
        virtual void resize(double xs, double ys);
        virtual void rotate(double r) { /* TODO */}
        virtual void createStamp(KisPaintDeviceSP stamp, KisDynamicColoring* coloringsrc,const QPointF &pos, const KisPaintInformation& info);
        virtual QRect rect()
        {
            return QRect(-autoDab.width/2, -autoDab.width/2, autoDab.width, autoDab.height);
        }
    public:
        KisAutoDab autoDab;
        KisAutobrushShape* m_shape;
};

#endif
