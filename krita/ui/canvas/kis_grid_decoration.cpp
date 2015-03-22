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
#include <klocale.h>

#include "kis_grid_painter_configuration.h"
#include "kis_config.h"

KisGridDecoration::KisGridDecoration(KisView* parent) : KisCanvasDecoration("grid", parent)
{

}

KisGridDecoration::~KisGridDecoration()
{

}

void KisGridDecoration::drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter* converter, KisCanvas2* canvas)
{
    Q_UNUSED(canvas);

    KisConfig cfg;

    quint32 offsetx = cfg.getGridOffsetX();
    quint32 offsety = cfg.getGridOffsetY();
    quint32 hspacing = cfg.getGridHSpacing();
    quint32 vspacing = cfg.getGridVSpacing();
    quint32 subdivision = cfg.getGridSubdivisions() - 1;

    QPen mainPen = KisGridPainterConfiguration::mainPen();
    QPen subdivisionPen = KisGridPainterConfiguration::subdivisionPen();

    qreal x1, y1, x2, y2;
    QRectF imageRect = converter->documentToImage(updateArea);
    imageRect.getCoords(&x1, &y1, &x2, &y2);

    QTransform transform = converter->imageToWidgetTransform();

    gc.save();
    gc.setTransform(transform);

    quint32 i;

    // Draw vertical line
    i = subdivision - (offsetx / hspacing) % (subdivision + 1);
    double x = offsetx % hspacing;
    while (x <= x2) {
        if (i == subdivision) {
            gc.setPen(mainPen);
            i = 0;
        } else {
            gc.setPen(subdivisionPen);
            i++;
        }
        if (x >= x1) {
            // Always draw the full line otherwise the line stippling varies
            // with the location of area and we get glitchy patterns.
            gc.drawLine(QPointF(x, y1),QPointF(x, y2));
        }
        x += hspacing;
    }

    // Draw horizontal line
    i = subdivision - (offsety / vspacing) % (subdivision + 1);
    qreal y = offsety % vspacing;
    while (y <= y2) {
        if (i == subdivision) {
            gc.setPen(mainPen);
            i = 0;
        } else {
            gc.setPen(subdivisionPen);
            i++;
        }
        if (y >= y1) {
            gc.drawLine(QPointF(x1, y), QPointF(x2, y));
        }
        y += vspacing;
    }

    gc.restore();
}
