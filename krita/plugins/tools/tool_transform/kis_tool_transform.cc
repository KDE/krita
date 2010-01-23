/*
 *  kis_tool_transform.cc -- part of Krita
 *
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

#include "kis_tool_transform.h"


#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QApplication>

#include <kis_debug.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <klocale.h>
#include <knuminput.h>

#include <KoPointerEvent.h>
#include <KoID.h>
#include <KoCanvasBase.h>
#include <KoViewConverter.h>
#include <KoSelection.h>
#include <KoCompositeOp.h>
#include <KoShapeManager.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_global.h>
#include <canvas/kis_canvas2.h>
#include <kis_view2.h>
#include <kis_painter.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_selected_transaction.h>
#include <kis_selection.h>
#include <kis_filter_strategy.h>
#include <widgets/kis_cmb_idlist.h>
#include <kis_statusbar.h>
#include <kis_transform_worker.h>
#include <kis_pixel_selection.h>
#include <kis_shape_selection.h>
#include <kis_selection_manager.h>

#include <KoShapeTransformCommand.h>

#include "flake/kis_node_shape.h"
#include "flake/kis_layer_container_shape.h"
#include "flake/kis_shape_layer.h"
#include "kis_canvas_resource_provider.h"
#include "widgets/kis_progress_widget.h"

namespace
{
class TransformCmd : public KisSelectedTransaction
{

public:
    TransformCmd(KisToolTransform *tool, KisNodeSP node, double scaleX, double scaleY, QPointF translate, double a, KisSelectionSP origSel, QPoint startPos, QPoint endPos);
    virtual ~TransformCmd();

public:
    virtual void redo();
    virtual void undo();
    void transformArgs(double &sx, double &sy, QPointF &translate, double &a) const;
    KisSelectionSP origSelection(QPoint &startPos, QPoint &endPos) const;
    void setNewPosition(int x, int y);
private:
    double m_scaleX;
    double m_scaleY;
    QPointF m_translate;
    double m_a;
    KisToolTransform *m_tool;
    KisSelectionSP m_origSelection;
    QPoint m_originalTopLeft;
    QPoint m_originalBottomRight;
    QPoint m_newPosition;
};

TransformCmd::TransformCmd(KisToolTransform *tool, KisNodeSP node, double scaleX, double scaleY, QPointF translate, double a, KisSelectionSP origSel, QPoint originalTopLeft, QPoint originalBottomRight)
        : KisSelectedTransaction(i18n("Transform"), node)
        , m_scaleX(scaleX)
        , m_scaleY(scaleY)
        , m_translate(translate)
        , m_a(a)
        , m_tool(tool)
        , m_origSelection(origSel)
        , m_originalTopLeft(originalTopLeft)
        , m_originalBottomRight(originalBottomRight)
{
}

TransformCmd::~TransformCmd()
{
}

void TransformCmd::transformArgs(double &sx, double &sy, QPointF &translate, double &a) const
{
    sx = m_scaleX;
    sy = m_scaleY;
    translate = m_translate;
    a = m_a;
}

KisSelectionSP TransformCmd::origSelection(QPoint &originalTopLeft, QPoint &originalBottomRight) const
{
    originalTopLeft = m_originalTopLeft;
    originalBottomRight = m_originalBottomRight;
    return m_origSelection;
}

void TransformCmd::redo()
{
    KisSelectedTransaction::redo();
    layer()->paintDevice()->move(m_newPosition);
}

void TransformCmd::undo()
{
    KisSelectedTransaction::undo();
    layer()->paintDevice()->move(m_originalTopLeft);
}

void TransformCmd::setNewPosition(int x, int y)
{
    m_newPosition.setX(x);
    m_newPosition.setY(y);
}

}

KisToolTransform::KisToolTransform(KoCanvasBase * canvas)
        : KisTool(canvas, KisCursor::rotateCursor())
        , m_canvas(canvas)
{
    setObjectName("tool_transform");
    useCursor(KisCursor::selectCursor());
    m_selecting = false;
    m_originalTopLeft = QPoint(0, 0);
    m_originalBottomRight = QPoint(0, 0);
    m_optWidget = 0;
    m_sizeCursors[0] = KisCursor::sizeVerCursor();
    m_sizeCursors[1] = KisCursor::sizeBDiagCursor();
    m_sizeCursors[2] = KisCursor::sizeHorCursor();
    m_sizeCursors[3] = KisCursor::sizeFDiagCursor();
    m_sizeCursors[4] = KisCursor::sizeVerCursor();
    m_sizeCursors[5] = KisCursor::sizeBDiagCursor();
    m_sizeCursors[6] = KisCursor::sizeHorCursor();
    m_sizeCursors[7] = KisCursor::sizeFDiagCursor();
    m_origDevice = 0;
    m_origSelection = 0;
}

KisToolTransform::~KisToolTransform()
{
}

void KisToolTransform::deactivate()
{
    if (image()->undoAdapter())
        image()->undoAdapter()->removeCommandHistoryListener(this);

    if (image()) return;

    m_canvas->updateCanvas(QRect(m_originalTopLeft, m_originalBottomRight));
}

void KisToolTransform::activate(bool temporary)
{
    Q_UNUSED(temporary);

    if (currentNode() && currentNode()->paintDevice()) {
        image()->undoAdapter()->setCommandHistoryListener(this);

        const TransformCmd * cmd = 0;

        if (image()->undoAdapter()->presentCommand())
            cmd = dynamic_cast<const TransformCmd*>(image()->undoAdapter()->presentCommand());

        if (cmd == 0) {
            initHandles();
        } else {
            // One of our commands is on top
            // We should ask for tool args and orig selection
            cmd->transformArgs(m_scaleX, m_scaleY, m_translate, m_a);
            m_origSelection = cmd->origSelection(m_originalTopLeft, m_originalBottomRight);
            m_canvas->updateCanvas(QRect(m_originalTopLeft, m_originalBottomRight));
        }
    }
    currentNode() =
        m_canvas->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
}

void KisToolTransform::initHandles()
{
    int x, y, w, h;

    KisPaintDeviceSP dev = currentNode()->paintDevice();

    // Create a lazy copy of the current state
    m_origDevice = new KisPaintDevice(*dev.data());
    Q_ASSERT(m_origDevice);

    KisSelectionSP selection = currentSelection();
    if (selection) {
        QRect r = selection->selectedExactRect();
        m_origSelection = new KisSelection();
        KisPixelSelectionSP origPixelSelection = new KisPixelSelection(*selection->getOrCreatePixelSelection().data());
        m_origSelection->setPixelSelection(origPixelSelection);
        r.getRect(&x, &y, &w, &h);
    } else {
        dev->exactBounds(x, y, w, h);
        m_origSelection = 0;
    }
    m_originalTopLeft = QPoint(x, y);
    m_originalBottomRight = QPoint(x + w - 1, y + h - 1);
    m_originalCenter = QPointF(m_originalTopLeft + m_originalBottomRight + QPoint(1, 1)) / 2.0;

    m_a = 0.0;
    m_scaleX = 1.0;
    m_scaleY = 1.0;
    m_translate = m_originalCenter;

    m_canvas->updateCanvas(QRect(m_originalTopLeft, m_originalBottomRight));
}

void KisToolTransform::mousePressEvent(KoPointerEvent *e)
{
    KisImageWSP kisimage = image();

    if (!currentNode())
        return;

    if (kisimage && currentNode()->paintDevice() && e->button() == Qt::LeftButton) {
        QPointF mousePos = QPointF(e->point.x() * kisimage->xRes(), e->point.y() * kisimage->yRes());
        switch (m_function) {
        case ROTATE:
            m_clickoffset = mousePos - m_translate;
            m_clickangle = -m_a - atan2(m_clickoffset.x(), m_clickoffset.y());
            m_clickoffset = QPoint(0, 0);
            break;
        case MOVE:
            m_clickoffset = mousePos - m_translate;
            break;
        case TOPSCALE:
            m_clickoffset = mousePos - (m_topleft + m_topright) / 2.0;
            break;
        case TOPRIGHTSCALE:
            m_clickoffset = mousePos - m_topright;
            break;
        case RIGHTSCALE:
            m_clickoffset = mousePos - (m_topright + m_bottomright) / 2.0;
            break;
        case BOTTOMRIGHTSCALE:
            m_clickoffset = mousePos - m_bottomright;
            break;
        case BOTTOMSCALE:
            m_clickoffset = mousePos - (m_bottomleft + m_bottomright) / 2.0;
            break;
        case BOTTOMLEFTSCALE:
            m_clickoffset = mousePos - m_bottomleft;
            break;
        case LEFTSCALE:
            m_clickoffset = mousePos - (m_topleft + m_bottomleft) / 2.0;
            break;
        case TOPLEFTSCALE:
            m_clickoffset = mousePos - m_topleft;
            break;
        }
        m_selecting = true;
        m_actualyMoveWhileSelected = false;
    }
}

int KisToolTransform::det(const QPointF & v, const QPointF & w)
{
    return int(v.x()*w.y() - v.y()*w.x());
}

double KisToolTransform::distsq(const QPointF & v, const QPointF & w)
{
    QPointF v2 = v - w;
    return v2.x()*v2.x() + v2.y()*v2.y();
}

void KisToolTransform::setFunctionalCursor()
{
    int rotOctant = 8 + int(8.5 + m_a * 4 / M_PI);

    int s;
    if ((m_scaleX * m_scaleY) < 0)
        s = -1;
    else
        s = 1;

    switch (m_function) {
    case MOVE:
        useCursor(KisCursor::moveCursor());
        break;
    case ROTATE:
        useCursor(KisCursor::rotateCursor());
        break;
    case TOPSCALE:
        useCursor(m_sizeCursors[(0*s +rotOctant)%8]);
        break;
    case TOPRIGHTSCALE:
        useCursor(m_sizeCursors[(1*s +rotOctant)%8]);
        break;
    case RIGHTSCALE:
        useCursor(m_sizeCursors[(2*s +rotOctant)%8]);
        break;
    case BOTTOMRIGHTSCALE:
        useCursor(m_sizeCursors[(3*s +rotOctant)%8]);
        break;
    case BOTTOMSCALE:
        useCursor(m_sizeCursors[(4*s +rotOctant)%8]);
        break;
    case BOTTOMLEFTSCALE:
        useCursor(m_sizeCursors[(5*s +rotOctant)%8]);
        break;
    case LEFTSCALE:
        useCursor(m_sizeCursors[(6*s +rotOctant)%8]);
        break;
    case TOPLEFTSCALE:
        useCursor(m_sizeCursors[(7*s +rotOctant)%8]);
        break;
    }
}

void KisToolTransform::mouseMoveEvent(KoPointerEvent *e)
{
    QPointF topleft = m_topleft;
    QPointF topright = m_topright;
    QPointF bottomleft = m_bottomleft;
    QPointF bottomright = m_bottomright;

    KisImageWSP kisimage = image();
    QPointF mousePos = QPointF(e->point.x() * kisimage->xRes(), e->point.y() * kisimage->yRes());

    if (m_selecting) {
        m_canvas->updateCanvas(QRect(m_originalTopLeft, m_originalBottomRight));

        m_actualyMoveWhileSelected = true;

        mousePos -= m_clickoffset;

        // transform mousePos coords, so it seems like it isn't rotated and centered at 0,0
        QPointF newpos = invrot(mousePos.x() - m_translate.x(), mousePos.y() - m_translate.y());
        double dx = 0, dy = 0;
        double oldScaleX = m_scaleX;
        double oldScaleY = m_scaleY;

        if (m_function == MOVE) {
            m_translate += mousePos - m_translate;
        }

        if (m_function == ROTATE) {
            m_a = -atan2(mousePos.x() - m_translate.x(), mousePos.y() - m_translate.y())
                  - m_clickangle;
        }

        if (m_function == TOPSCALE) {
            dy = (newpos.y() - m_scaleY * (m_originalTopLeft.y() - m_originalCenter.y())) / 2;
            m_scaleY = (newpos.y() - dy) / (m_originalTopLeft.y() - m_originalCenter.y());

            // enforce same acpect if shift button is pressed
            if (e->modifiers() & Qt::ShiftModifier) {
                if (m_scaleX > 0) // handle the mirrored cases
                    m_scaleX = fabs(m_scaleY);
                else
                    m_scaleX = -fabs(m_scaleY);
            }
        }

        if (m_function == TOPRIGHTSCALE) {
            dx = (newpos.x() - m_scaleX * (m_originalBottomRight.x() - m_originalCenter.x())) / 2;
            m_scaleX = (newpos.x() - dx) / (m_originalBottomRight.x() - m_originalCenter.x());

            dy = (newpos.y() - m_scaleY * (m_originalTopLeft.y() - m_originalCenter.y())) / 2;
            m_scaleY = (newpos.y() - dy) / (m_originalTopLeft.y() - m_originalCenter.y());

            // enforce same aspect if shift button is pressed
            if (e->modifiers() & Qt::ShiftModifier) {
                if (m_scaleX < m_scaleY) {
                    if (m_scaleX > 0) // handle the mirrored cases
                        m_scaleX = fabs(m_scaleY);
                    else
                        m_scaleX = -fabs(m_scaleY);
                    dx = (m_scaleX - oldScaleX) * (m_originalBottomRight.x() - m_originalCenter.x());
                } else {
                    if (m_scaleY > 0) // handle the mirrored cases
                        m_scaleY = fabs(m_scaleX);
                    else
                        m_scaleY = -fabs(m_scaleX);
                    dy = (m_scaleY - oldScaleY) * (m_originalTopLeft.y() - m_originalCenter.y());
                }
            }
        }

        if (m_function == RIGHTSCALE) {
            dx = (newpos.x() - m_scaleX * (m_originalBottomRight.x() - m_originalCenter.x())) / 2;
            m_scaleX = (newpos.x() - dx) / (m_originalBottomRight.x() - m_originalCenter.x());

            // enforce same acpect if shift button is pressed
            if (e->modifiers() & Qt::ShiftModifier) {
                if (m_scaleY > 0) // handle the mirrored cases
                    m_scaleY = fabs(m_scaleX);
                else
                    m_scaleY = -fabs(m_scaleX);
            }
        }

        if (m_function == BOTTOMRIGHTSCALE) {
            dx = (newpos.x() - m_scaleX * (m_originalBottomRight.x() - m_originalCenter.x())) / 2;
            m_scaleX = (newpos.x() - dx) / (m_originalBottomRight.x() - m_originalCenter.x());

            dy = (newpos.y() - m_scaleY * (m_originalBottomRight.y() - m_originalCenter.y())) / 2;
            m_scaleY = (newpos.y() - dy) / (m_originalBottomRight.y() - m_originalCenter.y());

            // enforce same acpect if shift button is pressed
            if (e->modifiers() & Qt::ShiftModifier) {
                if (m_scaleX < m_scaleY) {
                    if (m_scaleX > 0) // handle the mirrored cases
                        m_scaleX = fabs(m_scaleY);
                    else
                        m_scaleX = -fabs(m_scaleY);
                    dx = (m_scaleX - oldScaleX) * (m_originalBottomRight.x() - m_originalCenter.x());
                } else {
                    if (m_scaleY > 0) // handle the mirrored cases
                        m_scaleY = fabs(m_scaleX);
                    else
                        m_scaleY = -fabs(m_scaleX);
                    dy = (m_scaleY - oldScaleY) * (m_originalBottomRight.y() - m_originalCenter.y());
                }
            }
        }

        if (m_function == BOTTOMSCALE) {
            dy = (newpos.y() - m_scaleY * (m_originalBottomRight.y() - m_originalCenter.y())) / 2;
            m_scaleY = (newpos.y() - dy) / (m_originalBottomRight.y() - m_originalCenter.y());

            // enforce same acpect if shift button is pressed
            if (e->modifiers() & Qt::ShiftModifier) {
                if (m_scaleX > 0) // handle the mirrored cases
                    m_scaleX = fabs(m_scaleY);
                else
                    m_scaleX = -fabs(m_scaleY);
            }
        }

        if (m_function == BOTTOMLEFTSCALE) {
            dx = (newpos.x() - m_scaleX * (m_originalTopLeft.x() - m_originalCenter.x())) / 2;
            m_scaleX = (newpos.x() - dx) / (m_originalTopLeft.x() - m_originalCenter.x());

            dy = (newpos.y() - m_scaleY * (m_originalBottomRight.y() - m_originalCenter.y())) / 2;
            m_scaleY = (newpos.y() - dy) / (m_originalBottomRight.y() - m_originalCenter.y());

            // enforce same acpect if shift button is pressed
            if (e->modifiers() & Qt::ShiftModifier) {
                if (m_scaleX < m_scaleY) {
                    if (m_scaleX > 0) // handle the mirrored cases
                        m_scaleX = fabs(m_scaleY);
                    else
                        m_scaleX = -fabs(m_scaleY);
                    dx = (m_scaleX - oldScaleX) * (m_originalTopLeft.x() - m_originalCenter.x());
                } else {
                    if (m_scaleY > 0) // handle the mirrored cases
                        m_scaleY = fabs(m_scaleX);
                    else
                        m_scaleY = -fabs(m_scaleX);
                    dy = (m_scaleY - oldScaleY) * (m_originalBottomRight.y() - m_originalCenter.y());
                }
            }
        }

        if (m_function == LEFTSCALE) {
            dx = (newpos.x() - m_scaleX * (m_originalTopLeft.x() - m_originalCenter.x())) / 2;
            m_scaleX = (newpos.x() - dx) / (m_originalTopLeft.x() - m_originalCenter.x());

            // enforce same acpect if shift button is pressed
            if (e->modifiers() & Qt::ShiftModifier) {
                if (m_scaleY > 0) // handle the mirrored cases
                    m_scaleY = fabs(m_scaleX);
                else
                    m_scaleY = -fabs(m_scaleX);
            }
        }

        if (m_function == TOPLEFTSCALE) {
            dx = (newpos.x() - m_scaleX * (m_originalTopLeft.x() - m_originalCenter.x())) / 2;
            m_scaleX = (newpos.x() - dx) / (m_originalTopLeft.x() - m_originalCenter.x());

            dy = (newpos.y() - m_scaleY * (m_originalTopLeft.y() - m_originalCenter.y())) / 2;
            m_scaleY = (newpos.y() - dy) / (m_originalTopLeft.y() - m_originalCenter.y());

            // enforce same acpect if shift button is pressed
            if (e->modifiers() & Qt::ShiftModifier) {
                if (m_scaleX < m_scaleY) {
                    if (m_scaleX > 0) // handle the mirrored cases
                        m_scaleX = fabs(m_scaleY);
                    else
                        m_scaleX = -fabs(m_scaleY);
                    dx = (m_scaleX - oldScaleX) * (m_originalTopLeft.x() - m_originalCenter.x());
                } else {
                    if (m_scaleY > 0) // handle the mirrored cases
                        m_scaleY = fabs(m_scaleX);
                    else
                        m_scaleY = -fabs(m_scaleX);
                    dy = (m_scaleY - oldScaleY) * (m_originalTopLeft.y() - m_originalCenter.y());
                }
            }
        }

        m_translate += rot(dx, dy);

        m_canvas->updateCanvas(QRect(m_originalTopLeft, m_originalBottomRight));
    } else {
        if (det(mousePos - topleft, topright - topleft) > 0)
            m_function = ROTATE;
        else if (det(mousePos - topright, bottomright - topright) > 0)
            m_function = ROTATE;
        else if (det(mousePos - bottomright, bottomleft - bottomright) > 0)
            m_function = ROTATE;
        else if (det(mousePos - bottomleft, topleft - bottomleft) > 0)
            m_function = ROTATE;
        else
            m_function = MOVE;

        double handleradius = m_canvas->viewConverter()->viewToDocumentX(5);
        handleradius *= handleradius; // square it so it fits with distsq

        if (distsq(mousePos, (m_topleft + m_topright) / 2.0) <= handleradius)
            m_function = TOPSCALE;
        if (distsq(mousePos, m_topright) <= handleradius)
            m_function = TOPRIGHTSCALE;
        if (distsq(mousePos, (m_topright + m_bottomright) / 2.0) <= handleradius)
            m_function = RIGHTSCALE;
        if (distsq(mousePos, m_bottomright) <= handleradius)
            m_function = BOTTOMRIGHTSCALE;
        if (distsq(mousePos, (m_bottomleft + m_bottomright) / 2.0) <= handleradius)
            m_function = BOTTOMSCALE;
        if (distsq(mousePos, m_bottomleft) <= handleradius)
            m_function = BOTTOMLEFTSCALE;
        if (distsq(mousePos, (m_topleft + m_bottomleft) / 2.0) <= handleradius)
            m_function = LEFTSCALE;
        if (distsq(mousePos, m_topleft) <= handleradius)
            m_function = TOPLEFTSCALE;

        setFunctionalCursor();
    }
}

void KisToolTransform::mouseReleaseEvent(KoPointerEvent */*e*/)
{
    m_selecting = false;

    if (m_actualyMoveWhileSelected) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());
        m_canvas->updateCanvas(QRect(m_originalTopLeft, m_originalBottomRight));
        transform();
        QApplication::restoreOverrideCursor();
    }
}

