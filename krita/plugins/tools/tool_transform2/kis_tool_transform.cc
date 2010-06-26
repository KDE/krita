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

#include <math.h>

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

namespace
{
class TransformCmdData : public KisSelectedTransactionData
{

public:
    TransformCmdData(KisToolTransform *tool, KisNodeSP node, ToolTransformArgs args, KisSelectionSP origSel, QPoint startPos, QPoint endPos);
    virtual ~TransformCmdData();

public:
    virtual void redo();
    virtual void undo();
    void transformArgs(ToolTransformArgs &args) const;
    KisSelectionSP origSelection(QPoint &startPos, QPoint &endPos) const;
    void setNewPosition(int x, int y);
private:
	ToolTransformArgs m_args;
    KisToolTransform *m_tool;
    KisSelectionSP m_origSelection;
    QPoint m_originalTopLeft;
    QPoint m_originalBottomRight;
};

TransformCmdData::TransformCmdData(KisToolTransform *tool, KisNodeSP node, ToolTransformArgs args, KisSelectionSP origSel, QPoint originalTopLeft, QPoint originalBottomRight)
        : KisSelectedTransactionData(i18n("Transform"), node)
        , m_args(args)
        , m_tool(tool)
        , m_origSelection(origSel)
        , m_originalTopLeft(originalTopLeft)
        , m_originalBottomRight(originalBottomRight)
{
}

TransformCmdData::~TransformCmdData()
{
}

void TransformCmdData::transformArgs(ToolTransformArgs &args) const
{
	args = m_args;
}

KisSelectionSP TransformCmdData::origSelection(QPoint &originalTopLeft, QPoint &originalBottomRight) const
{
    originalTopLeft = m_originalTopLeft;
    originalBottomRight = m_originalBottomRight;
    return m_origSelection;
}

void TransformCmdData::redo()
{
    KisSelectedTransactionData::redo();
}

void TransformCmdData::undo()
{
    KisSelectedTransactionData::undo();
}
}

class TransformCmd : public KisTransaction
{
public:
    TransformCmd(KisToolTransform *tool, KisNodeSP node,
							ToolTransformArgs args,
							KisSelectionSP origSel,
							QPoint originalTopLeft, QPoint originalBottomRight)
    {
        m_transactionData =
            new TransformCmdData(tool, node, args, origSel, originalTopLeft, originalBottomRight);
    }
};

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
    m_sizeCursors[0] = KisCursor::sizeHorCursor();
    m_sizeCursors[1] = KisCursor::sizeBDiagCursor();
    m_sizeCursors[2] = KisCursor::sizeVerCursor();
    m_sizeCursors[3] = KisCursor::sizeFDiagCursor();
    m_sizeCursors[4] = KisCursor::sizeHorCursor();
    m_sizeCursors[5] = KisCursor::sizeBDiagCursor();
    m_sizeCursors[6] = KisCursor::sizeVerCursor();
    m_sizeCursors[7] = KisCursor::sizeFDiagCursor();
	m_handleDir[0] = QPointF(1, 0);
	m_handleDir[1] = QPointF(1, -1);
	m_handleDir[2] = QPointF(0, -1);
	m_handleDir[3] = QPointF(-1, -1);
	m_handleDir[4] = QPointF(-1, 0);
	m_handleDir[5] = QPointF(-1, 1);
	m_handleDir[6] = QPointF(0, 1);
	m_handleDir[7] = QPointF(1, 1);
    m_origDevice = 0;
    m_origSelection = 0;
	m_handleRadius = 8;
	m_rotationCenterRadius = 12;
	m_maxRadius = (m_handleRadius > m_rotationCenterRadius) ? m_handleRadius : m_rotationCenterRadius;
}

KisToolTransform::~KisToolTransform()
{
}

void KisToolTransform::storeArgs(ToolTransformArgs &args)
{
	args = m_currentArgs;
}

void KisToolTransform::restoreArgs(ToolTransformArgs args)
{
	m_currentArgs = args;
}

