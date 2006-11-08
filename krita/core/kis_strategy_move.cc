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

#include <qpoint.h>
#include <qcolor.h>

#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <kdebug.h>

#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_strategy_move.h"
#include "kis_undo_adapter.h"

KisStrategyMove::KisStrategyMove()
{
    reset(0);
}

KisStrategyMove::KisStrategyMove(KisCanvasSubject *subject)
{
    reset(subject);
}

KisStrategyMove::~KisStrategyMove()
{
}

void KisStrategyMove::reset(KisCanvasSubject *subject)
{
    m_subject = subject;
    m_dragging = false;

    if (m_subject) {
        m_controller = subject->canvasController();
    } else {
        m_controller = 0;
    }
}

void KisStrategyMove::startDrag(const QPoint& pos)
{
    // pos is the user chosen handle point

    if (m_subject) {
        KisImageSP img;
        KisLayerSP dev;

        if (!(img = m_subject->currentImg()))
            return;

        dev = img->activeLayer();

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

    if (m_subject && m_dragging) {
        KisImageSP img = m_subject->currentImg();
        KisLayerSP dev;

        if (img && (dev = img->activeLayer())) {
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
    if (m_subject && m_dragging) {
        KisImageSP img = m_subject->currentImg();
        KisLayerSP dev;

        if (img && (dev = img->activeLayer())) {
            drag(pos);
            m_dragging = false;

            if (undo && img->undo()) {
                KCommand *cmd = dev->moveCommand(m_layerStart, m_layerPosition);
                Q_CHECK_PTR(cmd);
                
                KisUndoAdapter *adapter = img->undoAdapter();
                if (adapter) {
                    adapter->addCommand(cmd);
                } else {
                    delete cmd;
                }
            }
            img->setModified();
        }
    }
}

void KisStrategyMove::simpleMove(const QPoint& pt1, const QPoint& pt2)
{
    startDrag(pt1);
    endDrag(pt2);
}

void KisStrategyMove::simpleMove(Q_INT32 x1, Q_INT32 y1, Q_INT32 x2, Q_INT32 y2)
{
    startDrag(QPoint(x1, y1));
    endDrag(QPoint(x2, y2));
}

