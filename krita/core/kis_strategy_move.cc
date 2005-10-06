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

#include <stdlib.h>
#include <qpoint.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <qcolor.h>
#include <koDocument.h>
#include <kis_doc.h>
#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_image.h"
#include "kis_paint_device_impl.h"
#include "kis_strategy_move.h"
#include "kis_undo_adapter.h"

namespace {
    class MoveCommand : public KNamedCommand {
        typedef KNamedCommand super;

    public:
        MoveCommand(KisCanvasController *controller, KisImageSP img, KisPaintDeviceImplSP device, const QPoint& oldpos, const QPoint& newpos);
        virtual ~MoveCommand();

        virtual void execute();
        virtual void unexecute();

    private:
        void moveTo(const QPoint& pos);

    private:
        KisCanvasController *m_controller;
        KisPaintDeviceImplSP m_device;
        QRect m_deviceBounds;
        QPoint m_oldPos;
        QPoint m_newPos;
        KisImageSP m_img;
    };

    MoveCommand::MoveCommand(KisCanvasController *controller, KisImageSP img, KisPaintDeviceImplSP device, const QPoint& oldpos, const QPoint& newpos) :
        super(i18n("Moved Painting Device"))
    {
        m_controller = controller;
        m_img = img;
        m_device = device;
        m_oldPos = oldpos;
        m_newPos = newpos;
        m_deviceBounds = m_device->exactBounds();
    }

    MoveCommand::~MoveCommand()
    {
    }

    void MoveCommand::execute()
    {
        moveTo(m_newPos);
    }

    void MoveCommand::unexecute()
    {
        moveTo(m_oldPos);
    }

    void MoveCommand::moveTo(const QPoint& pos)
    { 
       m_device -> move(pos.x(), pos.y());
       m_controller -> updateCanvas(m_deviceBounds |= QRect(pos.x(), pos.y(), m_deviceBounds.width(), m_deviceBounds.height()));
       m_deviceBounds.setRect(pos.x(), pos.y(), m_deviceBounds.width(), m_deviceBounds.height());
    }
}

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
        m_doc = subject -> document();
        m_controller = subject -> canvasController();
    } else {
        m_doc = 0;
        m_controller = 0;
    }
}

void KisStrategyMove::startDrag(const QPoint& pos)
{
    // pos is the user chosen handle point
    
    if (m_subject) {
        KisImageSP img;
        KisPaintDeviceImplSP dev;

        if (!(img = m_subject -> currentImg()))
            return;

        dev = img -> activeDevice();

        if (!dev || !dev -> visible())
            return;

        m_dragging = true;
        m_doc -> setModified(true);
        m_dragStart.setX(pos.x());
        m_dragStart.setY(pos.y());
        m_layerStart.setX(dev -> getX());
        m_layerStart.setY(dev -> getY());
        m_layerPosition = m_layerStart;
    }
}

void KisStrategyMove::drag(const QPoint& original)
{
    // original is the position of the user chosen handle point
    
    if (m_subject && m_dragging) {
        KisImageSP img = m_subject -> currentImg();
        KisPaintDeviceImplSP dev;

        if (img && (dev = img -> activeDevice())) {
            QPoint pos = original;
            QRect rc;

            if (pos.x() >= img -> width() || pos.y() >= img -> height())
                return;

            pos -= m_dragStart; // convert to delta
            rc = dev -> extent();
            dev -> move(dev ->getX() + pos.x(), dev->getY() + pos.y());
            rc = rc.unite(dev->extent());
            
            m_layerPosition = QPoint(dev ->getX(), dev ->getY());
             m_dragStart = original;

            img->notify(rc);
        }
    }
}

void KisStrategyMove::endDrag(const QPoint& pos, bool undo)
{
    if (m_subject && m_dragging) {
        KisImageSP img = m_subject -> currentImg();
        KisPaintDeviceImplSP dev;

        if (img && (dev = img -> activeDevice())) {
            drag(pos);
            m_dragging = false;

            if (undo) {
                KCommand *cmd = new MoveCommand(m_controller, img, img -> activeDevice(), m_layerStart, m_layerPosition);
                Q_CHECK_PTR(cmd);
                KisUndoAdapter *adapter = img -> undoAdapter();

                if (adapter)
                    adapter -> addCommand(cmd);
            }
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