void KisToolTransform::updateCanvas()
{
	KisImageWSP kisimage = image();

	if (kisimage) {
		QRectF rc = boundRect(m_topLeft, m_topRight, m_bottomRight, m_bottomLeft);
		rc = QRect(QPoint(rc.left() / kisimage->xRes(), rc.top() / kisimage->yRes()), QPoint(rc.right() / kisimage->xRes(), rc.bottom() / kisimage->yRes()));
		double maxRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_maxRadius);
		double maxRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_maxRadius);
		m_canvas->updateCanvas(rc.adjusted(-maxRadiusX, -maxRadiusY, maxRadiusX, maxRadiusY));
	}
}

void KisToolTransform::deactivate()
{
    KisImageWSP kisimage = image();

	updateCanvas();

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

        const TransformCmdData * cmd = 0;

        if (image()->undoAdapter()->presentCommand())
            cmd = dynamic_cast<const TransformCmdData*>(image()->undoAdapter()->presentCommand());

        if (cmd == 0) {
            initHandles();
        } else {
            // One of our commands is on top
            // We should ask for tool args and orig selection
            cmd->transformArgs(m_currentArgs);
            m_origSelection = cmd->origSelection(m_originalTopLeft, m_originalBottomRight);
			m_originalCenter = (m_originalTopLeft + m_originalBottomRight) / 2;
			m_previousTopLeft = m_originalTopLeft;
			m_previousBottomRight = m_originalBottomRight;
			m_scaleX_wOutModifier = m_currentArgs.scaleX();
			m_scaleY_wOutModifier = m_currentArgs.scaleY();

			//update canvas
			const KisImage *kisimage = image();
			QRect rc(QPoint(m_originalTopLeft.x() / kisimage->xRes(), m_originalTopLeft.y() / kisimage->yRes()),
						QPoint(m_originalBottomRight.x() / kisimage->xRes(), m_originalBottomRight.y() / kisimage->yRes()));
			double maxRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_maxRadius);
			double maxRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_maxRadius);
			m_canvas->updateCanvas(rc.adjusted(-maxRadiusX, -maxRadiusY, maxRadiusX, maxRadiusY));
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

		//if (selection->hasShapeSelection())
		//{
		//	//save the state of the shapes
        //    KisShapeSelection* shapeSelection = static_cast<KisShapeSelection*>(selection->shapeSelection());
		//	m_origSelection->setShapeSelection(shapeSelection->clone(selection.data()));
		//}
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
	m_currentArgs = ToolTransformArgs(m_originalCenter, QPointF(0, 0), 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0);
    m_scaleX_wOutModifier = m_currentArgs.scaleX();
    m_scaleY_wOutModifier = m_currentArgs.scaleY();
	m_originalHeight2 = m_originalCenter.y() - m_originalTopLeft.y();
	m_originalWidth2 = m_originalCenter.x() - m_originalTopLeft.x();
	m_topLeft = m_originalTopLeft;
	m_bottomRight = m_originalBottomRight;

	//update canvas
	const KisImage *kisimage = image();
	QRect rc(QPoint(m_originalTopLeft.x() / kisimage->xRes(), m_originalTopLeft.y() / kisimage->yRes()),
				QPoint(m_originalBottomRight.x() / kisimage->xRes(), m_originalBottomRight.y() / kisimage->yRes()));
	double maxRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_maxRadius);
	double maxRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_maxRadius);
    m_canvas->updateCanvas(rc.adjusted(- maxRadiusX, - maxRadiusY, maxRadiusX, maxRadiusY));
}

