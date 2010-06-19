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
#include <kis_transaction.h>
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

#define KISPAINTDEVICE_MOVE_DEPRECATED

//returns the smallest QRectF containing 4 points
static QRectF boundRect(QPointF P0, QPointF P1, QPointF P2, QPointF P3)
{
	QRectF res(P0, P0);
	QPointF P[] = {P1, P2, P3};

	for (int i = 0; i < 3; ++i) {
		if ( P[i].x() < res.left() )
			res.setLeft(P[i].x());
		else if ( P[i].x() > res.right() )
			res.setRight(P[i].x());

		if ( P[i].y() < res.top() )
			res.setTop(P[i].y());
		else if ( P[i].y() > res.bottom() )
			res.setBottom(P[i].y());
	}

	return res;
}

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
	//TODO reimplement redo/undo
    //KisSelectedTransaction::redo();
    //layer()->paintDevice()->move(m_newPosition);
}

void TransformCmd::undo()
{
    //KisSelectedTransaction::undo();
    //layer()->paintDevice()->move(m_originalTopLeft);
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
    m_previousTopLeft = QPoint(0, 0);
    m_previousBottomRight = QPoint(0, 0);
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
	m_handleRadius = 8;
}

KisToolTransform::~KisToolTransform()
{
}

void KisToolTransform::deactivate()
{
    KisImageWSP kisimage = image();

	//update canvas
	QRectF rc = boundRect(m_topLeft, m_topRight, m_bottomRight, m_bottomLeft);
	rc = QRect(QPoint(rc.left() / kisimage->xRes(), rc.top() / kisimage->yRes()), QPoint(rc.right() / kisimage->xRes(), rc.bottom() / kisimage->yRes()));
	double handleRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_handleRadius);
	double handleRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_handleRadius);
    m_canvas->updateCanvas(rc.adjusted(-handleRadiusX, -handleRadiusY, handleRadiusX, handleRadiusY));

	if (!kisimage)
		return;

    if (kisimage->undoAdapter())
        kisimage->undoAdapter()->removeCommandHistoryListener(this);
}

