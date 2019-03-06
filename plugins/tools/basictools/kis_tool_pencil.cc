/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tool_pencil.h"
#include <KoPathShape.h>
#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoShapeStroke.h>

#include <kis_cursor.h>

KisToolPencil::KisToolPencil(KoCanvasBase * canvas)
    : DelegatedPencilTool(canvas, Qt::ArrowCursor,
                          new __KisToolPencilLocalTool(canvas, this))
{
}

void KisToolPencil::resetCursorStyle()
{
    DelegatedPencilTool::resetCursorStyle();
    overrideCursorIfNotEditable();
}

void KisToolPencil::updatePencilCursor(bool value)
{
    setCursor(value ? Qt::ArrowCursor : Qt::ForbiddenCursor);
    resetCursorStyle();
}

void KisToolPencil::mousePressEvent(KoPointerEvent *event)
{
    if (!nodeEditable()) return;
    DelegatedPencilTool::mousePressEvent(event);
}

QList<QPointer<QWidget> > KisToolPencil::createOptionWidgets()
{
    QList<QPointer<QWidget> > widgetsList =
            DelegatedPencilTool::createOptionWidgets();

    QList<QPointer<QWidget> > filteredWidgets;
    Q_FOREACH (QWidget* widget, widgetsList) {
        if (widget->objectName() != "Stroke widget") {
            filteredWidgets.push_back(widget);
        }
    }
    return filteredWidgets;
}

__KisToolPencilLocalTool::__KisToolPencilLocalTool(KoCanvasBase * canvas, KisToolPencil* parentTool)
    : KoPencilTool(canvas), m_parentTool(parentTool) {}

void __KisToolPencilLocalTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_parentTool->strokeStyle() == KisPainter::StrokeStyleNone) {
        paintPath(path(), painter, converter);
    } else {
        KoPencilTool::paint(painter, converter);
    }
}



void __KisToolPencilLocalTool::paintPath(KoPathShape *pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    if (!pathShape) {
        return;
    }

    QTransform matrix;
    matrix.scale(m_parentTool->image()->xRes(), m_parentTool->image()->yRes());
    matrix.translate(pathShape->position().x(), pathShape->position().y());
    m_parentTool->paintToolOutline(&painter, m_parentTool->pixelToView(matrix.map(pathShape->outline())));
}

void __KisToolPencilLocalTool::addPathShape(KoPathShape* pathShape, bool closePath)
{
    if (closePath) {
        pathShape->close();
        pathShape->normalize();
    }

    m_parentTool->addPathShape(pathShape, kundo2_i18n("Draw Freehand Path"));
}

void __KisToolPencilLocalTool::slotUpdatePencilCursor()
{
    KoShapeStrokeSP stroke = this->createStroke();
    m_parentTool->updatePencilCursor(stroke && stroke->isVisible());
}
