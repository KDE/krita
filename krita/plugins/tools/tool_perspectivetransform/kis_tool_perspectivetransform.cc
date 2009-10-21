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
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_tool_perspectivetransform.h"
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QApplication>

#include <kaction.h>
#include <kactioncollection.h>
#include <kis_debug.h>
#include <klocale.h>
#include <knuminput.h>

#include <KoID.h>

#include <kis_global.h>
#include <kis_painter.h>
#include <canvas/kis_canvas.h>
#include <kis_canvas_controller.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_selected_transaction.h>
#include <KoPointerEvent.h>
#include <kis_selection.h>
#include <kis_filter_strategy.h>
#include <widgets/kis_cmb_idlist.h>
#include <kis_perspectivetransform_worker.h>


namespace
{
class PerspectiveTransformCmd : public KisSelectedTransaction
{
public:
    PerspectiveTransformCmd(KisToolPerspectiveTransform *tool, KisPaintDeviceSP device, KisPaintDeviceSP origDevice,  QPointF topleft, QPointF topright, QPointF bottomleft, QPointF bottomright, KisSelectionSP origSel, QRect initialRect);
    virtual ~PerspectiveTransformCmd();

public:
    virtual void execute();
    virtual void unexecute();
    void transformArgs(QPointF &topleft, QPointF &topright, QPointF &bottomleft, QPointF& bottomright) const;
    KisSelectionSP origSelection(QRect& initialRect) const;
    KisPaintDeviceSP theDevice();
    KisPaintDeviceSP origDevice();

private:
    QRect m_initialRect;
    QPointF m_topleft, m_topright, m_bottomleft, m_bottomright;
    KisToolPerspectiveTransform *m_tool;
    KisSelectionSP m_origSelection;
    KisPaintDeviceSP m_device;
    KisPaintDeviceSP m_origDevice;
};

PerspectiveTransformCmd::PerspectiveTransformCmd(KisToolPerspectiveTransform *tool, KisPaintDeviceSP device, KisPaintDeviceSP origDevice, QPointF topleft, QPointF topright, QPointF bottomleft, QPointF bottomright, KisSelectionSP origSel, QRect initialRect) :
        KisSelectedTransaction(i18n("Perspective Transform"), device), m_initialRect(initialRect)
        , m_topleft(topleft), m_topright(topright), m_bottomleft(bottomleft), m_bottomright(bottomright)
        , m_tool(tool), m_origSelection(origSel), m_device(device), m_origDevice(origDevice)
{
}

PerspectiveTransformCmd::~PerspectiveTransformCmd()
{
}

void PerspectiveTransformCmd::transformArgs(QPointF &topleft, QPointF &topright, QPointF &bottomleft, QPointF& bottomright) const
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
    KisSelectedTransaction::execute();
}