void KisToolTransform::activate(ToolActivation toolActivation, const QSet<KoShape*> &)
{
    Q_UNUSED(toolActivation);

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
			m_previousTopLeft = m_originalTopLeft;
			m_previousBottomRight = m_originalBottomRight;

			//update canvas
			const KisImage *kisimage = image();
			QRect rc(QPoint(m_originalTopLeft.x() / kisimage->xRes(), m_originalTopLeft.y() / kisimage->yRes()),
						QPoint(m_originalBottomRight.x() / kisimage->xRes(), m_originalBottomRight.y() / kisimage->yRes()));
			double handleRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_handleRadius);
			double handleRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_handleRadius);
			m_canvas->updateCanvas(rc.adjusted(-handleRadiusX, -handleRadiusY, handleRadiusX, handleRadiusY));
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
		//we take all of the paintDevice
        dev->exactBounds(x, y, w, h);
        m_origSelection = 0;
    }
    m_originalTopLeft = QPoint(x, y);
    m_originalBottomRight = QPoint(x + w - 1, y + h - 1);
	m_previousTopLeft = m_originalTopLeft;
	m_previousBottomRight = m_originalBottomRight;
    m_originalCenter = QPointF(m_originalTopLeft + m_originalBottomRight) / 2.0;

    m_a = 0.0;
    m_scaleX = 1.0;
    m_scaleY = 1.0;
    m_scaleX_wOutModifier = 1.0;
    m_scaleY_wOutModifier = 1.0;
    m_translate = m_originalCenter;
	m_originalHeight2 = m_originalCenter.y() - m_originalTopLeft.y();
	m_originalWidth2 = m_originalCenter.x() - m_originalTopLeft.x();
	m_topLeft = m_originalTopLeft;
	m_bottomRight = m_originalBottomRight;

	//update canvas
	const KisImage *kisimage = image();
	QRect rc(QPoint(m_originalTopLeft.x() / kisimage->xRes(), m_originalTopLeft.y() / kisimage->yRes()),
				QPoint(m_originalBottomRight.x() / kisimage->xRes(), m_originalBottomRight.y() / kisimage->yRes()));
	double handleRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_handleRadius);
	double handleRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_handleRadius);
    m_canvas->updateCanvas(rc.adjusted(- handleRadiusX, - handleRadiusY, handleRadiusX, handleRadiusY));
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
            m_clickangle = m_a + atan2(-m_clickoffset.y(), m_clickoffset.x());
			m_refPoint = m_translate;
            break;
		case MOVE:
			m_refPoint = m_translate;
            break;
        case TOPSCALE:
        case TOPRIGHTSCALE:
        case RIGHTSCALE:
        case BOTTOMRIGHTSCALE:
        case BOTTOMSCALE:
        case BOTTOMLEFTSCALE:
        case LEFTSCALE:
        case TOPLEFTSCALE:
			m_refPoint = QPointF(0, 0);
            break;
        }
		m_clickoffset = mousePos - m_refPoint;
        m_selecting = true;
        m_actuallyMoveWhileSelected = false;
		m_prevMousePos = mousePos;
    }

	recalcOutline();
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
    KisCanvas2 *canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
    if (!canvas)
        return;

    KisImageWSP kisimage = image();
    QPointF mousePos = QPointF(e->point.x() * kisimage->xRes(), e->point.y() * kisimage->yRes());

    if (m_selecting) {
        m_actuallyMoveWhileSelected = true;

		QPointF move_vect = mousePos - m_prevMousePos;

        // transform move_vect coords, so it seems like it isn't rotated and centered at m_translate
		move_vect = invrot(move_vect.x(), move_vect.y());

		double previousHeight2 = 0, previousWidth2 = 0;
		int signY = 1, signX = 1;
		QPointF t(0,0);

		switch (m_function) {
		case MOVE:
            m_translate = mousePos - m_clickoffset;
			break;
		case ROTATE:
            m_a = m_clickangle - atan2(-mousePos.y() + m_refPoint.y(), mousePos.x() - m_refPoint.x());
			break;
		case TOPSCALE:
			signY = -1;
		case BOTTOMSCALE:
			move_vect /= 2;
			m_scaleY_wOutModifier = (m_scaleY_wOutModifier * m_originalHeight2 + signY * move_vect.y()) / m_originalHeight2;
			previousHeight2 = m_scaleY * m_originalHeight2;

			//applies the shift modifier
			if (e->modifiers() & Qt::ShiftModifier) {
				double a_scaleY = fabs(m_scaleY_wOutModifier);

				m_scaleX = (m_scaleX_wOutModifier > 0) ? a_scaleY : -a_scaleY;
				m_scaleY = m_scaleY_wOutModifier;
			} else
				m_scaleY = m_scaleY_wOutModifier;

			t.setY(signY * (m_originalHeight2 * m_scaleY - previousHeight2));
			m_translate += rot( t.x(), t.y());
			break;
		case LEFTSCALE:
			signX = -1;
		case RIGHTSCALE:
			move_vect /= 2;
			m_scaleX_wOutModifier = (m_scaleX_wOutModifier * m_originalWidth2 + signX * move_vect.x()) / m_originalWidth2;
			previousWidth2 = m_scaleX * m_originalWidth2;

			//applies the shift modifier
			if (e->modifiers() & Qt::ShiftModifier) {
				double a_scaleX = fabs(m_scaleX_wOutModifier);

				m_scaleY = (m_scaleY_wOutModifier > 0) ? a_scaleX : -a_scaleX;
				m_scaleX = m_scaleX_wOutModifier;
			} else {
				m_scaleX = m_scaleX_wOutModifier;
			}

			t.setX(signX * (m_originalWidth2 * m_scaleX - previousWidth2));
			m_translate += rot( t.x(), t.y());
			break;
		case TOPRIGHTSCALE:
		case BOTTOMRIGHTSCALE:
		case TOPLEFTSCALE:
		case BOTTOMLEFTSCALE: 
			switch(m_function) {
			case TOPRIGHTSCALE:
				signY = -1;
			case BOTTOMRIGHTSCALE:
				break;
			case TOPLEFTSCALE:
				signY = -1;
			case BOTTOMLEFTSCALE:
				signX = -1;
				break;
			default:
				break;
			}

			move_vect /= 2;
			m_scaleX_wOutModifier = (m_scaleX_wOutModifier * m_originalWidth2 + signX * move_vect.x()) / m_originalWidth2;
			m_scaleY_wOutModifier = (m_scaleY_wOutModifier * m_originalHeight2 + signY * move_vect.y()) / m_originalHeight2;
			previousWidth2 = m_scaleX * m_originalWidth2;
			previousHeight2 = m_scaleY * m_originalHeight2;

			//applies the shift modifier
			if (e->modifiers() & Qt::ShiftModifier) {
				double a_scaleX = fabs(m_scaleX_wOutModifier);
				double a_scaleY = fabs(m_scaleY_wOutModifier);

				if (a_scaleX > a_scaleY) {
					m_scaleY = (m_scaleY_wOutModifier > 0) ? a_scaleX : -a_scaleX;
					m_scaleX = m_scaleX_wOutModifier;
				} else {
					m_scaleX = (m_scaleX_wOutModifier > 0) ? a_scaleY : -a_scaleY;
					m_scaleY = m_scaleY_wOutModifier;
				}
			} else {
				m_scaleX = m_scaleX_wOutModifier;
				m_scaleY = m_scaleY_wOutModifier;
			}

			t.setX(signX * (m_originalWidth2 * m_scaleX - previousWidth2));
			t.setY(signY * (m_originalHeight2 * m_scaleY - previousHeight2));

			m_translate += rot( t.x(), t.y());
	 		break;
		}

		//canvas update
		double handleRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_handleRadius);
		double handleRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_handleRadius);

		//get the smallest rectangle containing the previous frame (we need to use the 4 points because the rectangle
		//described by m_topLeft, .., m_bottomLeft can be rotated
		QRectF oldRectF = boundRect(m_topLeft, m_topRight, m_bottomRight, m_bottomLeft);
		//we convert it to the right scale
		QRect oldRect = QRect( QPoint(oldRectF.left() / kisimage->xRes(), oldRectF.top() / kisimage->yRes()), QPoint(oldRectF.right() / kisimage->xRes(), oldRectF.bottom() / kisimage->yRes()) );

		recalcOutline(); //computes new m_topLeft, .., m_bottomLeft points
		QRectF newRectF = boundRect(m_topLeft, m_topRight, m_bottomRight, m_bottomLeft);
		QRect newRect = QRect( QPoint(newRectF.left() / kisimage->xRes(), newRectF.top() / kisimage->yRes()), QPoint(newRectF.right() / kisimage->xRes(), newRectF.bottom() / kisimage->yRes()) );

		//the rectangle to update is the union of the old rectangle et the new one
		newRect = oldRect.united(newRect);

		//we need to add adjust the rectangle because of the handles
		newRect.adjust(- handleRadiusX, - handleRadiusY, handleRadiusX, handleRadiusY);
        m_canvas->updateCanvas(newRect);

    } else {
		recalcOutline();
		
		QPointF *topLeft, *topRight, *bottomLeft, *bottomRight;
		QPointF *tmp;

		//depending on the scale factor, we need to exchange left<->right and right<->left
		if (m_scaleX > 0) {
			topLeft = &m_topLeft;
			bottomLeft = &m_bottomLeft;
			topRight = &m_topRight;
			bottomRight = &m_bottomRight;
		} else {
			topLeft = &m_topRight;
			bottomLeft = &m_bottomRight;
			topRight = &m_topLeft;
			bottomRight = &m_bottomLeft;
		}

		if (m_scaleY < 0) {
			tmp = topLeft;
			topLeft = bottomLeft;
			bottomLeft = tmp;
			tmp = topRight;
			topRight = bottomRight;
			bottomRight = tmp;
		}

        if (det(mousePos - *topLeft, *topRight - *topLeft) > 0)
            m_function = ROTATE;
        else if (det(mousePos - *topRight, *bottomRight - *topRight) > 0)
            m_function = ROTATE;
        else if (det(mousePos - *bottomRight, *bottomLeft - *bottomRight) > 0)
            m_function = ROTATE;
        else if (det(mousePos - *bottomLeft, *topLeft - *bottomLeft) > 0)
            m_function = ROTATE;
        else
            m_function = MOVE;

        double handleRadius = m_canvas->viewConverter()->viewToDocumentX(m_handleRadius);
        handleRadius *= handleRadius; // square it so it fits with distsq

        if (distsq(mousePos, (m_topLeft + m_topRight) / 2.0) <= handleRadius)
            m_function = TOPSCALE;
        if (distsq(mousePos, m_topRight) <= handleRadius)
            m_function = TOPRIGHTSCALE;
        if (distsq(mousePos, (m_topRight + m_bottomRight) / 2.0) <= handleRadius)
            m_function = RIGHTSCALE;
        if (distsq(mousePos, m_bottomRight) <= handleRadius)
            m_function = BOTTOMRIGHTSCALE;
        if (distsq(mousePos, (m_bottomLeft + m_bottomRight) / 2.0) <= handleRadius)
            m_function = BOTTOMSCALE;
        if (distsq(mousePos, m_bottomLeft) <= handleRadius)
            m_function = BOTTOMLEFTSCALE;
        if (distsq(mousePos, (m_topLeft + m_bottomLeft) / 2.0) <= handleRadius)
            m_function = LEFTSCALE;
        if (distsq(mousePos, m_topLeft) <= handleRadius)
            m_function = TOPLEFTSCALE;

        setFunctionalCursor();
    }

	m_prevMousePos = mousePos;
}