void KisToolTransform::mousePressEvent(KoPointerEvent *e)
{
    KisImageWSP kisimage = image();

    if (!currentNode())
        return;


    if (kisimage && currentNode()->paintDevice() && e->button() == Qt::LeftButton) {
        QPointF mousePos = QPointF(e->point.x() * kisimage->xRes(), e->point.y() * kisimage->yRes());
		if (m_function == ROTATE) {
			QPointF clickoffset = mousePos - m_rotationCenter;
            m_clickangle = atan2(-clickoffset.y(), clickoffset.x());
		}

		m_clickPoint = mousePos;
        m_selecting = true;
        m_actuallyMoveWhileSelected = false;
		m_prevMousePos = mousePos;
    }

	m_clickRotationCenter = m_rotationCenter;
	storeArgs(m_clickArgs);

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

int KisToolTransform::octant(double x, double y)
{
	double angle = atan2(- y, x) + M_PI / 8;
	//M_PI / 8 to get the correct octant

	//we want an angle in [0; 2 * Pi[
	angle = fmod(angle, 2. * M_PI);
	if (angle < 0)
		angle += 2 * M_PI;

	int octant = (int)(angle * 4. / M_PI);

	return octant;
}

void KisToolTransform::setFunctionalCursor()
{
	QPointF dir_vect;
	int rotOctant;

    switch (m_function) {
    case MOVE:
        useCursor(KisCursor::moveCursor());
        break;
    case ROTATE:
        useCursor(KisCursor::rotateCursor());
        break;
	case RIGHTSCALE:
	case TOPRIGHTSCALE: case TOPSCALE:
	case TOPLEFTSCALE:
	case LEFTSCALE:
	case BOTTOMLEFTSCALE:
	case BOTTOMSCALE:
	case BOTTOMRIGHTSCALE:
		dir_vect = m_handleDir[m_function - RIGHTSCALE];
		if (m_currentArgs.scaleX() < 0)
			dir_vect.setX(- dir_vect.x());
		if (m_currentArgs.scaleY() < 0)
			dir_vect.setY(- dir_vect.y());
		dir_vect = rotZ(dir_vect.x(), dir_vect.y(), 0).toPointF();

		rotOctant = octant(dir_vect.x(), dir_vect.y());
        useCursor(m_sizeCursors[rotOctant]);
        break;
	case MOVECENTER:
		useCursor(KisCursor::handCursor());
		break;
	case BOTTOMSHEAR:
	case RIGHTSHEAR:
	case TOPSHEAR:
	case LEFTSHEAR:
		dir_vect = m_handleDir[(m_function - BOTTOMSHEAR) * 2];
		if (m_currentArgs.scaleX() < 0)
			dir_vect.setX(- dir_vect.x());
		if (m_currentArgs.scaleY() < 0)
			dir_vect.setY(- dir_vect.y());
		dir_vect = rotZ(dir_vect.x(), dir_vect.y(), 0).toPointF();

		rotOctant = octant(dir_vect.x(), dir_vect.y());
        useCursor(m_sizeCursors[rotOctant]);
    }
}

void KisToolTransform::mouseMoveEvent(KoPointerEvent *e)
{
    KisCanvas2 *canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
    if (!canvas)
        return;

    KisImageWSP kisimage = image();
    QPointF mousePos = QPointF(e->point.x() * kisimage->xRes(), e->point.y() * kisimage->yRes());
	double dx, dy;

    if (m_selecting) {
        m_actuallyMoveWhileSelected = true;

		QPointF move_vect = mousePos - m_prevMousePos;

		double previousHeight2 = 0, previousWidth2 = 0;
		int signY = 1, signX = 1;
		QPointF t(0,0);

		switch (m_function) {
		case MOVE:
			m_currentArgs.setTranslate(m_clickArgs.translate() + mousePos - m_clickPoint);
			break;
		case ROTATE:
			{
				double theta = m_clickangle - atan2(-mousePos.y() + m_clickRotationCenter.y(), mousePos.x() - m_clickRotationCenter.x());
				std::complex<qreal> exp_theta = exp(std::complex<qreal>(0, theta));
				std::complex<qreal> rotationCenter(m_clickRotationCenter.x(), m_clickRotationCenter.y());
				std::complex<qreal> origin(m_clickArgs.translate().x(), m_clickArgs.translate().y());
				std::complex<qreal> translate((exp_theta - std::complex<qreal>(1, 0)) * (origin - rotationCenter));
				
				m_currentArgs.setTranslate(m_clickArgs.translate() + QPointF(translate.real(), translate.imag()));
				m_currentArgs.setAZ(m_clickArgs.aZ() + theta);
			}
			break;
		case TOPSCALE:
			signY = -1;
		case BOTTOMSCALE:
			// transform move_vect coords, so it seems like it isn't rotated and centered at m_translate
			move_vect = invrotZ(move_vect.x(), move_vect.y(), 0).toPointF();
			move_vect = invshear(move_vect.x(), move_vect.y(), 0).toPointF();

			move_vect /= 2;
			m_scaleY_wOutModifier = (m_scaleY_wOutModifier * m_originalHeight2 + signY * move_vect.y()) / m_originalHeight2;
			previousHeight2 = m_currentArgs.scaleY() * m_originalHeight2;

			//applies the shift modifier
			if (e->modifiers() & Qt::ShiftModifier) {
				double a_scaleY = fabs(m_scaleY_wOutModifier);

				m_currentArgs.setScaleX((m_scaleX_wOutModifier > 0) ? a_scaleY : -a_scaleY);
				m_currentArgs.setScaleY(m_scaleY_wOutModifier);
			} else
				m_currentArgs.setScaleY(m_scaleY_wOutModifier);

			t.setY(signY * (m_originalHeight2 * m_currentArgs.scaleY() - previousHeight2));
			t = shear(t.x(), t.y(), 0).toPointF();

			m_currentArgs.setTranslate(m_currentArgs.translate() + rotZ(t.x(), t.y(), 0).toPointF());
			break;
		case LEFTSCALE:
			signX = -1;
		case RIGHTSCALE:
			move_vect = invrotZ(move_vect.x(), move_vect.y(), 0).toPointF();
			move_vect = invshear(move_vect.x(), move_vect.y(), 0).toPointF();

			move_vect /= 2;
			m_scaleX_wOutModifier = (m_scaleX_wOutModifier * m_originalWidth2 + signX * move_vect.x()) / m_originalWidth2;
			previousWidth2 = m_currentArgs.scaleX() * m_originalWidth2;

			//applies the shift modifier
			if (e->modifiers() & Qt::ShiftModifier) {
				double a_scaleX = fabs(m_scaleX_wOutModifier);

				m_currentArgs.setScaleY((m_scaleY_wOutModifier > 0) ? a_scaleX : -a_scaleX);
				m_currentArgs.setScaleX(m_scaleX_wOutModifier);
			} else
				m_currentArgs.setScaleX(m_scaleX_wOutModifier);

			t.setX(signX * (m_originalWidth2 * m_currentArgs.scaleX() - previousWidth2));
			t.setY(t.y() + t.x() * m_currentArgs.shearY());
			m_currentArgs.setTranslate(m_currentArgs.translate() + rotZ(t.x(), t.y(), 0).toPointF());
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

			move_vect = invrotZ(move_vect.x(), move_vect.y(), 0).toPointF();
			move_vect = invshear(move_vect.x(), move_vect.y(), 0).toPointF();
			move_vect /= 2;
			m_scaleX_wOutModifier = (m_scaleX_wOutModifier * m_originalWidth2 + signX * move_vect.x()) / m_originalWidth2;
			m_scaleY_wOutModifier = (m_scaleY_wOutModifier * m_originalHeight2 + signY * move_vect.y()) / m_originalHeight2;
			previousWidth2 = m_currentArgs.scaleX() * m_originalWidth2;
			previousHeight2 = m_currentArgs.scaleY() * m_originalHeight2;

			//applies the shift modifier
			if (e->modifiers() & Qt::ShiftModifier) {
				double a_scaleX = fabs(m_scaleX_wOutModifier);
				double a_scaleY = fabs(m_scaleY_wOutModifier);

				if (a_scaleX > a_scaleY) {
					m_currentArgs.setScaleY((m_scaleY_wOutModifier > 0) ? a_scaleX : -a_scaleX);
					m_currentArgs.setScaleX(m_scaleX_wOutModifier);
				} else {
					m_currentArgs.setScaleX((m_scaleX_wOutModifier > 0) ? a_scaleY : -a_scaleY);
					m_currentArgs.setScaleY(m_scaleY_wOutModifier);
				}
			} else {
				m_currentArgs.setScaleX(m_scaleX_wOutModifier);
				m_currentArgs.setScaleY(m_scaleY_wOutModifier);
			}

			t.setX(signX * (m_originalWidth2 * m_currentArgs.scaleX() - previousWidth2));
			t.setY(signY * (m_originalHeight2 * m_currentArgs.scaleY() - previousHeight2));
			t = shear(t.x(), t.y(), 0).toPointF();

			m_currentArgs.setTranslate(m_currentArgs.translate() + rotZ(t.x(), t.y(), 0).toPointF());
	 		break;
		case MOVECENTER:
			t = mousePos - m_currentArgs.translate();
			t = invrotZ(t.x(), t.y(), 0).toPointF();
			t = invshear(t.x(), t.y(), 0).toPointF();
			t = invscale(t.x(), t.y(), 0).toPointF();

			//now we need to clip t in the rectangle of the tool
			if (t.y() != 0) {
				double slope = t.y() / t.x();

				if (t.x() < - m_originalWidth2) {
					t.setY(- m_originalWidth2 * slope);
					t.setX(- m_originalWidth2);
				} else if (t.x() > m_originalWidth2) {
					t.setY(m_originalWidth2 * slope);
					t.setX(m_originalWidth2);
				}

				if (t.y() < - m_originalHeight2) {
					if (t.x() != 0)
						t.setX(- m_originalHeight2 / slope);
					t.setY(- m_originalHeight2);
				} else if (t.y() > m_originalHeight2) {
					if (t.x() != 0)
						t.setX(m_originalHeight2 / slope);
					t.setY(m_originalHeight2);
				}
			} else {
				if (t.x() < - m_originalWidth2)
					t.setX(- m_originalWidth2);
				else if (t.x() > m_originalWidth2)
					t.setX(m_originalWidth2);
			}

			m_currentArgs.setRotationCenterOffset(t);
			break;
		case TOPSHEAR:
			signX = -1;
		case BOTTOMSHEAR:
			signX *= (m_currentArgs.scaleY() < 0) ? -1 : 1;
			move_vect = invrotZ(move_vect.x(), move_vect.y(), 0).toPointF();
			t = move_vect;
			t = invshear(t.x(), t.y(), 0).toPointF();

			dx = signX *  m_currentArgs.shearX() * m_originalHeight2; //get the dx pixels corresponding to the current shearX factor
			dx += t.x(); //add the horizontal movement
			m_currentArgs.setShearX(signX * dx / m_originalHeight2); // calculate the new shearX factor
			break;
		case LEFTSHEAR:
			signY = -1;
		case RIGHTSHEAR:
			signY *= (m_currentArgs.scaleX() < 0) ? -1 : 1;
			move_vect = invrotZ(move_vect.x(), move_vect.y(), 0).toPointF();
			t = move_vect;
			t = invshear(t.x(), t.y(), 0).toPointF();

			dy = signY *  m_currentArgs.shearY() * m_originalWidth2; //get the dx pixels corresponding to the current shearX factor
			dy += t.y(); //add the horizontal movement
			m_currentArgs.setShearY(signY * dy / m_originalWidth2); // calculate the new shearX factor
			break;
		}

		//canvas update
		double maxRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_maxRadius);
		double maxRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_maxRadius);

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
		newRect.adjust(- maxRadiusX, - maxRadiusY, maxRadiusX, maxRadiusY);
        m_canvas->updateCanvas(newRect);

    } else {
		recalcOutline();
		
		QPointF *topLeft, *topRight, *bottomLeft, *bottomRight;
		QPointF *tmp;

		//depending on the scale factor, we need to exchange left<->right and right<->left
		if (m_currentArgs.scaleX() > 0) {
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

		if (m_currentArgs.scaleY() < 0) {
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

        double handleRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_handleRadius);
        double handleRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_handleRadius);
        double handleRadius = (handleRadiusX > handleRadiusY) ? handleRadiusX : handleRadiusY;
        double handleRadiusSq = handleRadius * handleRadius; // square it so it fits with distsq

		double rotationCenterRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_rotationCenterRadius);
		double rotationCenterRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_rotationCenterRadius);
        double rotationCenterRadius = (rotationCenterRadiusX > rotationCenterRadiusY) ? rotationCenterRadiusX : rotationCenterRadiusY;
		rotationCenterRadius *= rotationCenterRadius;

        if (distsq(mousePos, (m_topLeft + m_topRight) / 2.0) <= handleRadiusSq)
            m_function = TOPSCALE;
        if (distsq(mousePos, m_topRight) <= handleRadiusSq)
            m_function = TOPRIGHTSCALE;
        if (distsq(mousePos, (m_topRight + m_bottomRight) / 2.0) <= handleRadiusSq)
            m_function = RIGHTSCALE;
        if (distsq(mousePos, m_bottomRight) <= handleRadiusSq)
            m_function = BOTTOMRIGHTSCALE;
        if (distsq(mousePos, (m_bottomLeft + m_bottomRight) / 2.0) <= handleRadiusSq)
            m_function = BOTTOMSCALE;
        if (distsq(mousePos, m_bottomLeft) <= handleRadiusSq)
            m_function = BOTTOMLEFTSCALE;
        if (distsq(mousePos, (m_topLeft + m_bottomLeft) / 2.0) <= handleRadiusSq)
            m_function = LEFTSCALE;
        if (distsq(mousePos, m_topLeft) <= handleRadiusSq)
            m_function = TOPLEFTSCALE;
		if (distsq(mousePos, m_rotationCenter) <= rotationCenterRadius)
			m_function = MOVECENTER;

		if (m_function == ROTATE || m_function == MOVE) {
			//We check for shearing only if we aren't near a handle (for scale) or the rotation center
			QPointF t = mousePos - m_currentArgs.translate();
			t = invrotZ(t.x(), t.y(), 0).toPointF();
			t = invshear(t.x(), t.y(), 0).toPointF();
			t = invscale(t.x(), t.y(), 0).toPointF();
			t += m_originalCenter;
	 
			if (t.x() >= m_originalTopLeft.x() && t.x() <= m_originalBottomRight.x()) {
				if (fabs(t.y() - m_originalTopLeft.y()) <= 12)
					m_function = TOPSHEAR;
				else if (fabs(t.y() - m_originalBottomRight.y()) <= 12)
					m_function = BOTTOMSHEAR;
			} else if (t.y() >= m_originalTopLeft.y() && t.y() <= m_originalBottomRight.y()) {
				if (fabs(t.x() - m_originalTopLeft.x()) <= 12)
					m_function = LEFTSHEAR;
				else if (fabs(t.x() - m_originalBottomRight.x()) <= 12)
					m_function = RIGHTSHEAR;
			}
		}

        setFunctionalCursor();
    }

	m_prevMousePos = mousePos;
}

