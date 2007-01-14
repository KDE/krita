/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#include "kis_dynamicop.h"

#include <QRect>
#include <QWidget>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QHBoxLayout>

#include <kdebug.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_autobrush_resource.h>
#include <kis_brush.h>
#include <kis_global.h>
#include <kis_iterators_pixel.h>
#include <KoInputDevice.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_types.h>

#include "kis_darken_transformation.h"
#include "kis_dynamic_shape.h"
#include "kis_dynamic_coloring.h"
#include "kis_size_transformation.h"
#include "kis_transform_parameter.h"

KisPaintOp * KisDynamicOpFactory::createOp(const KisPaintOpSettings *settings, KisPainter * painter)
{
    Q_UNUSED(settings);

    KisPaintOp * op = new KisDynamicOp(painter);
    Q_CHECK_PTR(op);
    return op;
}

KisDynamicOp::KisDynamicOp( KisPainter *painter)
    : super(painter)
{
    m_firstTransfo = new KisSizeTransformation(new KisTransformParameterXTilt(), new KisTransformParameterYTilt() );
}

KisDynamicOp::~KisDynamicOp()
{
    delete m_firstTransfo;
}

void KisDynamicOp::paintAt(const QPointF &pos, const KisPaintInformation& info)
{
    KisPaintInformation adjustedInfo(info);


    kDebug() << info.pressure << " " << info.xTilt << " " << info.yTilt << endl;

    // Painting should be implemented according to the following algorithm:
    // retrieve brush
    // if brush == mask
    //          retrieve mask
    // else if brush == image
    //          retrieve image
    // subsample (mask | image) for position -- pos should be double!
    // apply filters to mask (color | gradient | pattern | etc.
    // composite filtered mask into temporary layer
    // composite temporary layer into target layer
    // @see: doc/brush.txt

    if (!m_painter->device()) return;

    KisPaintDeviceSP device = m_painter->device();

    QPointF pt = pos;

    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisPaintDeviceSP dab = new KisPaintDevice(device->colorSpace());

    quint8 origOpacity = m_painter->opacity();
    KoColor origColor = m_painter->paintColor();

    // First apply the transfo to the dab source
    KisDabBrush* dabsrc = new KisAutoMaskBrush;
    dabsrc->autoDab.shape = KisAutoDab::ShapeCircle;
    dabsrc->autoDab.width = 10;
    dabsrc->autoDab.height = 10;
    dabsrc->autoDab.hfade = 2;
    dabsrc->autoDab.vfade = 2;
    KisDynamicTransformation* transfo = m_firstTransfo;
    while(transfo)
    {
        m_firstTransfo->transformBrush(dabsrc, adjustedInfo);
        transfo = transfo->nextTransformation();
    }

    // Then to the coloring source
    KisDynamicColoring* coloringsrc = new KisPlainColoring;
    coloringsrc->type = KisDynamicColoring::ColoringPlainColor;
    coloringsrc->color = KoColor(QColor(255,200,100), 255, coloringsrc->color.colorSpace() );
    transfo = m_firstTransfo;
    while(transfo)
    {
        transfo->transformColoring(coloringsrc, adjustedInfo);
        transfo = transfo->nextTransformation();
    }

    // Transform into the paintdevice to apply
    switch(dabsrc->type)
    {
        case KisDabBrush::DabAuto:
        {
            switch(dabsrc->autoDab.shape)
            {
                case KisAutoDab::ShapeCircle:
                    dabsrc->m_shape = new KisAutobrushCircleShape(dabsrc->autoDab.width, dabsrc->autoDab.height, dabsrc->autoDab.hfade, dabsrc->autoDab.vfade);
                    break;
                case KisAutoDab::ShapeRectangle:
                    dabsrc->m_shape = new KisAutobrushRectShape(dabsrc->autoDab.width, dabsrc->autoDab.height, dabsrc->autoDab.hfade, dabsrc->autoDab.vfade);
                    break;
            }
        }
            break;
        case KisDabBrush::DabAlphaMask:
            break;
    }

    // Apply the coloring
    switch(coloringsrc->type)
    {
        case KisDynamicColoring::ColoringPlainColor:
        {
            KoColorSpace * colorSpace = dab->colorSpace();

            // Convert the kiscolor to the right colorspace.
            KoColor kc = coloringsrc->color;
            kc.convertTo(colorSpace);
            qint32 pixelSize = colorSpace->pixelSize();

            KisHLineIteratorPixel hiter = dab->createHLineIterator(0, 0, dabsrc->autoDab.width); // hum cheating
            for (int y = 0; y < dabsrc->autoDab.height; y++) // hum cheating (once again) /me slaps himself
            {
                int x=0;
                while(! hiter.isDone())
                {
                    // XXX: Set mask
                    colorSpace->setAlpha(kc.data(), dabsrc->alphaAt(x++, y), 1);
                    memcpy(hiter.rawData(), kc.data(), pixelSize);
//                     kDebug() << x << " " << y << " " << (int)dabsrc->alphaAt(x, y)  << " " << (int)hiter.rawData()[0] << " " << (int)hiter.rawData()[1] << " " << (int)hiter.rawData()[2] << " " << (int)hiter.rawData()[3] << endl;
                    ++hiter;
                }
                hiter.nextRow();
            }

        }
            break;
        case KisDynamicColoring::ColoringPaintDevice:
            // TODO: implement it
            break;
    }




//     if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
//         dab = brush->image(device->colorSpace(), adjustedInfo, xFraction, yFraction);
//     }
//     else {
//         KisQImagemaskSP mask = brush->mask(adjustedInfo, xFraction, yFraction);
//         dab = computeDab(mask);
//     }

//     m_painter->setPressure(adjustedInfo.pressure);

    QRect dabRect = QRect(0, 0, dabsrc->autoDab.width, dabsrc->autoDab.height); // cheating again
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImageSP image = device->image();

    if (image != 0) {
        dstRect &= image->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();
    kDebug() << sx << " " << sy << " " << sw << " " << sh << endl;
    if (m_source->hasSelection()) {
        m_painter->bltSelection(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab,
                                m_source->selection(), m_painter->opacity(), sx, sy, sw, sh);
    }
    else {
        m_painter->bitBlt(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab, m_painter->opacity(), sx, sy, sw, sh);
    }

    m_painter->setOpacity(origOpacity);
    m_painter->setPaintColor(origColor);
    delete dabsrc;
}
