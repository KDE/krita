/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *                1999 Michael Koch    <koch@kde.org>
 *                2002 Patrick Julien <freak@codepimps.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_tool_move.h"

#include <QPoint>
#include <QUndoCommand>

#include <klocale.h>

#include "commands/kis_node_commands.h"
#include "kis_cursor.h"
#include "kis_image.h"

#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "kis_node.h"

KisToolMove::KisToolMove(KoCanvasBase * canvas)
    :  KisTool(canvas, KisCursor::moveCursor())
{
    setObjectName("tool_move");
    m_dragging = false;
}

KisToolMove::~KisToolMove()
{
}

void KisToolMove::paint(QPainter& gc, const KoViewConverter &converter)
{
    Q_UNUSED(gc);
    Q_UNUSED(converter);
}

void KisToolMove::mousePressEvent(KoPointerEvent *e)
{
    if (m_canvas && e->button() == Qt::LeftButton) {
        QPointF pos = convertToPixelCoord(e);

        KisNodeSP node;

        if (!currentImage() || !(node = currentNode()) || !node->visible())
            return;

        m_dragging = true;
        m_dragStart = QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y()));
        m_layerStart.setX(node->x());
        m_layerStart.setY(node->y());
        m_layerPosition = m_layerStart;
    }
}

void KisToolMove::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_canvas && m_dragging) {
	QPointF posf = convertToPixelCoord(e);
        QPoint pos = QPoint(static_cast<int>(posf.x()), static_cast<int>(posf.y()));
        if((e->modifiers() & Qt::AltModifier) || (e->modifiers() & Qt::ControlModifier)) {
            if(fabs(pos.x() - m_dragStart.x()) > fabs(pos.y() - m_dragStart.y()))
                pos.setY(m_dragStart.y());
            else
                pos.setX(m_dragStart.x());
        }
        drag(pos);

        notifyModified();
    }
}

void KisToolMove::mouseReleaseEvent(KoPointerEvent *e)
{
    if (m_dragging) {
        if (m_canvas && e->button() == Qt::LeftButton) {
            QPointF pos = convertToPixelCoord(e);
            KisNodeSP node = currentNode();

            if (node) {
                drag(QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y())));
                m_dragging = false;

                QUndoCommand *cmd = new KisNodeMoveCommand(node, m_layerStart, m_layerPosition);
                Q_CHECK_PTR(cmd);

                m_canvas->addCommand(cmd);
            }
            currentImage()->setModified();
        }
    }
}

void KisToolMove::drag(const QPoint& original)
{
    // original is the position of the user chosen handle point
    KisNodeSP node = currentNode();

    if (node) {
        QPoint pos = original;
        QRect rc;

        pos -= m_dragStart; // convert to delta
        rc = node->extent();
        node->setX(node->x() + pos.x());
        node->setY(node->y() + pos.y());
        rc = rc.unite(node->extent());

        m_layerPosition = QPoint(node->x(), node->y());
        m_dragStart = original;

        node->setDirty(rc);
    }
}


#include "kis_tool_move.moc"