void KisToolTransform::mouseReleaseEvent(KoPointerEvent */*e*/)
{
    m_selecting = false;

    if (m_actuallyMoveWhileSelected) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());

		updateCanvas();

		if (m_function != MOVECENTER)
			transform();

		QApplication::restoreOverrideCursor();

		m_scaleX_wOutModifier = m_currentArgs.scaleX();
		m_scaleY_wOutModifier = m_currentArgs.scaleY();
    }
}

void KisToolTransform::recalcOutline()
{
	QPointF t;

    m_sinaZ = sin(m_currentArgs.aZ());
    m_cosaZ = cos(m_currentArgs.aZ());

	t = m_originalTopLeft - m_originalCenter;
	t = scale(t.x(), t.y(), 0).toPointF();
	t = shear(t.x(), t.y(), 0).toPointF();
    m_topLeft = rotZ(t.x(), t.y(), 0).toPointF() + m_currentArgs.translate();

	t = QPointF(m_originalBottomRight.x() - m_originalCenter.x(), m_originalTopLeft.y() - m_originalCenter.y());
	t = scale(t.x(), t.y(), 0).toPointF();
	t = shear(t.x(), t.y(), 0).toPointF();
    m_topRight = rotZ(t.x(), t.y(), 0).toPointF() + m_currentArgs.translate();

	t = QPointF(m_originalTopLeft.x() - m_originalCenter.x(), m_originalBottomRight.y() - m_originalCenter.y());
	t = scale(t.x(), t.y(), 0).toPointF();
	t = shear(t.x(), t.y(), 0).toPointF();
    m_bottomLeft = rotZ(t.x(), t.y(), 0).toPointF() + m_currentArgs.translate();

	t = m_originalBottomRight - m_originalCenter;
	t = scale(t.x(), t.y(), 0).toPointF();
	t = shear(t.x(), t.y(), 0).toPointF();
    m_bottomRight = rotZ(t.x(), t.y(), 0).toPointF() + m_currentArgs.translate();

	t = m_currentArgs.rotationCenterOffset();
	t = scale(t.x(), t.y(), 0).toPointF();
	t = shear(t.x(), t.y(), 0).toPointF();
	m_rotationCenter = rotZ(t.x(), t.y(), 0).toPointF() + m_currentArgs.translate();
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

    QRectF handleRect(- m_handleRadius / 2., - m_handleRadius / 2., m_handleRadius, m_handleRadius);

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

	QPointF rotationCenter = converter.documentToView(QPointF(m_rotationCenter.x() / kisimage->xRes(), m_rotationCenter.y() / kisimage->yRes()));
	QRectF rotationCenterRect(- m_rotationCenterRadius / 2., - m_rotationCenterRadius / 2., m_rotationCenterRadius, m_rotationCenterRadius);
	gc.drawEllipse(rotationCenterRect.translated(rotationCenter));
	gc.drawLine(QPointF(rotationCenter.x() - m_rotationCenterRadius / 2. - 2, rotationCenter.y()), QPointF(rotationCenter.x() + m_rotationCenterRadius / 2. + 2, rotationCenter.y()));
	gc.drawLine(QPointF(rotationCenter.x(), rotationCenter.y() - m_rotationCenterRadius / 2. - 2), QPointF(rotationCenter.x(), rotationCenter.y() + m_rotationCenterRadius / 2. + 2));

    gc.setPen(old);
}