void KisToolTransform::mouseReleaseEvent(KoPointerEvent */*e*/)
{
    m_selecting = false;

    if (m_actuallyMoveWhileSelected) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());

		//update canvas
		const KisImage *kisimage = image();
		QRectF rc = boundRect(m_topLeft, m_topRight, m_bottomRight, m_bottomLeft);
		rc = QRect(QPoint(rc.left() / kisimage->xRes(), rc.top() / kisimage->yRes()), QPoint(rc.right() / kisimage->xRes(), rc.bottom() / kisimage->yRes()));
		double handleRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_handleRadius);
		double handleRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_handleRadius);
		m_canvas->updateCanvas(rc.adjusted(-handleRadiusX, -handleRadiusY, handleRadiusX, handleRadiusY));

        transform();
        QApplication::restoreOverrideCursor();

		m_scaleX_wOutModifier = m_scaleX;
		m_scaleY_wOutModifier = m_scaleY;
    }
}

void KisToolTransform::recalcOutline()
{
    double x, y;

    m_sina = sin(m_a);
    m_cosa = cos(m_a);

    x = (m_originalTopLeft.x() - m_originalCenter.x()) * m_scaleX;
    y = (m_originalTopLeft.y() - m_originalCenter.y()) * m_scaleY;
    m_topLeft = rot(x, y) + m_translate;

    x = (m_originalBottomRight.x() - m_originalCenter.x()) * m_scaleX;
    y = (m_originalTopLeft.y() - m_originalCenter.y()) * m_scaleY;
    m_topRight = rot(x, y) + m_translate;

    x = (m_originalTopLeft.x() - m_originalCenter.x()) * m_scaleX;
    y = (m_originalBottomRight.y() - m_originalCenter.y()) * m_scaleY;
    m_bottomLeft = rot(x, y) + m_translate;

    x = (m_originalBottomRight.x() - m_originalCenter.x()) * m_scaleX;
    y = (m_originalBottomRight.y() - m_originalCenter.y()) * m_scaleY;
    m_bottomRight = rot(x, y) + m_translate;
}