void PerspectiveTransformCmd::unexecute()
{
    KisSelectedTransaction::unexecute();
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
        : KisToolNonPaint(i18n("Perspective Transform"))
{
    setName("tool_perspectivetransform");
    setCursor(KisCursor::selectCursor());
    m_subject = 0;
    m_origDevice = 0;
    m_origSelection = 0;
    m_handleHalfSize = 8;
    m_handleSize = 2 * m_handleHalfSize;
    m_handleSelected = NOHANDLE;
}

KisToolPerspectiveTransform::~KisToolPerspectiveTransform()
{
}

void KisToolPerspectiveTransform::deactivate()
{
    if (!m_subject) return;

    if (m_subject->undoAdapter()) m_subject->undoAdapter()->removeCommandHistoryListener(this);


    if (!m_currentImage) return;

    paintOutline();

}

void KisToolPerspectiveTransform::activate()
{
    KisToolNonPaint::activate();
    m_currentSelectedPoint = 0;
    if (m_subject && m_currentImage && currentNode()->paintDevice()) {
        //connect(m_subject, commandExecuted(K3Command *c), this, notifyCommandAdded( KCommand * c));
        m_subject->undoAdapter()->setCommandHistoryListener(this);

        PerspectiveTransformCmd * cmd = 0;

        if (m_currentImage->undoAdapter()->presentCommand())
            cmd = dynamic_cast<PerspectiveTransformCmd*>(m_currentImage->undoAdapter()->presentCommand());

        // One of our commands is on top
        if (cmd && cmd->theDevice() == currentNode()->paintDevice()) {
            m_interractionMode = EDITRECTINTERRACTION;
            // and it even has the same device
            // We should ask for tool args and orig selection
            m_origDevice = cmd->origDevice();
            cmd->transformArgs(m_topleft, m_topright, m_bottomleft, m_bottomright);
            m_origSelection = cmd->origSelection(m_initialRect);
            paintOutline();
        } else {
            m_interractionMode = DRAWRECTINTERRACTION;
            m_points.clear();
            initHandles();
        }
    }
}

void KisToolPerspectiveTransform::initHandles()
{
//     qint32 x,y,w,h;


    KisPaintDeviceSP dev = currentNode()->paintDevice();
    if (!dev) return;

    // Create a lazy copy of the current state
    m_origDevice = new KisPaintDevice(*dev.data());
    Q_ASSERT(m_origDevice);

    if (dev->hasSelection()) {
        KisSelectionSP sel = dev->selection();
        m_origSelection = new KisSelection(*sel.data());
        m_initialRect = sel->selectedExactRect();
    } else {
        m_initialRect = dev->exactBounds();
    }
    m_topleft = m_initialRect.topLeft();
    m_topright = m_initialRect.topRight();
    m_bottomleft = m_initialRect.bottomLeft();
    m_bottomright = m_initialRect.bottomRight();

    m_subject->canvasController() ->updateCanvas();
}

void KisToolPerspectiveTransform::paint(QPainter& gc)
{
    paintOutline(gc, QRect());
}

void KisToolPerspectiveTransform::paint(QPainter& gc, const QRect& rc)
{
    paintOutline(gc, rc);
}

bool KisToolPerspectiveTransform::mouseNear(const QPoint& mousep, const QPoint point)
{
    return (QRect((point.x() - m_handleHalfSize), (point.y() - m_handleHalfSize), m_handleSize, m_handleSize).contains(mousep));
}

void KisToolPerspectiveTransform::mousePressEvent(KoPointerEvent *event)
{
    if (m_subject) {
        switch (m_interractionMode) {
        case DRAWRECTINTERRACTION: {
            if (m_points.isEmpty()) {
                m_dragging = false;
                m_dragStart = event->pos().toPointF();
                m_dragEnd = event->pos().toPointF();
                m_points.append(m_dragStart);
                paintOutline();
            } else {
                m_dragging = true;
                m_dragStart = m_dragEnd;
                m_dragEnd = event->pos().toPointF();
                paintOutline();
            }
        }
        case EDITRECTINTERRACTION: {


            if (m_currentImage && currentNode()->paintDevice() && event->button() == Qt::LeftButton) {
                m_actualyMoveWhileSelected = false;
                m_dragEnd = event->pos().toPointF();
                KisCanvasController *controller = m_subject->canvasController();
                QPoint mousep = controller->windowToView(event->pos().roundQPoint());
                if (mouseNear(mousep, controller->windowToView(m_topleft.toPoint()))) {
                    dbgPlugins << " PRESS TOPLEFT HANDLE";
                    m_currentSelectedPoint = &m_topleft;
                } else if (mouseNear(mousep, controller->windowToView(m_topright.toPoint()))) {
                    dbgPlugins << " PRESS TOPRIGHT HANDLE";
                    m_currentSelectedPoint = &m_topright;
                } else if (mouseNear(mousep, controller->windowToView(m_bottomleft.toPoint()))) {
                    dbgPlugins << " PRESS BOTTOMLEFT HANDLE";
                    m_currentSelectedPoint = &m_bottomleft;
                } else if (mouseNear(mousep, controller->windowToView(m_bottomright.toPoint()))) {
                    dbgPlugins << " PRESS BOTTOMRIGHT HANDLE";
                    m_currentSelectedPoint = &m_bottomright;
                } else if (mouseNear(mousep, controller->windowToView(QPointF((m_topleft + m_topright)*0.5).toPoint()))) {
                    dbgPlugins << " PRESS TOP HANDLE";
                    m_handleSelected = TOPHANDLE;
                } else if (mouseNear(mousep, controller->windowToView(QPointF((m_topleft + m_bottomleft)*0.5).toPoint()))) {
                    dbgPlugins << " PRESS LEFT HANDLE";
                    m_handleSelected = LEFTHANDLE;
                } else if (mouseNear(mousep, controller->windowToView(QPointF((m_bottomleft + m_bottomright)*0.5).toPoint()))) {
                    dbgPlugins << " PRESS BOTTOM HANDLE";
                    m_handleSelected = BOTTOMHANDLE;
                } else if (mouseNear(mousep, controller->windowToView(QPointF((m_bottomright + m_topright)*0.5).toPoint()))) {
                    dbgPlugins << " PRESS RIGHT HANDLE";
                    m_handleSelected = RIGHTHANDLE;
                } else if (mouseNear(mousep, controller->windowToView(QPointF((m_topleft + m_bottomleft + m_bottomright + m_topright)*0.25).toPoint()))) {
                    dbgPlugins << " PRESS MIDDLE HANDLE";
                    m_handleSelected = MIDDLEHANDLE;
                }
            }
        }
        }
    }
}

void KisToolPerspectiveTransform::mouseMoveEvent(KoPointerEvent *event)
{
    switch (m_interractionMode) {
    case DRAWRECTINTERRACTION: {
        if (m_dragging) {
            // erase old lines on canvas
            paintOutline();
            // get current mouse position
            m_dragEnd = event->pos().toPointF();
            // draw new lines on canvas
            paintOutline();
        }
    }

    case EDITRECTINTERRACTION: {
        if (m_currentSelectedPoint) {
            paintOutline();
            QPointF translate = event->pos().toPointF() - m_dragEnd;
            m_dragEnd = event->pos().toPointF();
            *m_currentSelectedPoint += translate;;
            paintOutline();
            m_actualyMoveWhileSelected = true;
        } else if (m_handleSelected == TOPHANDLE || m_handleSelected == LEFTHANDLE || m_handleSelected == BOTTOMHANDLE || m_handleSelected == RIGHTHANDLE) {
            paintOutline();

            QPointF translate = event->pos().toPointF() - m_dragEnd;
            m_dragEnd = event->pos().toPointF();

            Matrix3qreal matrixFrom = KisPerspectiveMath::computeMatrixTransfoToPerspective(m_topleft, m_topright, m_bottomleft, m_bottomright, m_initialRect);

            QPointF topLeft = KisPerspectiveMath::matProd(matrixFrom, m_initialRect.topLeft());
            QPointF topRight = KisPerspectiveMath::matProd(matrixFrom, m_initialRect.topRight());
            QPointF bottomLeft = KisPerspectiveMath::matProd(matrixFrom, m_initialRect.bottomLeft());
            QPointF bottomRight = KisPerspectiveMath::matProd(matrixFrom, m_initialRect.bottomRight());
            QRect dstRect = m_initialRect;
            switch (m_handleSelected) {
            case TOPHANDLE:
                dstRect.setTop(static_cast<int>(dstRect.top() + translate.y())) ;
                break;
            case LEFTHANDLE:
                dstRect.setLeft(static_cast<int>(dstRect.left() + translate.x()));
                break;
            case BOTTOMHANDLE:
                dstRect.setBottom(static_cast<int>(dstRect.bottom() + translate.y()));
                break;
            case RIGHTHANDLE:
                dstRect.setRight(static_cast<int>(dstRect.right() + translate.x()));
                break;
            case MIDDLEHANDLE:
            case NOHANDLE:
                dbgPlugins << "Should NOT happen";
            }
            Matrix3qreal matrixTo = KisPerspectiveMath::computeMatrixTransfoToPerspective(topLeft, topRight, bottomLeft, bottomRight, dstRect);
            m_topleft = KisPerspectiveMath::matProd(matrixTo, m_initialRect.topLeft());
            m_topright = KisPerspectiveMath::matProd(matrixTo, m_initialRect.topRight());
            m_bottomleft = KisPerspectiveMath::matProd(matrixTo, m_initialRect.bottomLeft());
            m_bottomright = KisPerspectiveMath::matProd(matrixTo, m_initialRect.bottomRight());

            paintOutline();
            m_actualyMoveWhileSelected = true;
        } else if (m_handleSelected == MIDDLEHANDLE) {
            paintOutline();
            QPointF translate = event->pos().toPointF() - m_dragEnd;
            m_dragEnd = event->pos().toPointF();
            m_topleft += translate;
            m_topright += translate;
            m_bottomleft += translate;
            m_bottomright += translate;
            paintOutline();
            m_actualyMoveWhileSelected = true;
        }
    }
    };
}

void KisToolPerspectiveTransform::mouseReleaseEvent(KoPointerEvent * event)
{


    if (!m_currentImage)
        return;
    if (event->button() == Qt::LeftButton) {
        switch (m_interractionMode) {
        case DRAWRECTINTERRACTION: {
            if (m_dragging && event->button() == Qt::LeftButton)  {
                paintOutline();
                m_dragging = false;
                m_points.append(m_dragEnd);
                if (m_points.size() == 4) {
                    // from the points, select which is topleft ? topright ? bottomright ? and bottomleft ?
                    m_topleft = m_points[0];
                    m_topright  = m_points[1];
                    m_bottomleft  = m_points[3];
                    m_bottomright  = m_points[2];
                    Matrix3qreal matrix = KisPerspectiveMath::computeMatrixTransfoToPerspective(m_topleft, m_topright, m_bottomleft, m_bottomright, m_initialRect);
                    m_topleft = KisPerspectiveMath::matProd(matrix, m_initialRect.topLeft());
                    m_topright = KisPerspectiveMath::matProd(matrix, m_initialRect.topRight());
                    m_bottomleft = KisPerspectiveMath::matProd(matrix, m_initialRect.bottomLeft());
                    m_bottomright = KisPerspectiveMath::matProd(matrix, m_initialRect.bottomRight());
                    m_interractionMode = EDITRECTINTERRACTION;
                    paintOutline();
                    QApplication::setOverrideCursor(KisCursor::waitCursor());
                    transform();
                    QApplication::restoreOverrideCursor();
                } else {
                    paintOutline();
                }
            }
        }
        break;
        case EDITRECTINTERRACTION: {
            if (m_currentSelectedPoint) {
                m_currentSelectedPoint = 0;
                if (m_actualyMoveWhileSelected) {
                    paintOutline();
                    QApplication::setOverrideCursor(KisCursor::waitCursor());
                    transform();
                    QApplication::restoreOverrideCursor();
                }
            }
            if (m_handleSelected != NOHANDLE) {
                m_handleSelected = NOHANDLE;
                if (m_actualyMoveWhileSelected) {
//                         paintOutline();
                    QApplication::setOverrideCursor(KisCursor::waitCursor());
                    transform();
                    QApplication::restoreOverrideCursor();
                }
            }
        }
        break;
        }
    }
}

void KisToolPerspectiveTransform::paintOutline()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisCanvas *canvas = controller->kiscanvas();
        QPainter gc(canvas->canvasWidget());
        QRect rc;

        paintOutline(gc, rc);
    }
}