void KisToolTransform::recalcOutline()
{
    double x, y;

    m_sina = sin(m_a);
    m_cosa = cos(m_a);

    x = (m_originalTopLeft.x() - m_originalCenter.x()) * m_scaleX;
    y = (m_originalTopLeft.y() - m_originalCenter.y()) * m_scaleY;
    m_topleft = rot(x, y) + m_translate;

    x = (m_originalBottomRight.x() - m_originalCenter.x()) * m_scaleX;
    y = (m_originalTopLeft.y() - m_originalCenter.y()) * m_scaleY;
    m_topright = rot(x, y) + m_translate;

    x = (m_originalTopLeft.x() - m_originalCenter.x()) * m_scaleX;
    y = (m_originalBottomRight.y() - m_originalCenter.y()) * m_scaleY;
    m_bottomleft = rot(x, y) + m_translate;

    x = (m_originalBottomRight.x() - m_originalCenter.x()) * m_scaleX;
    y = (m_originalBottomRight.y() - m_originalCenter.y()) * m_scaleY;
    m_bottomright = rot(x, y) + m_translate;
}

void KisToolTransform::paint(QPainter& gc, const KoViewConverter &converter)
{
    QPen old = gc.pen();
    QPen pen(Qt::SolidLine);
    pen.setWidth(0);

    recalcOutline();
    KisImageWSP kisimage = image();
    QPointF topleft = converter.documentToView(QPointF(m_topleft.x() / kisimage->xRes(), m_topleft.y() / kisimage->yRes()));
    QPointF topright = converter.documentToView(QPointF(m_topright.x() / kisimage->xRes(), m_topright.y() / kisimage->yRes()));
    QPointF bottomleft = converter.documentToView(QPointF(m_bottomleft.x() / kisimage->xRes(), m_bottomleft.y() / kisimage->yRes()));
    QPointF bottomright = converter.documentToView(QPointF(m_bottomright.x() / kisimage->xRes(), m_bottomright.y() / kisimage->yRes()));

    QRectF handleRect(-4, -4, 8, 8);

    gc.setPen(pen);

    gc.drawRect(handleRect.translated(topleft));
    gc.drawRect(handleRect.translated((topleft + topright) / 2.0));
    gc.drawRect(handleRect.translated(topright));
    gc.drawRect(handleRect.translated((topright + bottomright) / 2.0));
    gc.drawRect(handleRect.translated(bottomright));
    gc.drawRect(handleRect.translated((bottomleft + bottomright) / 2.0));
    gc.drawRect(handleRect.translated(bottomleft));
    gc.drawRect(handleRect.translated((topleft + bottomleft) / 2.0));

    gc.drawLine(topleft, topright);
    gc.drawLine(topright, bottomright);
    gc.drawLine(bottomright, bottomleft);
    gc.drawLine(bottomleft, topleft);

    gc.setPen(old);
}

