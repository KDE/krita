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

class KisSelectionOffsetCommand : public KNamedCommand {
    typedef KNamedCommand super;

public:
    KisSelectionOffsetCommand(KisSelectionSP layer, const QPoint& oldpos, const QPoint& newpos);
    virtual ~KisSelectionOffsetCommand();

    virtual void execute();
    virtual void unexecute();

private:
    void moveTo(const QPoint& pos);

private:
    KisSelectionSP m_layer;
    QPoint m_oldPos;
    QPoint m_newPos;
};

    KisSelectionOffsetCommand::KisSelectionOffsetCommand(KisSelectionSP layer, const QPoint& oldpos, const QPoint& newpos) :
        super(i18n("Move Layer"))
    {
        m_layer = layer;
        m_oldPos = oldpos;
        m_newPos = newpos;

    }

    KisSelectionOffsetCommand::~KisSelectionOffsetCommand()
    {
    }

    void KisSelectionOffsetCommand::execute()
    {
        moveTo(m_newPos);
    }

    void KisSelectionOffsetCommand::unexecute()
    {
        moveTo(m_oldPos);
    }

    void KisSelectionOffsetCommand::moveTo(const QPoint& pos)
    {
        if (m_layer->undoAdapter()) {
            m_layer->undoAdapter()->setUndo(false);
        }

        m_layer->setX(pos.x());
        m_layer->setY(pos.y());

        m_layer->parentPaintDevice()->setDirty();

        if (m_layer->undoAdapter()) {
            m_layer->undoAdapter()->setUndo(true);
        }
    }

        
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
    m_dragging = false;
}

void KisToolMoveSelection::buttonPress(KisButtonPressEvent *e)
{
    m_dragging = false;
    if (m_subject && e->button() == QMouseEvent::LeftButton) {
        QPoint pos = e->pos().floorQPoint();
        KisImageSP img = m_subject->currentImg();
        KisPaintLayerSP lay;

        if (!img || !(lay = dynamic_cast<KisPaintLayer*>( img->activeLayer().data() )))
            return;

        m_dragStart = pos;
        
        if ( !lay->visible() || !lay->paintDevice()->hasSelection())
            return;
        KisSelectionSP sel = lay->paintDevice()->selection();

        m_dragging = true;
        m_dragStart.setX(pos.x());
        m_dragStart.setY(pos.y());
        m_layerStart.setX(sel->getX());
        m_layerStart.setY(sel->getY());
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
        KisSelectionSP sel = lay->paintDevice()->selection();
            
        QRect rc;

        pos -= m_dragStart; // convert to delta
        rc = sel->selectedRect();
        sel->setX(sel->getX() + pos.x());
        sel->setY(sel->getY() + pos.y());
        rc = rc.unite(sel->selectedRect());

        m_layerPosition = QPoint(sel->getX(), sel->getY());
        m_dragStart = e->pos().floorQPoint();

        lay->paintDevice()->setDirty(rc);
    }

}

void KisToolMoveSelection::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && e->button() == QMouseEvent::LeftButton && m_dragging) {
        m_dragging = false;
        KisImageSP img = m_subject->currentImg();
        if(!img) return;
        KisPaintLayerSP lay = dynamic_cast<KisPaintLayer*>(img->activeLayer().data());

        if (lay->paintDevice()->hasSelection()) {
          KisSelectionSP dev = lay->paintDevice()->selection();
            m_dragging = false;

            if (img->undo()) {
                KCommand *cmd = new KisSelectionOffsetCommand( dev, m_layerStart, m_layerPosition);
                Q_CHECK_PTR(cmd);
                KisUndoAdapter *adapter = img->undoAdapter();
                if (adapter) {
                    adapter->addCommand(cmd);
                } else {
                    delete cmd;
                }
            }
            img->setModified();
            lay->setDirty();
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
