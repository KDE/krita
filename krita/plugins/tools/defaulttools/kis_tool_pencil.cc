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

#include <kis_cursor.h>

KisToolPencil::KisToolPencil(KoCanvasBase * canvas)
        : KisToolShape(canvas, Qt::ArrowCursor), m_localTool(new LocalTool(canvas, this))
{
}

KisToolPencil::~KisToolPencil()
{
    delete m_localTool;
}

void KisToolPencil::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisToolShape::activate(toolActivation, shapes);
    m_localTool->activate(toolActivation, shapes);
}

void KisToolPencil::deactivate()
{
    KisToolShape::deactivate();
    m_localTool->deactivate();
}

void KisToolPencil::mousePressEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton, Qt::ShiftModifier |
                          Qt::ControlModifier | Qt::AltModifier)) {

        setMode(KisTool::PAINT_MODE);

        if (!nodeEditable()) {
            return;
        }

        Q_ASSERT(m_localTool);
        m_localTool->mousePressEvent(event);
    }
    else {
        KisToolShape::mousePressEvent(event);
    }
}

void KisToolPencil::mouseDoubleClickEvent(KoPointerEvent *event)
{
    if(PRESS_CONDITION_OM(event, KisTool::HOVER_MODE,
                          Qt::LeftButton, Qt::ShiftModifier |
                          Qt::ControlModifier | Qt::AltModifier)) {

        Q_ASSERT(m_localTool);
        m_localTool->mouseDoubleClickEvent(event);
    }
    else {
        KisToolShape::mouseDoubleClickEvent(event);
    }
}

void KisToolPencil::mouseMoveEvent(KoPointerEvent *event)
{
    Q_ASSERT(m_localTool);
    m_localTool->mouseMoveEvent(event);

    KisToolShape::mouseMoveEvent(event);
}

void KisToolPencil::mouseReleaseEvent(KoPointerEvent *event)
{
    if(RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        setMode(KisTool::HOVER_MODE);

        Q_ASSERT(m_localTool);
        m_localTool->mouseReleaseEvent(event);
    }
    else {
        KisToolShape::mouseReleaseEvent(event);
    }
}

void KisToolPencil::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_ASSERT(m_localTool);
    m_localTool->paint(painter, converter);
}

QList<QWidget *> KisToolPencil::createOptionWidgets()
{
    QList<QWidget *> list = KisToolShape::createOptionWidgets();
    list.append(m_localTool->createOptionWidgets());
    return list;
}

KisToolPencil::LocalTool::LocalTool(KoCanvasBase * canvas, KisToolPencil* selectingTool)
        : KoPencilTool(canvas), m_parentTool(selectingTool) {
            setToolId("pencilToolDummy");
        }

void KisToolPencil::LocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    QTransform matrix;
    matrix.scale(m_parentTool->image()->xRes(), m_parentTool->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());
    m_parentTool->paintToolOutline(&painter, m_parentTool->pixelToView(matrix.map(pathShape.outline())));
}

void KisToolPencil::LocalTool::addPathShape(KoPathShape* pathShape, bool closePath)
{
    pathShape->setShapeId(KoPathShapeId);
    pathShape->setStroke(currentStroke());
    if (closePath) {
        pathShape->close();
    }
    m_parentTool->addPathShape(pathShape, i18n("Freehand path"));
}

#include "kis_tool_pencil.moc"
