/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_grid_decoration.h"

#include <QPainter>
#include <QPen>
#include <QtMath>
#include <klocalizedstring.h>

#include <KoUnit.h>
#include "kis_grid_config.h"

struct KisGridDecoration::Private
{
    KisGridConfig config;
};

KisGridDecoration::KisGridDecoration(KisView* parent)
    : KisCanvasDecoration("grid", parent),
      m_d(new Private)
{

}

KisGridDecoration::~KisGridDecoration()
{

}

void KisGridDecoration::setGridConfig(const KisGridConfig &config)
{
    m_d->config = config;
}

void KisGridDecoration::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas)
{
    if (!m_d->config.showGrid()) return;

    Q_UNUSED(canvas);

    QTransform transform = converter->imageToWidgetTransform();

    const qreal scale = KoUnit::approxTransformScale(transform);
    const int minWidgetSize = 3;
    const int effectiveSize = qMin(m_d->config.spacing().x(), m_d->config.spacing().y());

    int scaleCoeff = 1;
    quint32 subdivision = m_d->config.subdivision();

    while (qFloor(scale * scaleCoeff * effectiveSize) <= minWidgetSize) {
        if (subdivision > 1) {
            scaleCoeff = subdivision;
            subdivision = 1;
        } else {
            scaleCoeff *= 2;
        }
    }

    const QPen mainPen = m_d->config.penMain();
    const QPen subdivisionPen = m_d->config.penSubdivision();

    gc.save();
    gc.setTransform(transform);

    qreal x1, y1, x2, y2;
    QRectF imageRect =
        converter->documentToImage(updateArea) &
        converter->imageRectInImagePixels();
    imageRect.getCoords(&x1, &y1, &x2, &y2);

    {   // vertical lines
        const int offset = m_d->config.offset().x();
        const int step = scaleCoeff * m_d->config.spacing().x();
        const int lineIndexFirst = qCeil((x1 - offset) / step);
        const int lineIndexLast = qFloor((x2 - offset) / step);

        for (int i = lineIndexFirst; i <= lineIndexLast; i++) {
            int w = offset + i * step;

            gc.setPen(i % subdivision == 0 ? mainPen : subdivisionPen);
            gc.drawLine(QPointF(w, y1),QPointF(w, y2));
        }
    }

    {   // horizontal lines
        const int offset = m_d->config.offset().y();
        const int step = scaleCoeff * m_d->config.spacing().y();
        const int lineIndexFirst = qCeil((y1 - offset) / step);
        const int lineIndexLast = qFloor((y2 - offset) / step);

        for (int i = lineIndexFirst; i <= lineIndexLast; i++) {
            int w = offset + i * step;

            gc.setPen(i % subdivision == 0 ? mainPen : subdivisionPen);
            gc.drawLine(QPointF(x1, w),QPointF(x2, w));
        }
    }

    gc.restore();
}
