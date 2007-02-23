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
#include <KoInputDevice.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_types.h>

#include "kis_dynamic_brush.h"
#include "kis_dynamic_brush_registry.h"
#include "kis_dynamic_coloring.h"
#include "kis_dynamic_shape.h"
#include "kis_size_transformation.h"

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
    m_brush = KisDynamicBrushRegistry::instance()->current();
}

KisDynamicOp::~KisDynamicOp()
{
//     delete m_brush;
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
    KisDynamicShape* dabsrc = m_brush->shape();
    for(QList<KisDynamicTransformation*>::iterator transfo = m_brush->beginTransformation();
        transfo != m_brush->endTransformation(); ++transfo)
    {
        (*transfo)->transformBrush(dabsrc, adjustedInfo);
    }

    // Then to the coloring source
    KisDynamicColoring* coloringsrc = m_brush->coloring();
    for(QList<KisDynamicTransformation*>::iterator transfo = m_brush->beginTransformation();
        transfo != m_brush->endTransformation(); ++transfo)
    {
        (*transfo)->transformColoring(coloringsrc, adjustedInfo);
    }

    dabsrc->createStamp(dab, coloringsrc);

    // paint the dab
    QRect dabRect = QRect(0, 0, dabsrc->width(), dabsrc->height());
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
}