void KisToolTransform::transform()
{
    if (!image() || !currentNode()->paintDevice())
        return;

    KisCanvas2 *canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
    if (!canvas)
        return;

    QPointF t = m_currentArgs.translate() - rotZ(m_originalCenter.x() * m_currentArgs.scaleX(), m_originalCenter.y() * m_currentArgs.scaleY(), 0).toPointF();
    KoProgressUpdater* updater = canvas->view()->createProgressUpdater();
    updater->start(100, i18n("Transform"));
    KoUpdaterPtr progress = updater->startSubtask();

	//todo/undo commented for now
    // This mementoes the current state of the active device.
    TransformCmd transaction(this, currentNode(), m_currentArgs, m_origSelection, m_originalTopLeft, m_originalBottomRight);


    //Copy the original state back.
    QRect rc = m_origDevice->extent();
    rc = rc.normalized();
    currentNode()->paintDevice()->clear();
    KisPainter gc(currentNode()->paintDevice());
    gc.setCompositeOp(COMPOSITE_COPY);
    gc.bitBlt(rc.topLeft(), m_origDevice, rc);
    gc.end();

    // Also restore the original pixel selection (the shape selection will also be restored : see below)
    if (m_origSelection && !m_origSelection->isDeselected()) {
        if (currentSelection()) {
			//copy the pixel selection
			QRect rc = m_origSelection->selectedRect();
			rc = rc.normalized();
            currentSelection()->getOrCreatePixelSelection()->clear();
            KisPainter sgc(KisPaintDeviceSP(currentSelection()->getOrCreatePixelSelection()));
            sgc.setCompositeOp(COMPOSITE_COPY);
            sgc.bitBlt(rc.topLeft(), m_origSelection->getOrCreatePixelSelection(), rc);
            sgc.end();

        	//if (m_origSelection->hasShapeSelection()) {
			//	KisShapeSelection* origShapeSelection = static_cast<KisShapeSelection*>(m_origSelection->shapeSelection());
			//	if (currentSelection()->hasShapeSelection()) {
			//		KisShapeSelection* currentShapeSelection = static_cast<KisShapeSelection*>(currentSelection()->shapeSelection());
			//		m_previousShapeSelection = static_cast<KisShapeSelection*>(currentShapeSelection->clone(currentSelection().data()));
			//	}
			//	else
			//		m_previousShapeSelection = static_cast<KisShapeSelection*>(origShapeSelection->clone(m_origSelection.data()));

			//	currentSelection()->setShapeSelection(origShapeSelection->clone(m_origSelection.data()));
			//}
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
        QRect selectRect = currentSelection()->selectedExactRect();
        KisPainter gc(tmpDevice, currentSelection());
        gc.bitBlt(selectRect.topLeft(), m_origDevice, selectRect);
        gc.end();

        KisTransformWorker worker(tmpDevice, m_currentArgs.scaleX(), m_currentArgs.scaleY(), m_currentArgs.shearX(), m_currentArgs.shearY(), m_currentArgs.aZ(), int(t.x()), int(t.y()), progress, m_filter);
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

		KisPixelSelectionSP pixelSelection = currentSelection()->getOrCreatePixelSelection();
		QRect pixelSelectRect = pixelSelection->selectedExactRect();

        KisPaintDeviceSP tmpDevice2 = new KisPaintDevice(pixelSelection->colorSpace());
        KisPainter gc2(tmpDevice2, currentSelection());
        gc2.bitBlt(pixelSelectRect.topLeft(), pixelSelection, pixelSelectRect);
        gc2.end();

        KisTransformWorker selectionWorker(tmpDevice2, m_currentArgs.scaleX(), m_currentArgs.scaleY(), m_currentArgs.shearX(), m_currentArgs.shearY(), m_currentArgs.aZ(), (int)(t.x()), (int)(t.y()), progress, m_filter);
        selectionWorker.run();

		pixelSelection->clear();

        QRect tmpRc2 = tmpDevice2->extent();
        KisPainter painter2(pixelSelection);
        painter2.bitBlt(tmpRc2.topLeft(), tmpDevice2, tmpRc2);
        painter2.end();
#else
        KisTransformWorker selectionWorker(currentSelection()->getOrCreatePixelSelection(), m_currentArgs.scaleX(), m_currentArgs.scaleY(), m_currentArgs.shearX(), m_currentArgs.shearY(), m_currentArgs.aZ(), (int)(t.x()), (int)(t.y()), progress, m_filter);
		selectionWorker.run();
#endif

		//Shape selection not transformed yet
        //if (m_origSelection->hasShapeSelection() && currentSelection()->hasShapeSelection() && m_previousShapeSelection) {
        //    KisShapeSelection* origShapeSelection = static_cast<KisShapeSelection*>(m_origSelection->shapeSelection());
        //    QList<KoShape *> origShapes = origShapeSelection->shapeManager()->shapes();
		//	KisShapeSelection* currentShapeSelection = static_cast<KisShapeSelection*>(currentSelection()->shapeSelection());
        //    QList<KoShape *> currentShapes = currentShapeSelection->shapeManager()->shapes();
        //    QList<KoShape *> previousShapes = m_previousShapeSelection->shapeManager()->shapes();
        //    QList<QMatrix> m_oldMatrixList;
        //    QList<QMatrix> m_newMatrixList;
		//	for (int i = 0; i < origShapes.size(); ++i) {
		//		KoShape *origShape = origShapes.at(i);
		//		QMatrix origMatrix = origShape->transformation();
		//		QMatrix previousMatrix = previousShapes.at(i)->transformation();

		//		QPointF center = origMatrix.map(QPointF(0.5 * origShape->size().width(), 0.5 * origShape->size().height()));
		//		QMatrix rotateMatrix;
		//		rotateMatrix.translate(center.x(), center.y());
		//		rotateMatrix.rotate(180. * m_a / M_PI);
		//		rotateMatrix.translate(-center.x(), -center.y());

        //        m_oldMatrixList << previousMatrix;
        //        m_newMatrixList << origMatrix * rotateMatrix; // we apply the rotation on the original matrix
        //    }
        //    KoShapeTransformCommand* cmd = new KoShapeTransformCommand(currentShapes, m_oldMatrixList, m_newMatrixList, transaction.undoCommand());
        //    cmd->redo();
        //}

    } else {
        KisTransformWorker worker(currentNode()->paintDevice(), m_currentArgs.scaleX(), m_currentArgs.scaleY(), m_currentArgs.shearX(), m_currentArgs.shearY(), m_currentArgs.aZ(), int(t.x()), int(t.y()), progress, m_filter);
        worker.run();
    }

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
    transaction.commit(image()->undoAdapter());
    updater->deleteLater();
}

void KisToolTransform::notifyCommandAdded(const QUndoCommand * command)
{
    const TransformCmdData * cmd = dynamic_cast<const TransformCmdData*>(command);
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
    const TransformCmdData * cmd = 0;

    if (image()->undoAdapter()->presentCommand())
        cmd = dynamic_cast<const TransformCmdData*>(image()->undoAdapter()->presentCommand());

    if (cmd == 0) {
        // The command now on the top of the stack isn't one of ours
        // We should treat this as if the tool has been just activated
        initHandles();
    } else {
        // One of our commands is now on top
        // We should ask for tool args and orig selection
        cmd->transformArgs(m_currentArgs);
        m_origSelection = cmd->origSelection(m_originalTopLeft, m_originalBottomRight);
		m_originalCenter = (m_originalTopLeft + m_originalBottomRight) / 2;
		m_previousTopLeft = m_originalTopLeft;
		m_previousBottomRight = m_originalBottomRight;
		m_scaleX_wOutModifier = m_currentArgs.scaleX();
		m_scaleY_wOutModifier = m_currentArgs.scaleY();

		updateCanvas();
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
