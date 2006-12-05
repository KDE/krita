/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tool_move_selection.h"

#include <stdlib.h>
#include <qpoint.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <qcolor.h>
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_selection.h"
#include "kis_selection_manager.h"
#include "kis_undo_adapter.h"

KisToolMoveSelection::KisToolMoveSelection()
    : super(i18n("Move Selection Tool"))
{
    setName("tool_move_selection");
    m_subject = 0;
    setCursor(KisCursor::moveCursor());
}

KisToolMoveSelection::~KisToolMoveSelection()
{
}

void KisToolMoveSelection::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(subject);
}

void KisToolMoveSelection::buttonPress(KisButtonPressEvent *e)
{
    m_dragging = false;
    if (m_subject && e->button() == QMouseEvent::LeftButton) {
        QPoint pos = e->pos().floorQPoint();
        KisImageSP img = m_subject->currentImg();
        KisPaintLayerSP dev;

        if (!img || !(dev = dynamic_cast<KisPaintLayer*>( img->activeLayer().data() )))
            return;

        m_dragStart = pos;
        
        if ( !dev->visible() || !dev->paintDevice()->hasSelection())
            return;

        m_dragging = true;
        m_dragStart.setX(pos.x());
        m_dragStart.setY(pos.y());
        m_layerStart.setX(dev->x());
        m_layerStart.setY(dev->y());
        m_layerPosition = m_layerStart;

    }
}

void KisToolMoveSelection::move(KisMoveEvent *e)
{
    if (m_subject && m_dragging) {
        QPoint pos = e->pos().floorQPoint();
        if((e->state() & Qt::AltButton) || (e->state() & Qt::ControlButton)) {
            if(fabs(pos.x() - m_dragStart.x()) > fabs(pos.y() - m_dragStart.y()))
                pos.setY(m_dragStart.y());
            else
                pos.setX(m_dragStart.x());
        }
    
        KisImageSP img = m_subject->currentImg();
        KisPaintLayerSP lay = dynamic_cast<KisPaintLayer*>(m_subject->currentImg()->activeLayer().data());
        if(!lay) return;
        KisSelectionSP dev = lay->paintDevice()->selection();
            
        QRect rc;

        pos -= m_dragStart; // convert to delta
        rc = dev->selectedRect();
        dev->setX(dev->getX() + pos.x());
        dev->setY(dev->getY() + pos.y());
        rc = rc.unite(dev->selectedRect());

        m_layerPosition = QPoint(dev->getX(), dev->getY());
        m_dragStart = e->pos().floorQPoint();

        lay->setDirty(rc);
    }

}

void KisToolMoveSelection::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && e->button() == QMouseEvent::LeftButton && m_dragging) {
        KisImageSP img = m_subject->currentImg();
        if(!img) return;
        KisPaintLayerSP lay = dynamic_cast<KisPaintLayer*>(img->activeLayer().data());

        if (lay->paintDevice()->hasSelection()) {
          KisSelectionSP dev = lay->paintDevice()->selection();
//             drag(pos);
            m_dragging = false;

            if (img->undo()) {
                KCommand *cmd = dev->moveCommand(m_layerPosition.x(), m_layerPosition.y());
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

void KisToolMoveSelection::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Move selection"),
                        "tool_move",
                        Qt::SHIFT+Qt::Key_V,
                        this,
                        SLOT(activate()),
                        collection,
                        name());
        m_action->setToolTip(i18n("Move the selection"));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_move_selection.moc"
