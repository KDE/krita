/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#include "kis_dab_shape.h"
#include <kis_auto_brush.h>
#include <kis_datamanager.h>
#include <kis_iterators_pixel.h>
#include <kis_paint_device.h>
#include <kis_painter.h>

#include "kis_dynamic_coloring.h"

#if 0

quint8 KisAlphaMaskShape::alphaAt(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
    return 255;
//     return alphaMask->alphaAt(x,y);
}

quint8 KisAutoMaskShape::alphaAt(int x, int y)
{
    return 255 - m_shape->valueAt(x, y);
}

KisAutoMaskShape::~KisAutoMaskShape()
{
    if (m_shape) delete m_shape;
}


KisAlphaMaskShape::KisAlphaMaskShape() {}

#endif


KisDabShape::~KisDabShape()
{
}

KisDabShape::KisDabShape(KisBrushSP brush)
        : m_scaleX(1.0)
        , m_scaleY(1.0)
        , m_rotate(0.0)
        , m_dab(0)
        , m_brush(brush)
{
}

KisDynamicShape* KisDabShape::clone() const
{
    return new KisDabShape(*this);
}

inline void splitCoordinate(double coordinate, qint32 *whole, double *fraction)
{
    qint32 i = static_cast<qint32>(coordinate);

    if (coordinate < 0) {
        // We always want the fractional part to be positive.
        // E.g. -1.25 becomes -2 and +0.75
        i--;
    }

    double f = coordinate - i;

    *whole = i;
    *fraction = f;
}

void KisDabShape::paintAt(const QPointF &pos, const KisPaintInformation& info, KisDynamicColoring* coloringsrc)
{

    if (!m_dab) {
        m_dab = new KisPaintDevice(painter()->device()->colorSpace());
    }


    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    QPointF hotSpot = m_brush->hotSpot(m_scaleX, m_scaleY);
    splitCoordinate(pos.x() - hotSpot.x(), &x, &xFraction);
    splitCoordinate(pos.y() - hotSpot.y(), &y, &yFraction);

    QRect dabRect = QRect(0, 0, m_brush->maskWidth(m_scaleX, m_rotate),
                          m_brush->maskHeight(m_scaleY, m_rotate));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    coloringsrc->colorize(m_dab, dabRect);
    KisFixedPaintDeviceSP fixedDab = new KisFixedPaintDevice(m_dab->colorSpace());
    fixedDab->setRect(dabRect);
    fixedDab->initialize();

    m_dab->writeBytes(fixedDab->data(), dstRect);
    m_brush->mask(fixedDab, m_scaleX, m_scaleY, m_rotate, info, xFraction, yFraction);
    m_dab->readBytes(fixedDab->data(), dstRect);

    if (painter()->bounds().isValid()) {
        dstRect &= painter()->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    painter()->bitBlt(dstRect.x(), dstRect.y(), m_dab, sx, sy, sw, sh);

}

void KisDabShape::resize(double xs, double ys)
{
    m_scaleX *= xs;
    m_scaleY *= ys;
}
void KisDabShape::rotate(double r)
{
    m_rotate += r;
}

QRect KisDabShape::rect() const
{
    int width = m_brush->maskWidth(m_scaleX, m_rotate);
    int height = m_brush->maskHeight(m_scaleY, m_rotate);
    QPointF hotSpot = m_brush->hotSpot(m_scaleX, m_scaleY);
    return QRect((int) - hotSpot.x(), (int) - hotSpot.y(), width, height);
}