void KisToolPerspectiveTransform::paintOutline(QPainter& gc, const QRect&)
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);
        pen.setWidth(1);
        Q_ASSERT(controller);

        switch (m_interractionMode) {
        case DRAWRECTINTERRACTION: {
            dbgPlugins << "DRAWRECTINTERRACTION paintOutline" << m_points.size();
            QPointF start, end;
            QPoint startPos;
            QPoint endPos;
            for (QPointFVector::iterator it = m_points.begin(); it != m_points.end(); ++it) {

                if (it == m_points.begin()) {
                    start = (*it);
                } else {
                    end = (*it);

                    startPos = controller->windowToView(start.toPoint());
                    endPos = controller->windowToView(end.toPoint());

                    gc.drawLine(startPos, endPos);

                    start = end;
                }
            }
        }
        break;
        case EDITRECTINTERRACTION: {
            QPoint topleft = controller->windowToView(m_topleft).roundQPoint();
            QPoint topright = controller->windowToView(m_topright).roundQPoint();
            QPoint bottomleft = controller->windowToView(m_bottomleft).roundQPoint();
            QPoint bottomright = controller->windowToView(m_bottomright).roundQPoint();

            gc.setPen(pen);
            gc.drawRect(topleft.x() - 4, topleft.y() - 4, 8, 8);
            gc.drawLine(topleft.x(), topleft.y(), (topleft.x() + topright.x()) / 2, (topleft.y() + topright.y()) / 2);
            gc.drawRect((topleft.x() + topright.x()) / 2 - 4, (topleft.y() + topright.y()) / 2 - 4, 8, 8);
            gc.drawLine((topleft.x() + topright.x()) / 2, (topleft.y() + topright.y()) / 2, topright.x(), topright.y());
            gc.drawRect(topright.x() - 4, topright.y() - 4, 8, 8);
            gc.drawLine(topright.x(), topright.y(), (topright.x() + bottomright.x()) / 2, (topright.y() + bottomright.y()) / 2);
            gc.drawRect((topright.x() + bottomright.x()) / 2 - 4, (topright.y() + bottomright.y()) / 2 - 4, 8, 8);
            gc.drawLine((topright.x() + bottomright.x()) / 2, (topright.y() + bottomright.y()) / 2, bottomright.x(), bottomright.y());
            gc.drawRect(bottomright.x() - 4, bottomright.y() - 4, 8, 8);
            gc.drawLine(bottomright.x(), bottomright.y(), (bottomleft.x() + bottomright.x()) / 2, (bottomleft.y() + bottomright.y()) / 2);
            gc.drawRect((bottomleft.x() + bottomright.x()) / 2 - 4, (bottomleft.y() + bottomright.y()) / 2 - 4, 8, 8);
            gc.drawLine((bottomleft.x() + bottomright.x()) / 2, (bottomleft.y() + bottomright.y()) / 2, bottomleft.x(), bottomleft.y());
            gc.drawRect(bottomleft.x() - 4, bottomleft.y() - 4, 8, 8);
            gc.drawLine(bottomleft.x(), bottomleft.y(), (topleft.x() + bottomleft.x()) / 2, (topleft.y() + bottomleft.y()) / 2);
            gc.drawRect((topleft.x() + bottomleft.x()) / 2 - 4, (topleft.y() + bottomleft.y()) / 2 - 4, 8, 8);
            gc.drawLine((topleft.x() + bottomleft.x()) / 2, (topleft.y() + bottomleft.y()) / 2, topleft.x(), topleft.y());
            gc.drawRect((bottomleft.x() + bottomright.x() + topleft.x() + topright.x()) / 4 - 4, (bottomleft.y() + bottomright.y() + topleft.y() + topright.y()) / 4 - 4, 8, 8);
        }
        break;
        }
        gc.setPen(old);
    }
}

