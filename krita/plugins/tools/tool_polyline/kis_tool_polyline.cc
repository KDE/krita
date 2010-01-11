/*
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_tool_polyline.h"

#include <QVector>

#include <KoCanvasBase.h>
#include <KoPathShape.h>
#include <KoShapeController.h>
#include <KoLineBorder.h>

#include <kis_paint_layer.h>
#include <kis_selection.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>
#include <kis_paintop_preset.h>


KisToolPolyline::KisToolPolyline(KoCanvasBase * canvas)
        : KisToolPolylineBase(canvas, KisCursor::load("tool_polyline_cursor.png", 6, 6))
{
    setObjectName("tool_polyline");
}

KisToolPolyline::~KisToolPolyline()
{
}

void KisToolPolyline::finishPolyline(const QVector<QPointF>& points)
{
    if (!currentNode()->inherits("KisShapeLayer")) {
        KisPaintDeviceSP device = currentNode()->paintDevice();
        if (!device) return;

        KisPainter painter(device, currentSelection());
        painter.beginTransaction(i18n("Polyline"));
        if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(currentNode().data())) {
            painter.setChannelFlags(l->channelFlags());
            if (l->alphaLocked()) {
                painter.setLockAlpha(l->alphaLocked());
            }
        }
        painter.setPaintColor(currentFgColor());
        painter.setOpacity(m_opacity);
        painter.setCompositeOp(m_compositeOp);
        painter.setPaintOpPreset(currentPaintOpPreset(), currentImage()); // Painter takes ownership

        QPointF start, end;
        QVector<QPointF>::const_iterator it;
        for (it =
             points.begin();
             it !=
                     points.end(); ++it) {
            if (it == points.begin()) {
                start = (*it);
            } else {
                end = (*it);
                painter.paintLine(start, end);
                start = end;
            }
        }
        device->setDirty(painter.dirtyRegion());
        canvas()->addCommand(painter.endTransaction());
    } else {
        KoPathShape* path = new KoPathShape();
        path->setShapeId(KoPathShapeId);

        QMatrix resolutionMatrix;
        resolutionMatrix.scale(1 / currentImage()->xRes(), 1 / currentImage()->yRes());
        path->moveTo(resolutionMatrix.map(points[0]));
        for (int i = 1; i < points.count(); i++)
            path->lineTo(resolutionMatrix.map(points[i]));
        path->normalize();

        KoLineBorder* border = new KoLineBorder(1.0, currentFgColor().toQColor());
        path->setBorder(border);

        QUndoCommand * cmd = canvas()->shapeController()->addShape(path);
        canvas()->addCommand(cmd);
    }

    notifyModified();
}

#include "kis_tool_polyline.moc"
