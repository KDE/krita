/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_boundary.h"
#include <QPainter>
#include <QPen>

#include "KoColorSpace.h"
#include "kis_fixed_paint_device.h"
#include "kis_outline_generator.h"

struct KisBoundary::Private {
    KisFixedPaintDeviceSP m_device;
    QVector<QPolygon> m_boundary;
    QPainterPath path;
};

KisBoundary::KisBoundary(KisFixedPaintDeviceSP dev) : d(new Private)
{
    d->m_device = dev;
}

KisBoundary::~KisBoundary()
{
    delete d;
}

void KisBoundary::generateBoundary()
{
    if (!d->m_device)
        return;

    KisOutlineGenerator generator(d->m_device->colorSpace(), OPACITY_TRANSPARENT_U8);
    generator.setSimpleOutline(true);
    d->m_boundary = generator.outline(d->m_device->data(), 0, 0, d->m_device->bounds().width(), d->m_device->bounds().height());

    d->path = QPainterPath();
    foreach(const QPolygon & polygon, d->m_boundary) {
        d->path.addPolygon(polygon);
        d->path.closeSubpath();
    }

}

void KisBoundary::paint(QPainter& painter) const
{ 
    QPen pen;
    pen.setWidth(0);
    pen.setBrush(Qt::black);
    painter.setPen(pen);
    
    foreach(const QPolygon & polygon, d->m_boundary) {
        painter.drawPolygon(polygon);
    }
}

QPainterPath KisBoundary::path() const
{
    return d->path;
}

