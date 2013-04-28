/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_tool_path.h"
#include <KoPathShape.h>
#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <kis_cursor.h>

KisToolPath::KisToolPath(KoCanvasBase * canvas)
        : KisToolShape(canvas, Qt::ArrowCursor), m_localTool(new LocalTool(canvas, this))
{
}

KisToolPath::~KisToolPath()
{
    delete m_localTool;
}

void KisToolPath::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisToolShape::activate(toolActivation, shapes);
    m_localTool->activate(toolActivation, shapes);
}

void KisToolPath::deactivate()
{
    KisToolShape::deactivate();
    m_localTool->deactivate();
}

void KisToolPath::mousePressEvent(KoPointerEvent *event)
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

void KisToolPath::mouseDoubleClickEvent(KoPointerEvent *event)
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

void KisToolPath::mouseMoveEvent(KoPointerEvent *event)
{
    Q_ASSERT(m_localTool);
    m_localTool->mouseMoveEvent(event);

    KisToolShape::mouseMoveEvent(event);
}

void KisToolPath::mouseReleaseEvent(KoPointerEvent *event)
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

void KisToolPath::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_ASSERT(m_localTool);
    m_localTool->paint(painter, converter);
}

QList<QWidget *> KisToolPath::createOptionWidgets()
{
    QList<QWidget *> list = KisToolShape::createOptionWidgets();
    list.append(m_localTool->createOptionWidgets());
    return list;
}

KisToolPath::LocalTool::LocalTool(KoCanvasBase * canvas, KisToolPath* selectingTool)
        : KoCreatePathTool(canvas), m_parentTool(selectingTool) {}

void KisToolPath::LocalTool::paintPath(KoPathShape &pathShape, QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    QTransform matrix;
    matrix.scale(m_parentTool->image()->xRes(), m_parentTool->image()->yRes());
    matrix.translate(pathShape.position().x(), pathShape.position().y());
    m_parentTool->paintToolOutline(&painter, m_parentTool->pixelToView(matrix.map(pathShape.outline())));
}

void KisToolPath::LocalTool::addPathShape(KoPathShape* pathShape)
{
    m_parentTool->addPathShape(pathShape, i18n("Path"));
}

#include "kis_tool_path.moc"