void KisToolTransform::transform()
{
    if (!image() || !currentNode()->paintDevice())
        return;

    KisCanvas2 *canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
    if (!canvas)
        return;

    QPointF t = m_translate - rot(m_originalCenter.x() * m_scaleX, m_originalCenter.y() * m_scaleY);
    KoProgressUpdater* updater = canvas->view()->createProgressUpdater();
    updater->start(100, i18n("Transform"));
    KoUpdaterPtr progress = updater->startSubtask();

    // This mementoes the current state of the active device.
    TransformCmd * transaction = new TransformCmd(this, currentNode(), m_scaleX,
            m_scaleY, m_translate, m_a, m_origSelection, m_originalTopLeft, m_originalBottomRight);


    //Copy the original state back.
    QRect rc = m_origDevice->extent();
    rc = rc.normalized();
    currentNode()->paintDevice()->clear();
    KisPainter gc(currentNode()->paintDevice());
    gc.setCompositeOp(COMPOSITE_COPY);
    gc.bitBlt(rc.topLeft(), m_origDevice, rc);
    gc.end();

    // Also restore the original selection.
    if (m_origSelection) {
        QRect rc = m_origSelection->selectedRect();
        rc = rc.normalized();
        if (currentSelection()) {
            currentSelection()->getOrCreatePixelSelection()->clear();
            KisPainter sgc(KisPaintDeviceSP(currentSelection()->getOrCreatePixelSelection()));
            sgc.setCompositeOp(COMPOSITE_COPY);
            sgc.bitBlt(rc.topLeft(), m_origSelection, rc);
            sgc.end();
        }
    } else if (currentSelection())
        currentSelection()->clear();

    // Perform the transform. Since we copied the original state back, this doesn't degrade
    // after many tweaks. Since we started the transaction before the copy back, the memento
    // has the previous state.
    if (m_origSelection) {
        KisPaintDeviceSP tmpDevice = new KisPaintDevice(m_origDevice->colorSpace());
        QRect selectRect = currentSelection()->selectedRect();
        KisPainter gc(tmpDevice, currentSelection());
        gc.bitBlt(selectRect.topLeft(), m_origDevice, selectRect);
        gc.end();

        KisTransformWorker worker(tmpDevice, m_scaleX, m_scaleY, 0, 0, m_a, int(t.x()), int(t.y()), progress, m_filter);
        worker.run();

        currentNode()->paintDevice()->clearSelection(currentSelection());

        KisTransformWorker selectionWorker(currentSelection()->getOrCreatePixelSelection(), m_scaleX, m_scaleY, 0, 0, m_a, int(t.x()), int(t.y()), progress, m_filter);
        selectionWorker.run();


        if (currentSelection()->hasShapeSelection()) {
            QMatrix resolutionMatrix;
            resolutionMatrix.scale(1 / image()->xRes(), 1 / image()->yRes());
            QPointF center = resolutionMatrix.map(m_originalCenter);

            QMatrix matrix;
            matrix.translate(center.x(), center.y());
            matrix.rotate(m_a / M_PI*180);
            matrix.translate(-center.x(), -center.y());

            KisShapeSelection* shapeSelection = static_cast<KisShapeSelection*>(currentSelection()->shapeSelection());
            QList<KoShape *> shapes = shapeSelection->shapeManager()->shapes();
            QList<QMatrix> m_oldMatrixList;
            QList<QMatrix> m_newMatrixList;
            foreach(KoShape *shape, shapes) {
                m_oldMatrixList << shape->transformation();
                m_newMatrixList << shape->transformation()*matrix;
            }
            KoShapeTransformCommand* cmd = new KoShapeTransformCommand(shapes, m_oldMatrixList, m_newMatrixList, transaction);
            cmd->redo();
        }

        QRect tmpRc = tmpDevice->extent();
        KisPainter painter(currentNode()->paintDevice());
        painter.bitBlt(tmpRc.topLeft(), tmpDevice, tmpRc);
        painter.end();
    } else {
        KisTransformWorker worker(currentNode()->paintDevice(), m_scaleX, m_scaleY, 0, 0, m_a, int(t.x()), int(t.y()), progress, m_filter);
        worker.run();
    }

// XXX_PROGRESS, XXX_LAYERS
//     // If canceled, go back to the memento
//     if(worker.isCanceled())
//     {
//         transaction->undo();
//         delete transaction;
//         return;
//     }

    currentNode()->paintDevice()->setDirty(rc); // XXX: This is not enough - should union with new extent

    canvas->view()->selectionManager()->selectionChanged();
    if (currentSelection() && currentSelection()->hasShapeSelection())
        canvas->view()->selectionManager()->shapeSelectionChanged();

    // Else add the command -- this will have the memento from the previous state,
    // and the transformed state from the original device we cached in our activated()
    // method.
    if (transaction) {
        transaction->setNewPosition(currentNode()->paintDevice()->x(), currentNode()->paintDevice()->y());
        if (image()->undo())
            image()->undoAdapter()->addCommand(transaction);
        else
            delete transaction;
    }
    updater->deleteLater();

}

