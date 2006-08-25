/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KROSS_KRITACOREKRS_PAINTER_H
#define KROSS_KRITACOREKRS_PAINTER_H

#include <QObject>

#include <kis_point.h>
#include <kis_types.h>
#include <kis_paint_layer.h>
//Added by qt3to4:
#include <Q3ValueList>

class KisPainter;
class KisFillPainter;

namespace Kross {

namespace KritaCore {

class Painter : public QObject
{
        //Q_OBJECT
    public:
        explicit Painter(KisPaintLayerSP layer);
        ~Painter();

    //public slots:

#if 0

        // Convolution
        /**
         * This function apply a convolution kernel to an image.
         * It takes at least three arguments :
         *  - a list of a list with the kernel (all lists need to have the same size)
         *  - factor
         *  - offset
         * 
         * The value of a pixel will be given by the following function K*P/factor + offset,
         * where K is the kernel and P is the neighbourhood.
         * 
         * It can takes the following optional arguments :
         *  - borderOp control how to convolve the pixels on the border of an image ( 0 use the default color
         * 1 use the pixel on the opposite side of the image 2 use the border pixel 3 avoid border pixels)
         *  - channel ( 1 for color 2 for alpha 3 for both)
         *  - x
         *  - y
         *  - width
         *  - height
         */
        Kross::Api::Object::Ptr convolve(Kross::Api::List::Ptr args);
        // Fill specific
        /**
         * Set the threshold the fill threshold.
         * It takes one argument :
         *  - threshold
         */
        Kross::Api::Object::Ptr setFillThreshold(Kross::Api::List::Ptr args);
        /**
         * Start filling color.
         * It takes two argument :
         *  - x
         *  - y
         */
        Kross::Api::Object::Ptr fillColor(Kross::Api::List::Ptr args);
        /**
         * start filling a pattern
         * It takes two argument :
         *  - x
         *  - y
         */
        Kross::Api::Object::Ptr fillPattern(Kross::Api::List::Ptr args);
        // Painting operations
        /**
         * This function will paint a polyline.
         * It takes two arguments :
         *  - a list of x position
         *  - a list of y position
         */
        Kross::Api::Object::Ptr paintPolyline(Kross::Api::List::Ptr args);
        /**
         * This function will paint a line.
         * It takes five arguments :
         *  - x1
         *  - y1
         *  - x2
         *  - y2
         *  - pressure
         */
        Kross::Api::Object::Ptr paintLine(Kross::Api::List::Ptr args);
        /**
         * This function will paint a Bezier curve.
         * It takes ten arguments :
         *  - x1
         *  - y1
         *  - p1
         *  - cx1
         *  - cy1
         *  - cx2
         *  - cx2
         *  - x2
         *  - y2
         *  - p2
         * 
         * Where (x1,y1) is the start position, p1 is the pressure at the start,
         * (x2,y2) is the ending position, p2 is the pressure at the end. (cx1,cy1) and (cx2,cy2)
         * are the position of the control points.
         */
        Kross::Api::Object::Ptr paintBezierCurve(Kross::Api::List::Ptr args);
        /**
         * This function will paint an ellipse.
         * It takes five arguments :
         *  - x1
         *  - y1
         *  - x2
         *  - y2
         *  - pressure
         * 
         * Where (x1,y1) and (x2,y2) are the position of the two centers.
         */
        Kross::Api::Object::Ptr paintEllipse(Kross::Api::List::Ptr args);
        /**
         * This function will paint a polygon.
         * It takes two arguments :
         *  - a list of x position
         *  - a list of y position
         */
        Kross::Api::Object::Ptr paintPolygon(Kross::Api::List::Ptr args);
        /**
         * This function will paint a rectangle.
         * It takes five arguments :
         *  - x
         *  - y
         *  - width
         *  - height
         *  - pressure
         */
        Kross::Api::Object::Ptr paintRect(Kross::Api::List::Ptr args);
        /**
         * This function will paint at a given position.
         * It takes three arguments :
         *  - x
         *  - y
         *  - pressure
         */
        Kross::Api::Object::Ptr paintAt(Kross::Api::List::Ptr args);
        // Color operations
        /**
         * This functions set the paint color (also called foreground color).
         * It takes one argument :
         *  - a Color
         */
        Kross::Api::Object::Ptr setPaintColor(Kross::Api::List::Ptr args);
        /**
         * This functions set the background color.
         * It takes one argument :
         *  - a Color
         */
        Kross::Api::Object::Ptr setBackgroundColor(Kross::Api::List::Ptr args);
        // How is painting done operations
        /**
         * This functions set the pattern used for filling.
         * It takes one argument :
         *  - a Pattern object
         */
        Kross::Api::Object::Ptr setPattern(Kross::Api::List::Ptr args);
        /**
         * This functions set the brush used for painting.
         * It takes one argument :
         *  - a Brush object
         */
        Kross::Api::Object::Ptr setBrush(Kross::Api::List::Ptr args);
        /**
         * This function define the paint operation.
         * It takes one argument :
         *  - the name of the paint operation
         */
        Kross::Api::Object::Ptr setPaintOp(Kross::Api::List::Ptr args);
        // Special settings
        /**
         * This function define the duplicate offset.
         * It takes two arguments :
         *  - horizontal offset
         *  - vertical offset
         */
        Kross::Api::Object::Ptr setDuplicateOffset(Kross::Api::List::Ptr args);
        // Style operation
        /**
         * This function set the opacity of the painting
         * It takes one argument :
         *  - opacity in the range 0 to 255
         */
        Kross::Api::Object::Ptr setOpacity(Kross::Api::List::Ptr args);
        /**
         * This function set the style of the stroke.
         * It takes one argument :
         *  - 0 for none 1 for brush
         */
        Kross::Api::Object::Ptr setStrokeStyle(Kross::Api::List::Ptr args);
        /**
         * This function set the fill style of the Painter.
         * It takes one argument :
         *  - 0 for none 1 for fill with foreground color 2 for fill with background color
         * 3 for fill with a pattern
         */
        Kross::Api::Object::Ptr setFillStyle(Kross::Api::List::Ptr args);
#endif

    protected:
        inline KisPaintLayerSP paintLayer() { return m_layer; }
    private:
        inline vKisPoint createPointsVector( Q3ValueList<QVariant> xs, Q3ValueList<QVariant> ys)
        {
            vKisPoint a;
            Q3ValueList<QVariant>::iterator itx = xs.begin();
            Q3ValueList<QVariant>::iterator ity = ys.begin();
            for(; itx != xs.end(); ++itx, ++ity)
            {
                a.push_back(KisPoint( (*itx).toDouble(), (*ity).toDouble()));
            }
            return a;
        }
        inline KisFillPainter* createFillPainter();
    private:
        KisPaintLayerSP m_layer;
        KisPainter* m_painter;
        int m_threshold;
};

}

}

#endif