void KisToolTransform::paint(QPainter& gc, const KoViewConverter &converter)
{
    QPen old = gc.pen();
    QPen pen(Qt::SolidLine);
    pen.setWidth(0);

    recalcOutline();
    KisImageWSP kisimage = image();
    QPointF topleft = converter.documentToView(QPointF(m_topLeft.x() / kisimage->xRes(), m_topLeft.y() / kisimage->yRes()));
    QPointF topright = converter.documentToView(QPointF(m_topRight.x() / kisimage->xRes(), m_topRight.y() / kisimage->yRes()));
    QPointF bottomleft = converter.documentToView(QPointF(m_bottomLeft.x() / kisimage->xRes(), m_bottomLeft.y() / kisimage->yRes()));
    QPointF bottomright = converter.documentToView(QPointF(m_bottomRight.x() / kisimage->xRes(), m_bottomRight.y() / kisimage->yRes()));

    QRectF handleRect(- m_handleRadius / 2,0 - m_handleRadius / 2, m_handleRadius, m_handleRadius);

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

	//todo/undo commented for now
    // This mementoes the current state of the active device.
    //TransformCmd * transaction = new TransformCmd(this, currentNode(), m_scaleX,
    //        m_scaleY, m_translate, m_a, m_origSelection, m_originalTopLeft, m_originalBottomRight);


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
		//we copy the pixels of the selection into a tmpDevice before clearing them
		//we apply the transformation to the tmpDevice
		//and then we blit it into the currentNode's device
        KisPaintDeviceSP tmpDevice = new KisPaintDevice(m_origDevice->colorSpace());
        QRect selectRect = currentSelection()->selectedRect();
        KisPainter gc(tmpDevice, currentSelection());
        gc.bitBlt(selectRect.topLeft(), m_origDevice, selectRect);
        gc.end();

        KisTransformWorker worker(tmpDevice, m_scaleX, m_scaleY, 0, 0, m_a, int(t.x()), int(t.y()), progress, m_filter);
        worker.run();

        currentNode()->paintDevice()->clearSelection(currentSelection());

        QRect tmpRc = tmpDevice->extent();
        KisPainter painter(currentNode()->paintDevice());
        painter.bitBlt(tmpRc.topLeft(), tmpDevice, tmpRc);
        painter.end();

#ifdef KISPAINTDEVICE_MOVE_DEPRECATED
		//we do the same thing with the selection itself
		//we need to go through a temporary device because changing the offset of the
		//selection's paintDevice doesn't actually move the selection

        KisPaintDeviceSP tmpDevice2 = new KisPaintDevice(currentSelection()->colorSpace());
        KisPainter gc2(tmpDevice2, currentSelection());
        gc2.bitBlt(selectRect.topLeft(), currentSelection(), selectRect);
        gc2.end();

        KisTransformWorker selectionWorker(tmpDevice2, m_scaleX, m_scaleY, 0, 0, m_a, (int)(t.x()), (int)(t.y()), progress, m_filter);
        selectionWorker.run();

		currentSelection()->clear();

        QRect tmpRc2 = tmpDevice2->extent();
        KisPainter painter2(currentSelection()->getOrCreatePixelSelection());
        painter2.bitBlt(tmpRc2.topLeft(), tmpDevice2, tmpRc2);
        painter2.end();
#else
        KisTransformWorker selectionWorker(currentSelection()->getOrCreatePixelSelection(), m_scaleX, m_scaleY, 0, 0, m_a, (int)(t.x()), (int)(t.y()), progress, m_filter);
		selectionWorker.run();
#endif

//Shape selection transformations not supported yet
        //if (currentSelection()->hasShapeSelection()) {
        //    QMatrix resolutionMatrix;
        //    resolutionMatrix.scale(1 / image()->xRes(), 1 / image()->yRes());
        //    QPointF center = resolutionMatrix.map(m_originalCenter);

        //    QMatrix matrix;
        //    matrix.translate(center.x(), center.y());
        //    matrix.rotate(m_a / M_PI*180);
        //    matrix.translate(-center.x(), -center.y());

        //    KisShapeSelection* shapeSelection = static_cast<KisShapeSelection*>(currentSelection()->shapeSelection());
        //    QList<KoShape *> shapes = shapeSelection->shapeManager()->shapes();
        //    QList<QMatrix> m_oldMatrixList;
        //    QList<QMatrix> m_newMatrixList;
        //    foreach(KoShape *shape, shapes) {
        //        m_oldMatrixList << shape->transformation();
        //        m_newMatrixList << shape->transformation()*matrix;
        //    }
        //    KoShapeTransformCommand* cmd = new KoShapeTransformCommand(shapes, m_oldMatrixList, m_newMatrixList, transaction);
        //    cmd->redo();
        //}

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

	//canvas update
	QRect previousRect = QRect( m_previousTopLeft, m_previousBottomRight );

	QRectF currRectF = boundRect(m_topLeft, m_topRight, m_bottomRight, m_bottomLeft);
	QRect currRect = QRect( QPoint(currRectF.left(), currRectF.top()), QPoint(currRectF.right(), currRectF.bottom()) );
	currRect.adjust(- m_handleRadius - 1, - m_handleRadius - 1, m_handleRadius + 1, m_handleRadius + 1);

	QRect tmp = previousRect.united(currRect);

    currentNode()->setDirty(previousRect.united(currRect));

	//update previous points for later
	m_previousTopLeft = currRect.topLeft();
	m_previousBottomRight = currRect.bottomRight();

    canvas->view()->selectionManager()->selectionChanged();

    if (currentSelection() && currentSelection()->hasShapeSelection())
        canvas->view()->selectionManager()->shapeSelectionChanged();

    //// Else add the command -- this will have the memento from the previous state,
    //// and the transformed state from the original device we cached in our activated()
    //// method.
    //if (transaction) {
    //    transaction->setNewPosition(currentNode()->paintDevice()->x(), currentNode()->paintDevice()->y());
    //    if (image()->undo())
    //        image()->undoAdapter()->addCommand(transaction);
    //    else
    //        delete transaction;
    //}
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

		//update canvas
		const KisImage *kisimage = image();
		QRectF rc = boundRect(m_topLeft, m_topRight, m_bottomRight, m_bottomLeft);
		rc = QRect(QPoint(rc.left() / kisimage->xRes(), rc.top() / kisimage->yRes()), QPoint(rc.right() / kisimage->xRes(), rc.bottom() / kisimage->yRes()));
		double handleRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_handleRadius);
		double handleRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_handleRadius);
		m_canvas->updateCanvas(rc.adjusted(-handleRadiusX, -handleRadiusY, handleRadiusX, handleRadiusY));
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
