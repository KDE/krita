/*
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_boundary.h"
#include <QPainter>
#include <QPainterPath>
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
    Q_FOREACH (const QPolygon & polygon, d->m_boundary) {
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

    Q_FOREACH (const QPolygon & polygon, d->m_boundary) {
        painter.drawPolygon(polygon);
    }
}

QPainterPath KisBoundary::path() const
{
    return d->path;
}