void KisToolTransform::notifyCommandAdded(const QUndoCommand * command)
{
    const TransformCmd * cmd = dynamic_cast<const TransformCmd*>(command);
    if (cmd == 0) {
        // The last added command wasn't one of ours;
        // we should reset to the new state of the canvas.
        // In effect we should treat this as if the tool has been just activated
        initHandles();
    }
}

void KisToolTransform::notifyCommandExecuted(const QUndoCommand * command)
{
    Q_UNUSED(command);
    const TransformCmd * cmd = 0;

    if (image()->undoAdapter()->presentCommand())
        cmd = dynamic_cast<const TransformCmd*>(image()->undoAdapter()->presentCommand());

    if (cmd == 0) {
        // The command now on the top of the stack isn't one of ours
        // We should treat this as if the tool has been just activated
        initHandles();
    } else {
        // One of our commands is now on top
        // We should ask for tool args and orig selection
        cmd->transformArgs(m_scaleX, m_scaleY, m_translate, m_a);
        m_origSelection = cmd->origSelection(m_originalTopLeft, m_originalBottomRight);
        m_canvas->updateCanvas(QRect(m_originalTopLeft, m_originalBottomRight));
    }
}

void KisToolTransform::slotSetFilter(const KoID &filterID)
{
    m_filter = KisFilterStrategyRegistry::instance()->value(filterID.id());
}

QWidget* KisToolTransform::createOptionWidget()
{
    m_optWidget = new WdgToolTransform(0);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setObjectName(toolId() + " option widget");

    m_optWidget->cmbFilter->clear();
    m_optWidget->cmbFilter->setIDList(KisFilterStrategyRegistry::instance()->listKeys());

    m_optWidget->cmbFilter->setCurrent("Bicubic");
    connect(m_optWidget->cmbFilter, SIGNAL(activated(const KoID &)),
            this, SLOT(slotSetFilter(const KoID &)));

    KoID filterID = m_optWidget->cmbFilter->currentItem();
    m_filter = KisFilterStrategyRegistry::instance()->value(filterID.id());

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
    m_optWidget->setFixedHeight(m_optWidget->sizeHint().height());
    return m_optWidget;
}

QWidget* KisToolTransform::optionWidget()
{
    return m_optWidget;
}

#include "kis_tool_transform.moc"
