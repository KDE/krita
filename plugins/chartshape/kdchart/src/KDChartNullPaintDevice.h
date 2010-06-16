/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2009 Klaralvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#ifndef NULL_PAINT_DEVICE_H
#define NULL_PAINT_DEVICE_H

#include <QPaintDevice>
#include <QPaintEngine>

namespace KDChart
{
    class NullPaintEngine : public QPaintEngine
    {
    public:
        virtual bool begin(QPaintDevice * /*pdev*/) { return true; }
        virtual void drawEllipse(const QRectF & /*rect*/) { }
        virtual void drawEllipse(const QRect & /*rect*/) { }
        virtual void drawImage(const QRectF & /*rectangle*/, const QImage & /*image*/, const QRectF & /*sr*/, Qt::ImageConversionFlags /*flags*/) { }
        virtual void drawLines(const QLineF * /*lines*/, int /*lineCount*/) { }
        virtual void drawLines(const QLine * /*lines*/, int /*lineCount*/) { }
        virtual void drawPath(const QPainterPath & /*path*/) { }
        virtual void drawPixmap(const QRectF & /*r*/, const QPixmap & /*pm*/, const QRectF & /*sr*/) { }
        virtual void drawPoints(const QPointF * /*points*/, int /*pointCount*/) { }
        virtual void drawPoints(const QPoint * /*points*/, int /*pointCount*/) { }
        virtual void drawPolygon(const QPointF * /*points*/, int /*pointCount*/, PolygonDrawMode /*mode*/) { }
        virtual void drawPolygon(const QPoint * /*points*/, int /*pointCount*/, PolygonDrawMode /*mode*/) { }
        virtual void drawRects(const QRectF * /*rects*/, int /*rectCount*/) { }
        virtual void drawRects(const QRect * /*rects*/, int /*rectCount*/) { }
        virtual void drawTextItem(const QPointF & /*p*/, const QTextItem & /*textItem*/) { }
        virtual void drawTiledPixmap(const QRectF & /*rect*/, const QPixmap & /*pixmap*/, const QPointF & /*p*/) { }
        virtual bool end()  { return true; }

        virtual Type type() const { return QPaintEngine::User; }
        virtual void updateState(const QPaintEngineState & /*state*/) { }
    };

    class NullPaintDevice : public QPaintDevice
    {
    public:
        NullPaintDevice(const QSize& size) : m_size(size) { }
        ~NullPaintDevice() { }

        int metric(PaintDeviceMetric metric) const
        {
            switch(metric)
            {
            case QPaintDevice::PdmWidth:
                return m_size.width();
            case QPaintDevice::PdmHeight:
                return m_size.height();
            case QPaintDevice::PdmWidthMM:
                return 1;
            case QPaintDevice::PdmHeightMM:
                return 1;
            case QPaintDevice::PdmNumColors:
                return int((uint)(-1));
            case QPaintDevice::PdmDepth:
                return 1;
            case QPaintDevice::PdmDpiX:
                return 1;
            case QPaintDevice::PdmDpiY:
                return 1;
            case QPaintDevice::PdmPhysicalDpiX:
                return 1;
            case QPaintDevice::PdmPhysicalDpiY:
                return 1;
            }
            return 1;
        }

        QPaintEngine* paintEngine() const
        {
            static NullPaintEngine nullPaintEngine;
            return &nullPaintEngine;
        }

    private:
        QSize m_size;
    };

}

#endif
