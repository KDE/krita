/*
 *  kis_tool_ellipse.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf <jcaliff@compuzone.net>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
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

#include "kis_tool_ellipse.h"
#include <KoCanvasBase.h>
#include <KoShapeController.h>

#include <kis_selection.h>
#include <kis_shape_tool_helper.h>
#include <kis_paint_device.h>


KisToolEllipse::KisToolEllipse(KoCanvasBase * canvas)
        : KisToolEllipseBase(canvas, KisCursor::load("tool_ellipse_cursor.png", 6, 6))
{
    setObjectName("tool_ellipse");
}

KisToolEllipse::~KisToolEllipse()
{
}

void KisToolEllipse::finishEllipse(const QRectF& rect)
{
    if (rect.isEmpty())
        return;

    if (!currentNode()->inherits("KisShapeLayer")) {
        if (!currentNode()->paintDevice())
            return;

        KisPaintDeviceSP device = currentNode()->paintDevice();

        KisPainter painter(device, currentSelection());

        painter.beginTransaction(i18n("Ellipse"));
        setupPainter(&painter);
        painter.setOpacity(m_opacity);
        painter.setCompositeOp(m_compositeOp);

        painter.paintEllipse(rect);
        QRegion bound = painter.dirtyRegion();
        device->setDirty(bound);
        notifyModified();

        canvas()->addCommand(painter.endTransaction());
    } else {
        QRectF r = convertToPt(rect);
        KoShape* shape = KisShapeToolHelper::createEllipseShape(r);

        QUndoCommand * cmd = canvas()->shapeController()->addShape(shape);
        canvas()->addCommand(cmd);
     }
}

#include "kis_tool_ellipse.moc"
