/*
 *  kis_tool_transform.cc -- part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  Based on the transform tool from :
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include "kis_tool_perspectivetransform.h"


#include <qpainter.h>
#include <qpen.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qapplication.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <knuminput.h>

#include <kis_global.h>
#include <kis_painter.h>
#include <kis_canvas_controller.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_selected_transaction.h>
#include <kis_button_press_event.h>
#include <kis_button_release_event.h>
#include <kis_move_event.h>
#include <kis_selection.h>
#include <kis_filter_strategy.h>
#include <kis_cmb_idlist.h>
#include <kis_id.h>
#include <kis_tool_controller.h>
#include <kis_perspectivetransform_worker.h>

//#include "wdg_tool_transform.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"

namespace {
    class PerspectiveTransformCmd : public KisSelectedTransaction {
        typedef KisSelectedTransaction super;

    public:
        PerspectiveTransformCmd(KisToolPerspectiveTransform *tool, KisPaintDeviceSP device, KisPaintDeviceSP origDevice,  KisPoint topleft, KisPoint topright, KisPoint bottomleft, KisPoint bottomright, KisSelectionSP origSel, QRect initialRect);
        virtual ~PerspectiveTransformCmd();

    public:
        virtual void execute();
        virtual void unexecute();
        void transformArgs(KisPoint &topleft, KisPoint &topright, KisPoint &bottomleft, KisPoint& bottomright) const;
        KisSelectionSP origSelection(QRect& initialRect) const;
        KisPaintDeviceSP theDevice();
        KisPaintDeviceSP origDevice();

    private:
        QRect m_initialRect;
        KisPoint m_topleft, m_topright, m_bottomleft, m_bottomright;
        KisToolPerspectiveTransform *m_tool;
        KisSelectionSP m_origSelection;
        KisPaintDeviceSP m_device;
        KisPaintDeviceSP m_origDevice;
    };

    PerspectiveTransformCmd::PerspectiveTransformCmd(KisToolPerspectiveTransform *tool, KisPaintDeviceSP device, KisPaintDeviceSP origDevice, KisPoint topleft, KisPoint topright, KisPoint bottomleft, KisPoint bottomright, KisSelectionSP origSel, QRect initialRect) :
            super(i18n("Perspective Transform"), device), m_initialRect(initialRect)
            , m_topleft(topleft), m_topright(topright), m_bottomleft(bottomleft), m_bottomright(bottomright)
            , m_tool(tool), m_origSelection(origSel), m_device(device), m_origDevice(origDevice)
    {
    }

    PerspectiveTransformCmd::~PerspectiveTransformCmd()
    {
    }

    void PerspectiveTransformCmd::transformArgs(KisPoint &topleft, KisPoint &topright, KisPoint &bottomleft, KisPoint& bottomright) const
    {
        topleft = m_topleft;
        topright = m_topright;
        bottomleft = m_bottomleft;
        bottomright = m_bottomright;
    }

    KisSelectionSP PerspectiveTransformCmd::origSelection(QRect& initialRect) const
    {
        initialRect = m_initialRect;
        return m_origSelection;
    }

    void PerspectiveTransformCmd::execute()
    {
        super::execute();
    }

    void PerspectiveTransformCmd::unexecute()
    {
        super::unexecute();
    }

    KisPaintDeviceSP PerspectiveTransformCmd::theDevice()
    {
        return m_device;
    }

    KisPaintDeviceSP PerspectiveTransformCmd::origDevice()
    {
        return m_origDevice;
    }
}

KisToolPerspectiveTransform::KisToolPerspectiveTransform()
    : super(i18n("Perspective Transform"))
{
    setName("tool_perspectivetransform");
    setCursor(KisCursor::selectCursor());
    m_subject = 0;
    m_origDevice = 0;
    m_origSelection = 0;
    m_handleHalfSize = 8;
    m_handleSize = 2 * m_handleHalfSize;
}

KisToolPerspectiveTransform::~KisToolPerspectiveTransform()
{
}

void KisToolPerspectiveTransform::deactivate()
{
    if (m_subject && m_subject->undoAdapter()) m_subject->undoAdapter()->removeCommandHistoryListener( this );

    KisImageSP img = m_subject->currentImg();
    if (!img) return;

    paintOutline();

   disconnect(m_subject->currentImg().data(), SIGNAL(sigLayerActivated(KisLayerSP)), this, SLOT(slotLayerActivated(KisLayerSP)));
}

void KisToolPerspectiveTransform::activate()
{
    super::activate();
    m_currentSelectedPoint = 0;
    if(m_subject && m_subject->currentImg() && m_subject->currentImg()->activeDevice())
    {
        //connect(m_subject, commandExecuted(KCommand *c), this, notifyCommandAdded( KCommand * c));
        m_subject->undoAdapter()->setCommandHistoryListener( this );

//         KisToolControllerInterface *controller = m_subject->toolController();
//         if (controller)
//             controller->setCurrentTool(this);

        PerspectiveTransformCmd * cmd=0;

        if(m_subject->currentImg()->undoAdapter()->presentCommand())
            cmd = dynamic_cast<PerspectiveTransformCmd*>(m_subject->currentImg()->undoAdapter()->presentCommand());

        // One of our commands is on top
        if(cmd &&cmd->theDevice() == m_subject->currentImg()->activeDevice())
        {
            // and it even has the same device
            // We should ask for tool args and orig selection
            m_origDevice = cmd->origDevice();
            cmd->transformArgs(m_topleft, m_topright, m_bottomleft, m_bottomright);
            m_origSelection = cmd->origSelection(m_initialRect);
            paintOutline();
        }
        else
        {
            initHandles();
        }
    }
    connect(m_subject->currentImg(), SIGNAL(sigLayerActivated(KisLayerSP)), this, SLOT(slotLayerActivated(KisLayerSP)));
}

void KisToolPerspectiveTransform::initHandles()
{
    Q_INT32 x,y,w,h;
    KisImageSP img = m_subject->currentImg();

    KisPaintDeviceSP dev = img->activeDevice();
    if (!dev ) return;

    // Create a lazy copy of the current state
    m_origDevice = new KisPaintDevice(*dev.data());
    Q_ASSERT(m_origDevice);

    if(dev->hasSelection())
    {
        KisSelectionSP sel = dev->selection();
        m_origSelection = new KisSelection(*sel.data());
        m_initialRect = sel->selectedExactRect();
    }
    else {
        m_initialRect = dev->exactBounds();
    }
    m_topleft = m_initialRect.topLeft();
    m_topright = m_initialRect.topRight();
    m_bottomleft = m_initialRect.bottomLeft();
    m_bottomright = m_initialRect.bottomRight();

    m_subject->canvasController() ->updateCanvas();
}

void KisToolPerspectiveTransform::paint(KisCanvasPainter& gc)
{
    paintOutline(gc, QRect());
}

void KisToolPerspectiveTransform::paint(KisCanvasPainter& gc, const QRect& rc)
{
    paintOutline(gc, rc);
}

bool KisToolPerspectiveTransform::mouseNear(const QPoint& mousep, const QPoint point)
{
    return (QRect( (point.x() - m_handleHalfSize), (point.y() - m_handleHalfSize), m_handleSize, m_handleSize).contains(mousep) );
}

void KisToolPerspectiveTransform::buttonPress(KisButtonPressEvent *event)
{
    if (m_subject) {
        KisImageSP img = m_subject->currentImg();

        if (img && img->activeDevice() && event->button() == LeftButton) {
            m_actualyMoveWhileSelected = false;
            m_dragEnd = event->pos();
            KisCanvasController *controller = m_subject->canvasController();
            QPoint mousep = controller->windowToView( event->pos().roundQPoint() );
            if( mouseNear( mousep, controller->windowToView(m_topleft.roundQPoint() ) ) )
            {
                kdDebug() << " PRESS TOPLEFT HANDLE " << endl;
                m_currentSelectedPoint = &m_topleft;
            }
            else if( mouseNear( mousep, controller->windowToView(m_topright.roundQPoint() ) ) )
            {
                kdDebug() << " PRESS TOPRIGHT HANDLE " << endl;
                m_currentSelectedPoint = &m_topright;
            }
            else if( mouseNear( mousep, controller->windowToView(m_bottomleft.roundQPoint() ) ) )
            {
                kdDebug() << " PRESS BOTTOMLEFT HANDLE " << endl;
                m_currentSelectedPoint = &m_bottomleft;
            }
            else if( mouseNear( mousep, controller->windowToView(m_bottomright.roundQPoint() ) ) )
            {
                kdDebug() << " PRESS BOTTOMRIGHT HANDLE " << endl;
                m_currentSelectedPoint = &m_bottomright;
            }
        }
    }
}

void KisToolPerspectiveTransform::move(KisMoveEvent *event)
{
    if(m_currentSelectedPoint)
    {
        paintOutline();
        KisPoint translate = event->pos() - m_dragEnd;
        m_dragEnd = event->pos();
        *m_currentSelectedPoint += translate;;
        paintOutline();
        m_actualyMoveWhileSelected = true;
    }
}

void KisToolPerspectiveTransform::buttonRelease(KisButtonReleaseEvent * event)
{
    KisImageSP img = m_subject->currentImg();

    if (!img)
        return;
    if( m_currentSelectedPoint &&  event->button() == LeftButton)
    {
        m_currentSelectedPoint = 0;
        if(m_actualyMoveWhileSelected)
        {
            paintOutline();
            QApplication::setOverrideCursor(KisCursor::waitCursor());
            transform();
            QApplication::restoreOverrideCursor();
        }
    }
}

void KisToolPerspectiveTransform::paintOutline()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisCanvas *canvas = controller->kiscanvas();
        KisCanvasPainter gc(canvas);
        QRect rc;

        paintOutline(gc, rc);
    }
}

void KisToolPerspectiveTransform::paintOutline(KisCanvasPainter& gc, const QRect&)
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        RasterOp op = gc.rasterOp();
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);
        pen.setWidth(1);
        Q_ASSERT(controller);

        QPoint topleft = controller->windowToView(m_topleft ).roundQPoint();
        QPoint topright = controller->windowToView(m_topright).roundQPoint();
        QPoint bottomleft = controller->windowToView(m_bottomleft).roundQPoint();
        QPoint bottomright = controller->windowToView(m_bottomright).roundQPoint();

        gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        gc.drawRect(topleft.x()-4, topleft.y()-4, 8, 8);
        gc.drawLine(topleft.x(), topleft.y(), (topleft.x()+topright.x())/2, (topleft.y()+topright.y())/2);
//         gc.drawRect((topleft.x()+topright.x())/2-4, (topleft.y()+topright.y())/2-4, 8, 8);
        gc.drawLine((topleft.x()+topright.x())/2, (topleft.y()+topright.y())/2, topright.x(), topright.y());
        gc.drawRect(topright.x()-4, topright.y()-4, 8, 8);
        gc.drawLine(topright.x(), topright.y(), (topright.x()+bottomright.x())/2, (topright.y()+bottomright.y())/2);
//         gc.drawRect((topright.x()+bottomright.x())/2-4, (topright.y()+bottomright.y())/2-4, 8, 8);
        gc.drawLine((topright.x()+bottomright.x())/2, (topright.y()+bottomright.y())/2,bottomright.x(), bottomright.y());
        gc.drawRect(bottomright.x()-4, bottomright.y()-4, 8, 8);
        gc.drawLine(bottomright.x(), bottomright.y(), (bottomleft.x()+bottomright.x())/2, (bottomleft.y()+bottomright.y())/2);
//         gc.drawRect((bottomleft.x()+bottomright.x())/2-4, (bottomleft.y()+bottomright.y())/2-4, 8, 8);
        gc.drawLine((bottomleft.x()+bottomright.x())/2, (bottomleft.y()+bottomright.y())/2, bottomleft.x(), bottomleft.y());
        gc.drawRect(bottomleft.x()-4, bottomleft.y()-4, 8, 8);
        gc.drawLine(bottomleft.x(), bottomleft.y(), (topleft.x()+bottomleft.x())/2, (topleft.y()+bottomleft.y())/2);
//         gc.drawRect((topleft.x()+bottomleft.x())/2-4, (topleft.y()+bottomleft.y())/2-4, 8, 8);
        gc.drawLine((topleft.x()+bottomleft.x())/2, (topleft.y()+bottomleft.y())/2, topleft.x(), topleft.y());
        gc.setRasterOp(op);
        gc.setPen(old);
    }
}

void KisToolPerspectiveTransform::transform()
{
    KisImageSP img = m_subject->currentImg();

    if (!img || !img->activeDevice())
        return;

    KisProgressDisplayInterface *progress = m_subject->progressDisplay();

    // This mementoes the current state of the active device.
    PerspectiveTransformCmd * transaction = new PerspectiveTransformCmd(this, img->activeDevice(), m_origDevice,
            m_topleft, m_topright, m_bottomleft, m_bottomright, m_origSelection, m_initialRect);

    // Copy the original state back.
    QRect rc = m_origDevice->extent();
    rc = rc.normalize();
    img->activeDevice()->clear();
    KisPainter gc(img->activeDevice());
    gc.bitBlt(rc.x(), rc.y(), COMPOSITE_COPY, m_origDevice, rc.x(), rc.y(), rc.width(), rc.height());
    gc.end();

    // Also restore the original selection.
    if(m_origSelection)
    {
        QRect rc = m_origSelection->extent();
        rc = rc.normalize();
        img->activeDevice()->selection()->clear();
        KisPainter sgc(img->activeDevice()->selection().data());
        sgc.bitBlt(rc.x(), rc.y(), COMPOSITE_COPY, m_origSelection.data(), rc.x(), rc.y(), rc.width(), rc.height());
        sgc.end();
    }
    else
        if(img->activeDevice()->hasSelection())
            img->activeDevice()->selection()->clear();

    // Perform the transform. Since we copied the original state back, this doesn't degrade
    // after many tweaks. Since we started the transaction before the copy back, the memento
    // has the previous state.
    KisPerspectiveTransformWorker t(img->activeDevice(),m_topleft, m_topright, m_bottomleft, m_bottomright, progress);
    t.run();

    // If canceled, go back to the memento
    if(t.isCanceled())
    {
        transaction->unexecute();
        delete transaction;
        return;
    }

    img->activeDevice()->setDirty(rc); // XXX: This is not enough - should union with new extent

    // Else add the command -- this will have the memento from the previous state,
    // and the transformed state from the original device we cached in our activated()
    // method.
    if (transaction) {
        if (img->undo())
            img->undoAdapter()->addCommand(transaction);
        else
            delete transaction;
    }
}

void KisToolPerspectiveTransform::notifyCommandAdded( KCommand * command)
{
    PerspectiveTransformCmd * cmd = dynamic_cast<PerspectiveTransformCmd*>(command);
    if (cmd == 0) {
        // The last added command wasn't one of ours;
        // we should reset to the new state of the canvas.
        // In effect we should treat this as if the tool has been just activated
        initHandles();
    }
}

void KisToolPerspectiveTransform::notifyCommandExecuted( KCommand * command)
{
    Q_UNUSED(command);
    PerspectiveTransformCmd * cmd=0;
    if(m_subject->currentImg()->undoAdapter()->presentCommand())
        cmd = dynamic_cast<PerspectiveTransformCmd*>(m_subject->currentImg()->undoAdapter()->presentCommand());

    if (cmd == 0) {
        // The command now on the top of the stack isn't one of ours
        // We should treat this as if the tool has been just activated
        initHandles();
    }
    else
    {
        // One of our commands is now on top
        // We should ask for tool args and orig selection
        m_origDevice = cmd->origDevice();
        cmd->transformArgs(m_topleft, m_topright, m_bottomleft, m_bottomright);
        m_origSelection = cmd->origSelection(m_initialRect);
        m_subject->canvasController() ->updateCanvas();
    }
}

void KisToolPerspectiveTransform::slotLayerActivated(KisLayerSP)
{
    activate();
}


QWidget* KisToolPerspectiveTransform::createOptionWidget(QWidget* parent)
{
#if 0
    m_optWidget = new WdgToolPerspectiveTransform(parent);
    Q_CHECK_PTR(m_optWidget);

    m_optWidget->cmbFilter->clear();
    m_optWidget->cmbFilter->setIDList(KisFilterStrategyRegistry::instance()->listKeys());

    m_optWidget->cmbFilter->setCurrentText("Mitchell");
    connect(m_optWidget->cmbFilter, SIGNAL(activated(const KisID &)),
        this, SLOT(slotSetFilter(const KisID &)));

    KisID filterID = m_optWidget->cmbFilter->currentItem();
    m_filter = KisFilterStrategyRegistry::instance()->get(filterID);

/*
    connect(m_optWidget->intStartX, SIGNAL(valueChanged(int)), this, SLOT(setStartX(int)));
    connect(m_optWidget->intStartY, SIGNAL(valueChanged(int)), this, SLOT(setStartY(int)));
    connect(m_optWidget->intEndX, SIGNAL(valueChanged(int)), this, SLOT(setEndX(int)));
    connect(m_optWidget->intEndY, SIGNAL(valueChanged(int)), this, SLOT(setEndY(int)));
*/
    m_optWidget->intStartX->hide();
    m_optWidget->intStartY->hide();
    m_optWidget->intEndX->hide();
    m_optWidget->intEndY->hide();
    m_optWidget->textLabel1->hide();
    m_optWidget->textLabel2->hide();
    m_optWidget->textLabel3->hide();
    m_optWidget->textLabel4->hide();
#endif
    return 0;
}

QWidget* KisToolPerspectiveTransform::optionWidget()
{
    return 0;
}

void KisToolPerspectiveTransform::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection->action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Perspective Transform"),
                        "transform",
                        0,
                        this,
                        SLOT(activate()),
                        collection,
                        name());
        Q_CHECK_PTR(m_action);
        m_action->setToolTip(i18n("Perspective transform a layer or a selection"));
        m_action->setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_perspectivetransform.moc"
