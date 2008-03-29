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
#include <QMouseEvent>
#include <QUndoCommand>

#include <klocale.h>

#include "commands/kis_node_commands.h"
#include "kis_cursor.h"
#include "kis_image.h"

#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "kis_layer.h"

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

        KisLayerSP dev;

        if (!currentImage() || !(dev = currentLayer()) || !dev->visible())
            return;

        m_dragging = true;
        m_dragStart = QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y()));
        m_layerStart.setX(dev->x());
        m_layerStart.setY(dev->y());
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
            KisLayerSP layer = currentLayer();

            if (layer) {
                drag(QPoint(static_cast<int>(pos.x()), static_cast<int>(pos.y())));
                m_dragging = false;

                QUndoCommand *cmd = new KisNodeMoveCommand(layer, m_layerStart, m_layerPosition);
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
    KisLayerSP dev = currentLayer();

    if (dev) {
        QPoint pos = original;
        QRect rc;

        pos -= m_dragStart; // convert to delta
        rc = dev->extent();
        dev->setX(dev->x() + pos.x());
        dev->setY(dev->y() + pos.y());
        rc = rc.unite(dev->extent());

        m_layerPosition = QPoint(dev->x(), dev->y());
        m_dragStart = original;

        dev->setDirty(rc);
    }
}


#include "kis_tool_move.moc"