void KisToolPerspectiveTransform::transform()
{


    if (!m_currentImage || !currentNode()->paintDevice())
        return;

    KoUpdater *progress = m_subject->progressDisplay();

    // This mementoes the current state of the active device.
    PerspectiveTransformCmd * transaction = new PerspectiveTransformCmd(this, currentNode()->paintDevice(), m_origDevice,
            m_topleft, m_topright, m_bottomleft, m_bottomright, m_origSelection, m_initialRect);

    // Copy the original state back.
    QRect rc = m_origDevice->extent();
    rc = rc.normalize();
    currentNode()->paintDevice()->clear();
    KisPainter gc(currentNode()->paintDevice());
    gc.bitBlt(rc.x(), rc.y(), COMPOSITE_COPY, m_origDevice, rc.x(), rc.y(), rc.width(), rc.height());
    gc.end();

    // Also restore the original selection.
    if (m_origSelection) {
        QRect rc = m_origSelection->extent();
        rc = rc.normalize();
        currentNode()->paintDevice()->selection()->clear();
        KisPainter sgc(currentNode()->paintDevice()->selection().data());
        sgc.bitBlt(rc.x(), rc.y(), COMPOSITE_COPY, m_origSelection.data(), rc.x(), rc.y(), rc.width(), rc.height());
        sgc.end();
    } else if (currentNode()->paintDevice()->hasSelection())
        currentNode()->paintDevice()->selection()->clear();

    // Perform the transform. Since we copied the original state back, this doesn't degrade
    // after many tweaks. Since we started the transaction before the copy back, the memento
    // has the previous state.
    KisPerspectiveTransformWorker t(currentNode()->paintDevice(), m_topleft, m_topright, m_bottomleft, m_bottomright, progress);
    t.run();

    // If canceled, go back to the memento
    if (t.isCanceled()) {
        transaction->unexecute();
        delete transaction;
        return;
    }

    currentNode()->paintDevice()->setDirty(rc); // XXX: This is not enough - should union with new extent

    // Else add the command -- this will have the memento from the previous state,
    // and the transformed state from the original device we cached in our activated()
    // method.
    if (transaction) {
        if (m_currentImage->undo())
            m_currentImage->undoAdapter()->addCommandOld(transaction);
        else
            delete transaction;
    }
}

