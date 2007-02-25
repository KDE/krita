/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <QPoint>
#include <QColor>

#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <kdebug.h>

#include "kis_image.h"
#include "kis_layer.h"
#include "kis_strategy_move.h"
#include "kis_undo_adapter.h"

KisStrategyMove::KisStrategyMove()
    : m_image(0)
{
    m_dragging = false;
}

KisStrategyMove::KisStrategyMove(KisImageSP image)
    : m_image( image )
{
    m_dragging = false;
}

KisStrategyMove::~KisStrategyMove()
{
}

void KisStrategyMove::setImage(KisImageSP image)
{
    m_image = image;
}

void KisStrategyMove::startDrag(const QPoint& pos)
{
    // pos is the user chosen handle point

    if (m_image) {

        KisLayerSP dev;

        dev = m_image->activeLayer();

        if (!dev || !dev->visible())
            return;

        m_dragging = true;
        m_dragStart.setX(pos.x());
        m_dragStart.setY(pos.y());
        m_layerStart.setX(dev->x());
        m_layerStart.setY(dev->y());
        m_layerPosition = m_layerStart;
    }
}

void KisStrategyMove::drag(const QPoint& original)
{
    // original is the position of the user chosen handle point
    if (m_image && m_dragging) {

        KisLayerSP dev;

        if (dev = m_image->activeLayer()) {
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
}

void KisStrategyMove::endDrag(const QPoint& pos, bool undo)
{
    if (m_image && m_dragging) {
        KisLayerSP dev;

        if (m_image && (dev = m_image->activeLayer())) {
            drag(pos);
            m_dragging = false;

            if (undo && m_image->undo()) {
                KCommand *cmd = dev->moveCommand(m_layerStart, m_layerPosition);
                Q_CHECK_PTR(cmd);

                KisUndoAdapter *adapter = m_image->undoAdapter();
                if (adapter) {
                    adapter->addCommand(cmd);
                } else {
                    delete cmd;
                }
            }
            m_image->setModified();
        }
    }
}

void KisStrategyMove::simpleMove(const QPoint& pt1, const QPoint& pt2)
{
    startDrag(pt1);
    endDrag(pt2);
}

void KisStrategyMove::simpleMove(qint32 x1, qint32 y1, qint32 x2, qint32 y2)
{
    startDrag(QPoint(x1, y1));
    endDrag(QPoint(x2, y2));
}