void KisToolPerspectiveTransform::notifyCommandAdded(const QUndoCommand * command)
{
    PerspectiveTransformCmd * cmd = dynamic_cast<PerspectiveTransformCmd*>(command);
    if (cmd == 0) {
        // The last added command wasn't one of ours;
        // we should reset to the new state of the canvas.
        // In effect we should treat this as if the tool has been just activated
        initHandles();
    }
}

void KisToolPerspectiveTransform::notifyCommandExecuted(const QUndoCommand * command)
{
    Q_UNUSED(command);
    PerspectiveTransformCmd * cmd = 0;
    if (m_currentImage->undoAdapter()->presentCommand())
        cmd = dynamic_cast<PerspectiveTransformCmd*>(m_currentImage->undoAdapter()->presentCommand());

    if (cmd == 0) {
        // The command now on the top of the stack isn't one of ours
        // We should treat this as if the tool has been just activated
        initHandles();
    } else {
        // One of our commands is now on top
        // We should ask for tool args and orig selection
        m_origDevice = cmd->origDevice();
        cmd->transformArgs(m_topleft, m_topright, m_bottomleft, m_bottomright);
        m_origSelection = cmd->origSelection(m_initialRect);
        m_subject->canvasController() ->updateCanvas();
    }
}
#if 0
QWidget* KisToolPerspectiveTransform::createOptionWidget()
{

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

    return 0;
}

QWidget* KisToolPerspectiveTransform::optionWidget()
{
    return 0;
}
#endif

void KisToolPerspectiveTransform::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("tool_perspectivetransform"),
                               i18n("&Perspective Transform"),
                               collection,
                               objectName());
        Q_CHECK_PTR(m_action);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setToolTip(i18n("Perspective transform a layer or a selection"));
        m_action->setActionGroup(actionGroup());
        m_ownAction = true;
    }
}

#include "kis_tool_perspectivetransform.moc"
