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
#include "rotation_icons.h"

#include <math.h>

#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QObject>
#include <QLabel>
#include <QComboBox>
#include <QApplication>
#include <QMatrix4x4>

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
#include <kis_perspectivetransform_worker.h>
#include <kis_warptransform_worker.h>
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
class ApplyTransformCmdData : public KisSelectedTransactionData
{

public:
    ApplyTransformCmdData(KisToolTransform *tool, ToolTransformArgs::TransfMode mode, KisNodeSP node);
    virtual ~ApplyTransformCmdData();

public:
    virtual void redo();
    virtual void undo();
	ToolTransformArgs::TransfMode mode() const;
private:
    KisToolTransform *m_tool;
	ToolTransformArgs::TransfMode m_mode;
};

ApplyTransformCmdData::ApplyTransformCmdData(KisToolTransform *tool, ToolTransformArgs::TransfMode mode, KisNodeSP node)
        : KisSelectedTransactionData(i18n("Apply transformation"), node)
        , m_tool(tool)
{
	m_mode = mode;
}

ApplyTransformCmdData::~ApplyTransformCmdData()
{
}

ToolTransformArgs::TransfMode ApplyTransformCmdData::mode() const
{
	return m_mode;
}

void ApplyTransformCmdData::redo()
{
    KisSelectedTransactionData::redo();
}

void ApplyTransformCmdData::undo()
{
    KisSelectedTransactionData::undo();
}
}

class ApplyTransformCmd : public KisTransaction
{
public:
    ApplyTransformCmd(KisToolTransform *tool, ToolTransformArgs::TransfMode mode, KisNodeSP node)
    {
        m_transactionData =
            new ApplyTransformCmdData(tool, mode, node);
    }
};

namespace
{
class TransformCmd : public QUndoCommand
{
public:
    TransformCmd(KisToolTransform *tool, ToolTransformArgs args, KisSelectionSP origSel, QPoint startPos, QPoint endPos, QImage *origImg, QImage *origSelectionImg);
    virtual ~TransformCmd();

public:
    virtual void redo();
    virtual void undo();
    void transformArgs(ToolTransformArgs &args) const;
    KisSelectionSP origSelection(QPoint &startPos, QPoint &endPos) const;
    void origPreviews(QImage *&origImg, QImage *&origSelectionImg) const;
private:
    ToolTransformArgs m_args;
    KisToolTransform *m_tool;
    KisSelectionSP m_origSelection;
    QPoint m_originalTopLeft;
    QPoint m_originalBottomRight;
    QImage *m_origImg, *m_origSelectionImg;
};

TransformCmd::TransformCmd(KisToolTransform *tool, ToolTransformArgs args, KisSelectionSP origSel, QPoint startPos, QPoint endPos, QImage *origImg, QImage *origSelectionImg)
{
    m_args = args;
    m_tool = tool;
    m_origSelection = origSel;
    m_originalTopLeft = startPos;
    m_originalBottomRight = endPos;
    m_origImg = origImg;
    m_origSelectionImg = origSelectionImg;
}

TransformCmd::~TransformCmd()
{
}

void TransformCmd::transformArgs(ToolTransformArgs &args) const
{
    args = m_args;
}

KisSelectionSP TransformCmd::origSelection(QPoint &originalTopLeft, QPoint &originalBottomRight) const
{
    originalTopLeft = m_originalTopLeft;
    originalBottomRight = m_originalBottomRight;
    return m_origSelection;
}

void TransformCmd::origPreviews(QImage *&origImg, QImage *&origSelectionImg) const
{
    origImg = m_origImg;
    origSelectionImg = m_origSelectionImg;
}

void TransformCmd::redo()
{
    KoToolManager *manager = KoToolManager::instance();
    if (manager && m_tool && manager->activeToolId() != m_tool->toolId())
        manager->switchToolRequested(m_tool->toolId());
}

void TransformCmd::undo()
{
    KoToolManager *manager = KoToolManager::instance();
    if (manager && m_tool && manager->activeToolId() != m_tool->toolId())
        manager->switchToolRequested(m_tool->toolId());
}
}

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

static QPointF minMaxZ(QVector3D P0, QVector3D P1, QVector3D P2, QVector3D P3)
{
    QVector3D P[] = {P1, P2, P3};
    QPointF res(P0.z(), P0.z());

    for (int i = 0; i < 3; ++i) {
        if (P[i].z() < res.x())
            res.setX(P[i].z());
        if (P[i].z() > res.y())
            res.setY(P[i].z());
    }

    return res;
}

//returns a value in [0; 360[
static double radianToDegree(double rad)
{
    double piX2 = 2 * M_PI;

    if (rad < 0 || rad >= piX2) {
        rad = fmod(rad, piX2);
        if (rad < 0)
            rad += piX2;
    }

    return (rad * 360. / piX2);
}

//returns a value in [0; 2 * M_PI[
static double degreeToRadian(double degree)
{
    if (degree < 0. || degree >= 360.) {
        degree = fmod(degree, 360.);
        if (degree < 0)
            degree += 360.;
    }

    return (degree * M_PI / 180.);
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
    //m_viewerZ = 1000; //arbitrary value
    m_eyePos = QVector3D(0, 0, -1024);
    m_cameraPos = QVector3D(0, 0, 1024);
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
    m_handleDir[8] = QPointF(0, 0); //also add the center
	m_viewTransfPoints = NULL;
	m_viewOrigPoints = NULL;

    m_origDevice = 0;
    m_origSelection = 0;
    m_handleRadius = 12;
    m_rotationCenterRadius = 12;
    m_boxValueChanged = false;
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

QRectF KisToolTransform::calcWarpBoundRect()
{
    QRectF res(m_currentArgs.transfPoints()[0], m_currentArgs.transfPoints()[0]);
	
	int nbPoints = m_currentArgs.pointsPerLine() * m_currentArgs.pointsPerLine();
	for (int i = 0; i < nbPoints; ++i) {
		if ( m_currentArgs.transfPoints()[i].x() < res.left() )
			res.setLeft(m_currentArgs.transfPoints()[i].x());
		else if ( m_currentArgs.transfPoints()[i].x() > res.right() )
			res.setRight(m_currentArgs.transfPoints()[i].x());
		if ( m_currentArgs.transfPoints()[i].y() < res.top() )
			res.setTop(m_currentArgs.transfPoints()[i].y());
		else if ( m_currentArgs.transfPoints()[i].y() > res.bottom() )
			res.setBottom(m_currentArgs.transfPoints()[i].y());
	}

    return res;
}

void KisToolTransform::updateCurrentOutline()
{
    if (m_imageTooBig) {
        KisCanvas2 *canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
        if (!canvas)
            return;
        canvas->updateCanvas();
        return;
    }

    KisImageWSP kisimage = image();

    if (kisimage) {
        QRectF rc = boundRect(m_topLeftProj, m_topRightProj, m_bottomRightProj, m_bottomLeftProj);
        rc = QRect(QPoint(rc.left() / kisimage->xRes(), rc.top() / kisimage->yRes()), QPoint(rc.right() / kisimage->xRes(), rc.bottom() / kisimage->yRes()));
        double maxRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_maxRadius);
        double maxRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_maxRadius);
        m_canvas->updateCanvas(rc.adjusted(-maxRadiusX, -maxRadiusY, maxRadiusX, maxRadiusY));
    }
}

void KisToolTransform::updateOutlineChanged()
{
    if (m_imageTooBig) {
        recalcOutline();
        KisCanvas2 *canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
        if (!canvas)
            return;
        canvas->updateCanvas();
        return;
    }

    KisImageSP kisimage = image();
    double maxRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_maxRadius);
    double maxRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_maxRadius);

    //get the smallest rectangle containing the previous frame (we need to use the 4 points because the rectangle
    //described by m_topLeft, .., m_bottomLeft can be rotated
    QRectF oldRectF = boundRect(m_topLeftProj, m_topRightProj, m_bottomRightProj, m_bottomLeftProj);
    //we convert it to the right scale
    QRect oldRect = QRect( QPoint(oldRectF.left() / kisimage->xRes(), oldRectF.top() / kisimage->yRes()), QPoint(oldRectF.right() / kisimage->xRes(), oldRectF.bottom() / kisimage->yRes()) );

    recalcOutline(); //computes new m_topLeft, .., m_bottomLeft points
    QRectF newRectF = boundRect(m_topLeftProj, m_topRightProj, m_bottomRightProj, m_bottomLeftProj);
    QRect newRect = QRect( QPoint(newRectF.left() / kisimage->xRes(), newRectF.top() / kisimage->yRes()), QPoint(newRectF.right() / kisimage->xRes(), newRectF.bottom() / kisimage->yRes()) );

    //the rectangle to update is the union of the old rectangle et the new one
    newRect = oldRect.united(newRect);

    //we need to add adjust the rectangle because of the handles
    newRect.adjust(- maxRadiusX, - maxRadiusY, maxRadiusX, maxRadiusY);
    m_canvas->updateCanvas(newRect);
}

void KisToolTransform::refreshSpinBoxes()
{
    if (m_optWidget == 0)
        return;

    if (m_optWidget->scaleXBox)
        m_optWidget->scaleXBox->setValue(m_currentArgs.scaleX() * 100.);
    if (m_optWidget->scaleYBox)
        m_optWidget->scaleYBox->setValue(m_currentArgs.scaleY() * 100.);
    if (m_optWidget->shearXBox)
        m_optWidget->shearXBox->setValue(m_currentArgs.shearX());
    if (m_optWidget->shearYBox)
        m_optWidget->shearYBox->setValue(m_currentArgs.shearY());
    if (m_optWidget->translateXBox)
        m_optWidget->translateXBox->setValue(m_currentArgs.translate().x());
    if (m_optWidget->translateYBox)
        m_optWidget->translateYBox->setValue(m_currentArgs.translate().y());
    if (m_optWidget->aXBox)
        m_optWidget->aXBox->setValue(radianToDegree(m_currentArgs.aX()));
    if (m_optWidget->aYBox)
        m_optWidget->aYBox->setValue(radianToDegree(m_currentArgs.aY()));
    if (m_optWidget->aZBox)
        m_optWidget->aZBox->setValue(radianToDegree(m_currentArgs.aZ()));
}

void KisToolTransform::setButtonBoxDisabled(bool disabled)
{
    if (m_optWidget && m_optWidget->buttonBox) {
        QAbstractButton *applyButton = m_optWidget->buttonBox->button(QDialogButtonBox::Apply);
        QAbstractButton *resetButton = m_optWidget->buttonBox->button(QDialogButtonBox::Reset);

        if (applyButton)
            applyButton->setDisabled(disabled);
        if (resetButton)
            resetButton->setDisabled(disabled);
    }
}

void KisToolTransform::setFreeTBoxesDisabled(bool disabled)
{
	if (m_optWidget) {
		if (m_optWidget->translateXBox)
			m_optWidget->translateXBox->setDisabled(disabled);
		if (m_optWidget->translateYBox)
			m_optWidget->translateYBox->setDisabled(disabled);
		if (m_optWidget->scaleXBox)
			m_optWidget->scaleXBox->setDisabled(disabled);
		if (m_optWidget->scaleYBox)
			m_optWidget->scaleYBox->setDisabled(disabled);
		if (m_optWidget->shearXBox)
			m_optWidget->shearXBox->setDisabled(disabled);
		if (m_optWidget->shearYBox)
			m_optWidget->shearYBox->setDisabled(disabled);
		if (m_optWidget->aXBox)
			m_optWidget->aXBox->setDisabled(disabled);
		if (m_optWidget->aYBox)
			m_optWidget->aYBox->setDisabled(disabled);
		if (m_optWidget->aZBox)
			m_optWidget->aZBox->setDisabled(disabled);
		if (m_optWidget->cmbFilter)
			m_optWidget->cmbFilter->setDisabled(disabled);
		if (m_rotCenterButtons) {
			for (int i = 0; i < 9; ++i)
				if (m_rotCenterButtons->button(i))
					m_rotCenterButtons->button(i)->setDisabled(disabled);
		}
	}
}

void KisToolTransform::deactivate()
{
    KisImageWSP kisimage = image();

    updateCurrentOutline();
    m_canvas->updateCanvas(QRect(m_originalTopLeft, m_originalBottomRight));

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

        const ApplyTransformCmdData * presentCmd1 = 0;
        const TransformCmd* presentCmd2 = 0;

        if (image()->undoAdapter()->presentCommand()) {
            presentCmd1 = dynamic_cast<const ApplyTransformCmdData*>(image()->undoAdapter()->presentCommand());
            presentCmd2 = dynamic_cast<const TransformCmd*>(image()->undoAdapter()->presentCommand());
        }

        if (presentCmd1 == 0 && presentCmd2 == 0) {
            initHandles(ToolTransformArgs::FREE_TRANSFORM);
            refreshSpinBoxes();

			if (m_optWidget && m_optWidget->warpButton)
				m_optWidget->warpButton->setChecked(false);
        } else {
            // One of our commands is on top
            // We should ask for tool args and orig selection
            if (presentCmd1 != 0) {
                //we are just after an "apply" command
                //we just need to init the handles
                initHandles(presentCmd1->mode());

                setButtonBoxDisabled(true);
            } else {
                presentCmd2->transformArgs(m_currentArgs);
                m_origSelection = presentCmd2->origSelection(m_originalTopLeft, m_originalBottomRight);
                presentCmd2->origPreviews(m_origImg, m_origSelectionImg);

                setButtonBoxDisabled(false);
            }

            m_originalCenter = (m_originalTopLeft + m_originalBottomRight) / 2;
            m_previousTopLeft = m_originalTopLeft;
            m_previousBottomRight = m_originalBottomRight;
            m_scaleX_wOutModifier = m_currentArgs.scaleX();
            m_scaleY_wOutModifier = m_currentArgs.scaleY();
            m_refSize = QSizeF(0, 0); //will force the recalc of image in recalcOutline

            updateOutlineChanged();
            m_canvas->updateCanvas(QRect(m_originalTopLeft, m_originalBottomRight));
            refreshSpinBoxes();
			if (m_optWidget && m_optWidget->warpButton) {
				if (m_currentArgs.mode() == ToolTransformArgs::WARP)
					m_optWidget->warpButton->setChecked(true);
				else
					m_optWidget->warpButton->setChecked(false);
			}
        }
    }
    currentNode() =
        m_canvas->resourceManager()->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
}

void KisToolTransform::initWarpTransform()
{
    m_currentArgs = ToolTransformArgs(ToolTransformArgs::WARP, m_originalCenter, QPointF(0, 0), 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 3 , ToolTransformArgs::AFFINE_TRANSFORM, 1.0, m_originalTopLeft);

	QPointF *origPoints = m_currentArgs.origPoints();
	QPointF *transfPoints = m_currentArgs.transfPoints();
	int nbPoints = m_currentArgs.pointsPerLine() * m_currentArgs.pointsPerLine();
	m_viewOrigPoints = new QPointF[nbPoints];
	m_viewTransfPoints = new QPointF[nbPoints];

	m_gridSpaceX = (double)m_originalWidth2 * 2. / (m_currentArgs.pointsPerLine() - 1);
	m_gridSpaceY = (double)m_originalHeight2 * 2. / (m_currentArgs.pointsPerLine() - 1);
	double y = - m_originalHeight2 + m_originalCenter.y();
	for (int i = 0; i < m_currentArgs.pointsPerLine(); ++i) {
		double x = - m_originalWidth2 + m_originalCenter.x();
		for (int j = 0 ; j < m_currentArgs.pointsPerLine(); ++j) {
			origPoints[i * m_currentArgs.pointsPerLine() + j] = QPointF(x, y);
			transfPoints[i * m_currentArgs.pointsPerLine() + j] = QPointF(x, y);
			x += m_gridSpaceX;
		}
		y += m_gridSpaceY;
	}
    m_refSize = QSizeF(0, 0);

	setFreeTBoxesDisabled(true);

	if (m_optWidget && m_optWidget->warpButton)
		m_optWidget->warpButton->setChecked(true);
}

void KisToolTransform::initFreeTransform()
{
    m_previousTopLeft = m_originalTopLeft;
    m_previousBottomRight = m_originalBottomRight;
    m_currentArgs = ToolTransformArgs(ToolTransformArgs::FREE_TRANSFORM, m_originalCenter, QPointF(0, 0), 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0, ToolTransformArgs::AFFINE_TRANSFORM, 1.0, m_originalTopLeft);
    m_scaleX_wOutModifier = m_currentArgs.scaleX();
    m_scaleY_wOutModifier = m_currentArgs.scaleY();

    m_refSize = QSizeF(0, 0);

	setFreeTBoxesDisabled(false);
	if (m_optWidget && m_optWidget->warpButton)
		m_optWidget->warpButton->setChecked(false);
}

void KisToolTransform::initHandles(ToolTransformArgs::TransfMode mode)
{
    int x, y, w, h;

    KisPaintDeviceSP dev = currentNode()->paintDevice();

    //// Create a lazy copy of the current state
    //m_origDevice = new KisPaintDevice(*dev.data());
    //Q_ASSERT(m_origDevice);

    KisSelectionSP selection = currentSelection();
    if (selection) {
        QRect r = selection->selectedExactRect();
        m_origSelection = new KisSelection();
        KisPixelSelectionSP origPixelSelection = new KisPixelSelection(*selection->getOrCreatePixelSelection().data());
        m_origSelection->setPixelSelection(origPixelSelection);
        r.getRect(&x, &y, &w, &h);

        //if (selection->hasShapeSelection()) {
        //    //save the state of the shapes
        //    KisShapeSelection* shapeSelection = static_cast<KisShapeSelection*>(selection->shapeSelection());
        //    m_origSelection->setShapeSelection(shapeSelection->clone(selection.data()));
        //}
    } else {
        //we take all of the paintDevice
        dev->exactBounds(x, y, w, h);
        m_origSelection = 0;
    }
    m_originalTopLeft = QPoint(x, y);
    m_originalBottomRight = QPoint(x + w - 1, y + h - 1);
    m_originalCenter = QPointF(m_originalTopLeft + m_originalBottomRight) / 2.0;
    m_originalHeight2 = m_originalCenter.y() - m_originalTopLeft.y();
    m_originalWidth2 = m_originalCenter.x() - m_originalTopLeft.x();

	if (mode == ToolTransformArgs::WARP)
		initWarpTransform();
	else
		initFreeTransform();

    const KisImage *kisimage = image();
    m_transform = QTransform();
    //m_origImg = new QImage(m_origDevice->convertToQImage(0, x, y, w, h));
    m_origImg = new QImage(dev->convertToQImage(0, x, y, w, h));
    if (selection)
        m_origSelectionImg = new QImage(selection->convertToQImage(0, x, y, w, h));
    else {
        m_origSelectionImg = new QImage(w, h, QImage::Format_ARGB32_Premultiplied);
        m_origSelectionImg->fill(0xFFFFFFFF);
    }
    QImage alphaMask(*m_origSelectionImg);

    m_origImg->setAlphaChannel(alphaMask);
    m_currImg = QImage(*m_origImg); //create a shallow copy
    m_origSelectionImg->invertPixels();
    m_origSelectionImg->setAlphaChannel(alphaMask);
    if (m_canvas && m_canvas->viewConverter() && kisimage)
        m_refSize = m_canvas->viewConverter()->documentToView(QSizeF(1 / kisimage->xRes(), 1 / kisimage->yRes()));
    else
        m_refSize = QSizeF(1, 1);
    m_scaledOrigSelectionImg = m_origSelectionImg->scaled(m_refSize.width() * m_origSelectionImg->width(), m_refSize.height() * m_origSelectionImg->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    updateOutlineChanged();
}

void KisToolTransform::mousePressEvent(KoPointerEvent *e)
{
    KisImageWSP kisimage = image();

    if (!currentNode())
        return;

    if (kisimage && currentNode()->paintDevice() && e->button() == Qt::LeftButton) {
		QPointF mousePos = QPointF(e->point.x() * kisimage->xRes(), e->point.y() * kisimage->yRes());
		if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
			if (m_cursorOverPoint)
				m_selecting = true;
		} else {
			if (m_function == ROTATE) {
				QVector3D clickoffset(mousePos - m_rotationCenterProj);
				clickoffset = invperspective(clickoffset.x(), clickoffset.y(), m_currentPlan);
				clickoffset = invrotX(clickoffset.x(), clickoffset.y(), clickoffset.z());
				clickoffset = invrotY(clickoffset.x(), clickoffset.y(), clickoffset.z());
				m_clickangle = atan2(-clickoffset.y(), clickoffset.x());
			}

			m_selecting = true;
		}

        m_actuallyMoveWhileSelected = false;
        m_clickPoint = mousePos;
		m_prevMousePos = mousePos;
    }

    m_clickRotationCenterProj = m_rotationCenterProj;
    m_clickTopLeftProj = m_topLeftProj;
    m_clickTopRightProj = m_topRightProj;
    m_clickBottomLeftProj = m_bottomLeftProj;
    m_clickBottomRightProj = m_bottomRightProj;
    m_clickMiddleLeftProj = m_middleLeftProj;
    m_clickMiddleRightProj = m_middleRightProj;
    m_clickMiddleTopProj = m_middleTopProj;
    m_clickMiddleBottomProj = m_middleBottomProj;
    m_clickPlan = m_currentPlan;
    storeArgs(m_clickArgs);

    recalcOutline();
}

void KisToolTransform::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Meta) {
        m_function = PERSPECTIVE;
        setFunctionalCursor();
    }
}

void KisToolTransform::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);

    setTransformFunction(m_prevMousePos, event->modifiers());
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
    QVector3D dir_vect;
    int rotOctant;

	if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
		if (m_cursorOverPoint)
			useCursor(KisCursor::handCursor());
		else
			useCursor(KisCursor::arrowCursor());
	} else {
		switch (m_function) {
		case MOVE:
			useCursor(KisCursor::moveCursor());
			break;
		case ROTATE:
		case PERSPECTIVE:
			//TODO: find another cursor for perspective
			useCursor(KisCursor::rotateCursor());
			break;
		case RIGHTSCALE:
		case TOPRIGHTSCALE:
		case TOPSCALE:
		case TOPLEFTSCALE:
		case LEFTSCALE:
		case BOTTOMLEFTSCALE:
		case BOTTOMSCALE:
		case BOTTOMRIGHTSCALE:
			dir_vect = QVector3D(m_handleDir[m_function - RIGHTSCALE]);
			if (m_currentArgs.scaleX() < 0)
				dir_vect.setX(- dir_vect.x());
			if (m_currentArgs.scaleY() < 0)
				dir_vect.setY(- dir_vect.y());
			dir_vect = rotZ(dir_vect.x(), dir_vect.y(), dir_vect.z());
			dir_vect = rotY(dir_vect.x(), dir_vect.y(), dir_vect.z());
			dir_vect = rotX(dir_vect.x(), dir_vect.y(), dir_vect.z());
			dir_vect = QVector3D(perspective(dir_vect.x(), dir_vect.y(), dir_vect.z()));

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
			dir_vect = QVector3D(m_handleDir[(m_function - BOTTOMSHEAR) * 2]);
			if (m_currentArgs.scaleX() < 0)
					dir_vect.setX(- dir_vect.x());
			if (m_currentArgs.scaleY() < 0)
					dir_vect.setY(- dir_vect.y());
			dir_vect = rotZ(dir_vect.x(), dir_vect.y(), dir_vect.z());
			dir_vect = rotY(dir_vect.x(), dir_vect.y(), dir_vect.z());
			dir_vect = rotX(dir_vect.x(), dir_vect.y(), dir_vect.z());
			dir_vect = QVector3D(perspective(dir_vect.x(), dir_vect.y(), dir_vect.z()));

			rotOctant = octant(dir_vect.x(), dir_vect.y());
			useCursor(m_sizeCursors[rotOctant]);
		}
	}
}

// the interval for the dichotomy is [0, b]
// b can be positive or negative (depending on whether the scale factor
// we want to approach is supposed to be positive or negative)
// b is not necessarily a scale factor so that the resulting length is >= desired length
// the dichotomy function will increase b until it finds such a factor, before beginning the dichotomy
// v1 and v2 are the vectors which will be transformed, and we want length(v1 - v2) to be close to desiredLength
// precision is the wished precision
// maxIterations1 is the maximum number of iterations to find a "b" so that the resulting length is >= desired length
// if such a "b" is not found, the function will return 1
// if it is found, then the dichotomy will begin : it will stop once we have reached the precision or we have done maxIterations2
double KisToolTransform::dichotomyScaleY(QVector3D v1, QVector3D v2, DICHO_DROP flag, double desired, double b, double precision, double maxIterations1, double maxIterations2)
{
    double a = 0;
    int i = 0;
    double currentLength;
    double currentScaleY;
    bool b_found = false;
    QVector3D t1, t2;
    //first find b so that the length when currentScaleY = b is >= desired
    do {
        currentScaleY = b;

        t1 = v1;
        t1 = QVector3D(t1.x() * m_clickArgs.scaleX(), t1.y() * currentScaleY, t1.z());
        t1 = shear(t1.x(), t1.y(), t1.z());
        t1 = rotZ(t1.x(), t1.y(), t1.z());
        t1 = rotY(t1.x(), t1.y(), t1.z());
        t1 = rotX(t1.x(), t1.y(), t1.z());
        if (t1.z() > m_cameraPos.z()) {
            b_found = true;
            break;
        }
        t1 = QVector3D(perspective(t1.x(), t1.y(), t1.z()));

        t2 = v2;
        t2 = QVector3D(t2.x() * m_clickArgs.scaleX(), t2.y() * currentScaleY, t2.z());
        t2 = shear(t2.x(), t2.y(), t2.z());
        t2 = rotZ(t2.x(), t2.y(), t2.z());
        t2 = rotY(t2.x(), t2.y(), t2.z());
        t2 = rotX(t2.x(), t2.y(), t2.z());
        if (t2.z() > m_cameraPos.z()) {
            b_found = true;
            break;
        }
        t2 = QVector3D(perspective(t2.x(), t2.y(), t2.z()));

        t1 -= t2;

        if (flag != NONE) {
            t1 = invperspective(t1.x(), t1.y(), m_clickPlan);
            t1 = invrotX(t1.x(), t1.y(), t1.z());
            t1 = invrotY(t1.x(), t1.y(), t1.z());
            t1 = invrotZ(t1.x(), t1.y(), t1.z());
            t1 = invshear(t1.x(), t1.y(), t1.z());

            if (flag == XCOORD)
                t1.setX(0);
            else
                t1.setY(0);

            t1 = shear(t1.x(), t1.y(), t1.z());
            t1 = rotZ(t1.x(), t1.y(), t1.z());
            t1 = rotY(t1.x(), t1.y(), t1.z());
            t1 = rotX(t1.x(), t1.y(), t1.z());
            t1 = QVector3D(perspective(t1.x(), t1.y(), t1.z()));
        }

        currentLength = t1.length();
        ++i;

        if (i > maxIterations1)
            break;
        else if (currentLength < desired) {
            a = b;
            b *= 2;
        } else {
            b_found = true;
            break;
        }
    } while(true);

    if (b_found) {
        do {
            currentScaleY = (a + b) / 2.;
            t1 = v1;
            t1 = QVector3D(t1.x() * m_clickArgs.scaleX(), t1.y() * currentScaleY, t1.z());
            t1 = shear(t1.x(), t1.y(), t1.z());
            t1 = rotZ(t1.x(), t1.y(), t1.z());
            t1 = rotY(t1.x(), t1.y(), t1.z());
            t1 = rotX(t1.x(), t1.y(), t1.z());
            if (t1.z() > m_cameraPos.z()) {
                b = (a + b) / 2;
                continue;
            }
            t1 = QVector3D(perspective(t1.x(), t1.y(), t1.z()));

            t2 = v2;
            t2 = QVector3D(t2.x() * m_clickArgs.scaleX(), t2.y() * currentScaleY, t2.z());
            t2 = shear(t2.x(), t2.y(), t2.z());
            t2 = rotZ(t2.x(), t2.y(), t2.z());
            t2 = rotY(t2.x(), t2.y(), t2.z());
            t2 = rotX(t2.x(), t2.y(), t2.z());
            if (t2.z() > m_cameraPos.z()) {
                b = (a + b) / 2;
                continue;
            }
            t2 = QVector3D(perspective(t2.x(), t2.y(), t2.z()));

            t1 -= t2;

            if (flag != NONE) {
                t1 = invperspective(t1.x(), t1.y(), m_clickPlan);
                t1 = invrotX(t1.x(), t1.y(), t1.z());
                t1 = invrotY(t1.x(), t1.y(), t1.z());
                t1 = invrotZ(t1.x(), t1.y(), t1.z());
                t1 = invshear(t1.x(), t1.y(), t1.z());

                if (flag == XCOORD)
                    t1.setX(0);
                else
                    t1.setY(0);

                t1 = shear(t1.x(), t1.y(), t1.z());
                t1 = rotZ(t1.x(), t1.y(), t1.z());
                t1 = rotY(t1.x(), t1.y(), t1.z());
                t1 = rotX(t1.x(), t1.y(), t1.z());
                t1 = QVector3D(perspective(t1.x(), t1.y(), t1.z()));
            }

            currentLength = t1.length();
            ++i;

            if (i > maxIterations2 || fabs(currentLength - desired) <= precision)
                break;
            else if (currentLength < desired)
                a = (a + b) / 2;
            else
                b = (a + b) / 2;
        } while (true);

        return currentScaleY;
    } else
        return 1;
}

double KisToolTransform::dichotomyScaleX(QVector3D v1, QVector3D v2, DICHO_DROP flag, double desired, double b, double precision, double maxIterations1, double maxIterations2)
{
    double a = 0;
    int i = 0;
    double currentLength;
    double currentScaleX;
    bool b_found = false;
    QVector3D t1, t2;
    //first find b so that the length when currentScaleY = b is >= desired
    do {
        currentScaleX = b;

        t1 = v1;
        t1 = QVector3D(t1.x() * currentScaleX, t1.y() * m_clickArgs.scaleY(), t1.z());
        t1 = shear(t1.x(), t1.y(), t1.z());
        t1 = rotZ(t1.x(), t1.y(), t1.z());
        t1 = rotY(t1.x(), t1.y(), t1.z());
        t1 = rotX(t1.x(), t1.y(), t1.z());
        if (t1.z() > m_cameraPos.z()) {
            b_found = true;
            break;
        }
        t1 = QVector3D(perspective(t1.x(), t1.y(), t1.z()));

        t2 = v2;
        t2 = QVector3D(t2.x() * currentScaleX, t2.y() * m_clickArgs.scaleY(), t2.z());
        t2 = shear(t2.x(), t2.y(), t2.z());
        t2 = rotZ(t2.x(), t2.y(), t2.z());
        t2 = rotY(t2.x(), t2.y(), t2.z());
        t2 = rotX(t2.x(), t2.y(), t2.z());
        if (t2.z() > m_cameraPos.z()) {
            b_found = true;
            break;
        }
        t2 = QVector3D(perspective(t2.x(), t2.y(), t2.z()));

        t1 -= t2;

        if (flag != NONE) {
            t1 = invperspective(t1.x(), t1.y(), m_clickPlan);
            t1 = invrotX(t1.x(), t1.y(), t1.z());
            t1 = invrotY(t1.x(), t1.y(), t1.z());
            t1 = invrotZ(t1.x(), t1.y(), t1.z());
            t1 = invshear(t1.x(), t1.y(), t1.z());

            if (flag == XCOORD)
                t1.setX(0);
            else
                t1.setY(0);

            t1 = shear(t1.x(), t1.y(), t1.z());
            t1 = rotZ(t1.x(), t1.y(), t1.z());
            t1 = rotY(t1.x(), t1.y(), t1.z());
            t1 = rotX(t1.x(), t1.y(), t1.z());
            t1 = QVector3D(perspective(t1.x(), t1.y(), t1.z()));
        }

        currentLength = t1.length();
        ++i;

        if (i > maxIterations1)
            break;
        else if (currentLength < desired) {
            a = b;
            b *= 2;
        } else {
            b_found = true;
            break;
        }
    } while(true);

    if (b_found) {
        do {
            currentScaleX = (a + b) / 2.;
            t1 = v1;
            t1 = QVector3D(t1.x() * currentScaleX, t1.y() * m_clickArgs.scaleY(), t1.z());
            t1 = shear(t1.x(), t1.y(), t1.z());
            t1 = rotZ(t1.x(), t1.y(), t1.z());
            t1 = rotY(t1.x(), t1.y(), t1.z());
            t1 = rotX(t1.x(), t1.y(), t1.z());
            if (t1.z() > m_cameraPos.z()) {
                b = (a + b) / 2;
                continue;
            }
            t1 = QVector3D(perspective(t1.x(), t1.y(), t1.z()));

            t2 = v2;
            t2 = QVector3D(t2.x() * currentScaleX, t2.y() * m_clickArgs.scaleY(), t2.z());
            t2 = shear(t2.x(), t2.y(), t2.z());
            t2 = rotZ(t2.x(), t2.y(), t2.z());
            t2 = rotY(t2.x(), t2.y(), t2.z());
            t2 = rotX(t2.x(), t2.y(), t2.z());
            if (t2.z() > m_cameraPos.z()) {
                b = (a + b) / 2;
                continue;
            }
            t2 = QVector3D(perspective(t2.x(), t2.y(), t2.z()));

            t1 -= t2;

            if (flag != NONE) {
                t1 = invperspective(t1.x(), t1.y(), m_clickPlan);
                t1 = invrotX(t1.x(), t1.y(), t1.z());
                t1 = invrotY(t1.x(), t1.y(), t1.z());
                t1 = invrotZ(t1.x(), t1.y(), t1.z());
                t1 = invshear(t1.x(), t1.y(), t1.z());

                if (flag == XCOORD)
                    t1.setX(0);
                else
                    t1.setY(0);

                t1 = shear(t1.x(), t1.y(), t1.z());
                t1 = rotZ(t1.x(), t1.y(), t1.z());
                t1 = rotY(t1.x(), t1.y(), t1.z());
                t1 = rotX(t1.x(), t1.y(), t1.z());
                t1 = QVector3D(perspective(t1.x(), t1.y(), t1.z()));
            }

            currentLength = t1.length();
            ++i;

            if (i > maxIterations2 || fabs(currentLength - desired) <= precision)
                break;
            else if (currentLength < desired)
                a = (a + b) / 2;
            else
                b = (a + b) / 2;
        } while (true);

        return currentScaleX;
    } else
        return 1;
}

static inline void switchPoints(QPointF *p1, QPointF *p2)
{
    QPointF tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
}

void KisToolTransform::setTransformFunction(QPointF mousePos, Qt::KeyboardModifiers modifiers)
{
    recalcOutline();

	if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
		double handleRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_handleRadius);
		double handleRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_handleRadius);
		double handleRadius = (handleRadiusX > handleRadiusY) ? handleRadiusX : handleRadiusY;
		double handleRadiusSq = handleRadius * handleRadius; // square it so it fits with distsq

		m_cursorOverPoint = false;
		int nbPoints = m_currentArgs.pointsPerLine() * m_currentArgs.pointsPerLine();
		for (int i = 0; i < nbPoints; ++i) {
			if (distsq(mousePos, m_currentArgs.transfPoints()[i]) <= handleRadiusSq) {
				m_cursorOverPoint = true;
				m_pointUnderCursor = i;
				break;
			}
		}

		QRectF r = calcWarpBoundRect();
		m_topLeftProj = r.topLeft();
		m_bottomRightProj = r.bottomRight();
		m_topRightProj = QPointF(r.right(), r.top());
		m_bottomLeftProj = QPointF(r.left(), r.bottom());
	} else {
		if (modifiers & Qt::MetaModifier) {
			m_function = PERSPECTIVE;
			setFunctionalCursor();
			return;
		}

		QPointF topLeft, topRight, bottomLeft, bottomRight;
		QPointF tmp;

		//depending on the scale factor, we need to exchange left<->right and top<->bottom
		if (m_currentArgs.scaleX() > 0) {
			topLeft = m_topLeftProj;
			bottomLeft = m_bottomLeftProj;
			topRight = m_topRightProj;
			bottomRight = m_bottomRightProj;
		} else {
			topLeft = m_topRightProj;
			bottomLeft = m_bottomRightProj;
			topRight = m_topLeftProj;
			bottomRight = m_bottomLeftProj;
		}
		if (m_currentArgs.scaleY() < 0) {
			switchPoints(&topLeft, &bottomLeft);
			switchPoints(&topRight, &bottomRight);
		}

		//depending on the x & y rotations, we also need to exchange left<->right ant top<->bottom
		//first we constraint the angles in [0, 2*M_PI[
		double piX2 = 2 * M_PI;
		double aX = m_currentArgs.aX();
		double aY = m_currentArgs.aY();
		if (aX <= 0 || aX > piX2) {
			aX = fmod(aX, piX2);
			if (aX < 0)
				aX += piX2;
			m_currentArgs.setAX(aX);
		}
		if (aY <= 0 || aY > piX2) {
			aY = fmod(aY, piX2);
			if (aY < 0)
				aY += piX2;
			m_currentArgs.setAY(aY);
		}

		if (m_currentArgs.aX() >= M_PI / 2 && m_currentArgs.aX() < 3 * M_PI / 2) {
			switchPoints(&topLeft, &bottomLeft);
			switchPoints(&topRight, &bottomRight);
		}

		if (m_currentArgs.aY() > M_PI / 2 && m_currentArgs.aY() < 3 * M_PI / 2) {
			switchPoints(&topLeft, &topRight);
			switchPoints(&bottomLeft, &bottomRight);
		}

		if (det(mousePos - topLeft, topRight - topLeft) > 0)
			m_function = ROTATE;
		else if (det(mousePos - topRight, bottomRight - topRight) > 0)
			m_function = ROTATE;
		else if (det(mousePos - bottomRight, bottomLeft - bottomRight) > 0)
			m_function = ROTATE;
		else if (det(mousePos - bottomLeft, topLeft - bottomLeft) > 0)
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

		if (distsq(mousePos, m_middleTopProj) <= handleRadiusSq)
			m_function = TOPSCALE;
		if (distsq(mousePos, m_topRightProj) <= handleRadiusSq)
			m_function = TOPRIGHTSCALE;
		if (distsq(mousePos, m_middleRightProj) <= handleRadiusSq)
			m_function = RIGHTSCALE;
		if (distsq(mousePos, m_bottomRightProj) <= handleRadiusSq)
			m_function = BOTTOMRIGHTSCALE;
		if (distsq(mousePos, m_middleBottomProj) <= handleRadiusSq)
			m_function = BOTTOMSCALE;
		if (distsq(mousePos, m_bottomLeftProj) <= handleRadiusSq)
			m_function = BOTTOMLEFTSCALE;
		if (distsq(mousePos, m_middleLeftProj) <= handleRadiusSq)
			m_function = LEFTSCALE;
		if (distsq(mousePos, m_topLeftProj) <= handleRadiusSq)
			m_function = TOPLEFTSCALE;
		if (distsq(mousePos, m_rotationCenterProj) <= rotationCenterRadius)
			m_function = MOVECENTER;

		if (m_function == ROTATE || m_function == MOVE) {
			//We check for shearing only if we aren't near a handle (for scale) or the rotation center
			QVector3D v = QVector3D(mousePos - m_currentArgs.translate());
			v = invperspective(v.x(), v.y(), m_currentPlan);
			QPointF t = invTransformVector(v).toPointF();
			t += m_originalCenter;

			if (t.x() >= m_originalTopLeft.x() && t.x() <= m_originalBottomRight.x()) {
				if (fabs(t.y() - m_originalTopLeft.y()) <= handleRadius)
					m_function = TOPSHEAR;
				else if (fabs(t.y() - m_originalBottomRight.y()) <= handleRadius)
					m_function = BOTTOMSHEAR;
			} else if (t.y() >= m_originalTopLeft.y() && t.y() <= m_originalBottomRight.y()) {
				if (fabs(t.x() - m_originalTopLeft.x()) <= handleRadius)
					m_function = LEFTSHEAR;
				else if (fabs(t.x() - m_originalBottomRight.x()) <= handleRadius)
					m_function = RIGHTSHEAR;
			}
		}
	}

    setFunctionalCursor();
}

static inline QPointF clipInRect(QPointF p, QRectF r) {
	QPointF center = r.center();
	QPointF t = p - center;
	r.translate(- center);

	if (t.y() != 0) {
		if (t.x() != 0) {
			double slope = t.y() / t.x();

			if (t.x() < r.left()) {
				t.setY(r.left() * slope);
				t.setX(r.left());
			} else if (t.x() > r.right()) {
				t.setY(r.right() * slope);
				t.setX(r.right());
			}

			if (t.y() < r.top()) {
				t.setX(r.top() / slope);
				t.setY(r.top());
			} else if (t.y() > r.bottom()) {
				t.setX(r.bottom() / slope);
				t.setY(r.bottom());
			}
		} else {
			if (t.y() < r.top())
				t.setY(r.top());
			else if (t.y() > r.bottom())
				t.setY(r.bottom());
		}
	} else {
		if (t.x() < r.left())
			t.setX(r.left());
		else if (t.x() > r.right())
			t.setX(r.right());
	}

	t += center;

	return t;
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

		if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
			QPointF &origPoint = m_currentArgs.origPoints()[m_pointUnderCursor];
			QPointF &currPoint = m_currentArgs.transfPoints()[m_pointUnderCursor];
			QPointF &viewCurrPoint = m_viewTransfPoints[m_pointUnderCursor];
			QRectF clipRect(origPoint.x() - m_gridSpaceX / 2., origPoint.y() - m_gridSpaceY / 2., m_gridSpaceX, m_gridSpaceY);
			currPoint = clipInRect(mousePos, clipRect);
			viewCurrPoint = m_canvas->viewConverter()->documentToView(QPointF(currPoint.x() / kisimage->xRes(), currPoint.y() / kisimage->yRes()));

			if (currPoint.x() < m_clickTopLeftProj.x()) {
				m_topLeft.setX(currPoint.x());
				m_bottomLeft.setX(currPoint.x());
			} else if (currPoint.x() > m_clickBottomRightProj.x()) {
				m_topRight.setX(currPoint.x());
				m_bottomRight.setX(currPoint.x());
			}

			if (currPoint.y() < m_clickTopLeftProj.y()) {
				m_topLeft.setY(currPoint.y());
				m_topRight.setY(currPoint.y());
			} else if (currPoint.y() > m_clickBottomRightProj.y()) {
				m_bottomLeft.setY(currPoint.y());
				m_bottomRight.setY(currPoint.y());
			}
		} else {
			int signY = 1, signX = 1;
			QVector3D t(0,0,0);
			QVector3D v1(0, 0, 0), v2(0, 0, 0), v3(0, 0, 0), v4(0, 0, 0);
			QVector3D v1Proj(0, 0, 0), newV1Proj(0, 0, 0);
			QVector3D v2Proj(0, 0, 0), v3Proj(0, 0, 0), v4Proj(0, 0, 0);

			switch (m_function) {
			case MOVE:
				t = QVector3D(mousePos - m_clickPoint);
				if (e->modifiers() & Qt::ShiftModifier) {
					if (e->modifiers() & Qt::ControlModifier) {
						t = invperspective(t.x(), t.y(), m_currentPlan);
						t = invTransformVector(t.x(), t.y(), t.z()); //go to local/object space
						if (fabs(t.x()) >= fabs(t.y()))
							t.setY(0);
						else
							t.setX(0);
						t = transformVector(t.x(), t.y(), t.z()); //go back to global space
						t = QVector3D(perspective(t.x(), t.y(), t.z()));
					} else {
						if (fabs(t.x()) >= fabs(t.y()))
							t.setY(0);
						else
							t.setX(0);
					}
				}

				m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());
				m_optWidget->translateXBox->setValue(m_currentArgs.translate().x());
				m_optWidget->translateYBox->setValue(m_currentArgs.translate().y());
				break;
			case ROTATE:
				{
					t = invperspective(mousePos.x() - m_clickRotationCenterProj.x(), mousePos.y() - m_clickRotationCenterProj.y(), m_clickPlan);
					t = invrotX(t.x(), t.y(), t.z());
					t = invrotY(t.x(), t.y(), t.z());
					double theta = m_clickangle - atan2(- t.y(), t.x());
					if (e->modifiers() & Qt::ShiftModifier) {
						int quotient = theta * 12 / M_PI;
						theta = quotient * M_PI / 12;
					}

					m_currentArgs.setAZ(m_clickArgs.aZ() + theta);

					//we want the rotation center projection to be unchanged after rotation
					//thus we calculate it's new projection, and add the opposite translation to the current translation vector
					m_cosaZ = cos(m_currentArgs.aZ()); //update the cos/sin for transformation
					m_sinaZ = sin(m_currentArgs.aZ());
					QVector3D rotationCenterProj = transformVector(QVector3D(m_currentArgs.rotationCenterOffset()));
					rotationCenterProj = QVector3D(perspective(rotationCenterProj.x(), rotationCenterProj.y(), rotationCenterProj.z()) + m_clickArgs.translate());
					t= QVector3D(m_clickRotationCenterProj) - rotationCenterProj;

					m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

					m_optWidget->translateXBox->setValue(m_currentArgs.translate().x());
					m_optWidget->translateYBox->setValue(m_currentArgs.translate().y());
					m_optWidget->aZBox->setValue(radianToDegree(m_currentArgs.aZ()));
				}
				break;
			case PERSPECTIVE:
				{
					t = QVector3D(mousePos.x() - m_clickPoint.x(), mousePos.y() - m_clickPoint.y(), 0);
					double thetaX = - t.y() * M_PI / m_originalWidth2 / 2 / m_currentArgs.scaleX();

					if (e->modifiers() & Qt::ShiftModifier) {
						int quotient = thetaX * 12 / M_PI;
						thetaX = quotient * M_PI / 12;
					}

					m_currentArgs.setAX(m_clickArgs.aX() + thetaX);
					m_cosaX = cos(m_currentArgs.aX()); //update the cos/sin for transformation
					m_sinaX = sin(m_currentArgs.aX());
					t = invrotX(t.x(), t.y(), t.z());
					double thetaY = t.x() * M_PI / m_originalHeight2 / 2 / m_currentArgs.scaleY();

					if (e->modifiers() & Qt::ShiftModifier) {
						int quotient = thetaY * 12 / M_PI;
						thetaY = quotient * M_PI / 12;
					}

					m_currentArgs.setAY(m_clickArgs.aY() + thetaY);


					//we want the rotation center projection to be unchanged after rotation
					//thus we calculate it's new projection, and add the opposite translation to the current translation vector
					m_cosaY = cos(m_currentArgs.aY());
					m_sinaY = sin(m_currentArgs.aY());
					QVector3D rotationCenterProj = transformVector(QVector3D(m_currentArgs.rotationCenterOffset()));
					rotationCenterProj = QVector3D(perspective(rotationCenterProj.x(), rotationCenterProj.y(), rotationCenterProj.z()) + m_clickArgs.translate());
					t= QVector3D(m_clickRotationCenterProj) - rotationCenterProj;

					m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

					m_optWidget->translateXBox->setValue(m_currentArgs.translate().x());
					m_optWidget->translateYBox->setValue(m_currentArgs.translate().y());
					m_optWidget->aXBox->setValue(radianToDegree(m_currentArgs.aX()));
					m_optWidget->aYBox->setValue(radianToDegree(m_currentArgs.aY()));
				}
				break;
			case TOPSCALE:
			case BOTTOMSCALE:
				if (m_function == TOPSCALE) {
					signY = -1;
					//we want the result of v transformed to be equal to v1Proj (i.e. here we want the projection of the middle bottom point to be unchanged)
					v1 = QVector3D((double)(m_originalTopLeft.x() + m_originalBottomRight.x()) / 2.0, m_originalBottomRight.y(), 0) - QVector3D(m_originalCenter);
					v2 = QVector3D((double)(m_originalTopLeft.x() + m_originalBottomRight.x()) / 2.0, m_originalTopLeft.y(), 0) - QVector3D(m_originalCenter);
					//we save the projection, and we'll adjust the translation at the end
					v1Proj = QVector3D(m_clickMiddleBottomProj);
				}
				else {
					signY = 1;
					v1 = QVector3D((double)(m_originalTopLeft.x() + m_originalBottomRight.x()) / 2.0, m_originalTopLeft.y(), 0) - QVector3D(m_originalCenter);
					v2 = QVector3D((double)(m_originalTopLeft.x() + m_originalBottomRight.x()) / 2.0, m_originalBottomRight.y(), 0) - QVector3D(m_originalCenter);
					v1Proj = QVector3D(m_clickMiddleTopProj);
				}

				//first we need to get the scaleY factor
				//we cannot find it directly when there is perspective : we calculate the desired length, and search by dichotomy the corresponding scale factor
				if (m_clickArgs.aX() || m_clickArgs.aY()) {
					//there is a perspective projection
					//we have to find the scale factor the hard way

					t = QVector3D(mousePos) - v1Proj;

					double tmp = t.y();
					t = invperspective(t.x(), t.y(), m_clickPlan);
					if ( t.y() * tmp < 0) {
						//the invert perspective changed the orientation
						//we need to flip it
						signY *= -1;
					}
					t = invrotX(t.x(), t.y(), t.z());
					t = invrotY(t.x(), t.y(), t.z());
					t = invrotZ(t.x(), t.y(), t.z());
					t = invshear(t.x(), t.y(), t.z());

					double b;
					if (signY * t.y() < 0)
						b = - fabs(t.y() / m_originalHeight2 / 2.);
					else
						b = fabs(t.y() / m_originalHeight2 / 2.);

					//we keep the vertical component only
					t = QVector3D(0, t.y(), 0);

					//and we transform the vector again : that way, we can get the desired length
					//(the distance between the middle top and middle bottom)
					t = shear(t.x(), t.y(), t.z());
					t = rotZ(t.x(), t.y(), t.z());
					t = rotY(t.x(), t.y(), t.z());
					t = rotX(t.x(), t.y(), t.z());
					t = QVector3D(perspective(t.x(), t.y(), t.z()));

					double desiredLength = t.length();
					m_scaleY_wOutModifier = dichotomyScaleY(v1, v2, NONE, desiredLength, b, 0.00001, 64, 10);
				} else {
					//we invert the movement vector from the position of the decoration at click
					t = QVector3D(mousePos) - v1Proj;
					t = invperspective(t.x(), t.y(), m_clickPlan);
					t = invrotX(t.x(), t.y(), t.z());
					t = invrotY(t.x(), t.y(), t.z());
					t = invrotZ(t.x(), t.y(), t.z());
					t = invshear(t.x(), t.y(), t.z());

					t *= signY;

					m_scaleY_wOutModifier = t.y() / m_originalHeight2 / 2.;
				}

				//applies the shift modifier
				if (e->modifiers() & Qt::ShiftModifier) {
					double a_scaleY = fabs(m_scaleY_wOutModifier);

					m_currentArgs.setScaleX((m_scaleX_wOutModifier > 0) ? a_scaleY : -a_scaleY);
					m_currentArgs.setScaleY(m_scaleY_wOutModifier);
				} else
					m_currentArgs.setScaleY(m_scaleY_wOutModifier);

				newV1Proj = transformVector(v1);
				newV1Proj = QVector3D(perspective(newV1Proj.x(), newV1Proj.y(), newV1Proj.z()) + m_clickArgs.translate());
				t= v1Proj - newV1Proj;
				m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

				m_optWidget->translateXBox->setValue(m_currentArgs.translate().x());
				m_optWidget->translateYBox->setValue(m_currentArgs.translate().y());
				m_optWidget->scaleXBox->setValue(m_currentArgs.scaleX() * 100.);
				m_optWidget->scaleYBox->setValue(m_currentArgs.scaleY() * 100.);
				break;
			case LEFTSCALE:
			case RIGHTSCALE:
				if (m_function == LEFTSCALE) {
					signX = -1;
					v1 = QVector3D(m_originalBottomRight.x(), (double)(m_originalTopLeft.y() + m_originalBottomRight.y()) / 2.0, 0) - QVector3D(m_originalCenter);
					v2 = QVector3D(m_originalTopLeft.x(), (double)(m_originalTopLeft.y() + m_originalBottomRight.y()) / 2.0, 0) - QVector3D(m_originalCenter);
					v1Proj = QVector3D(m_clickMiddleRightProj);
				}
				else {
					signX = 1;
					v1 = QVector3D(m_originalTopLeft.x(), (double)(m_originalTopLeft.y() + m_originalBottomRight.y()) / 2.0, 0) - QVector3D(m_originalCenter);
					v2 = QVector3D(m_originalBottomRight.x(), (double)(m_originalTopLeft.y() + m_originalBottomRight.y()) / 2.0, 0) - QVector3D(m_originalCenter);
					v1Proj = QVector3D(m_clickMiddleLeftProj);
				}

				if (m_clickArgs.aX() || m_clickArgs.aY()) {
					t = QVector3D(mousePos) - v1Proj;
					double tmp = t.x();
					t = invperspective(t.x(), t.y(), m_clickPlan);
					if ( t.x() * tmp < 0)
						signX *= -1;
					t = invrotX(t.x(), t.y(), t.z());
					t = invrotY(t.x(), t.y(), t.z());
					t = invrotZ(t.x(), t.y(), t.z());
					t = invshear(t.x(), t.y(), t.z());

					double b;
					if (signX * t.x() < 0)
						b = - fabs(t.x() / m_originalWidth2 / 2.);
					else
						b = fabs(t.x() / m_originalWidth2 / 2.);

					t = QVector3D(t.x(), 0, 0);

					t = shear(t.x(), t.y(), t.z());
					t = rotZ(t.x(), t.y(), t.z());
					t = rotY(t.x(), t.y(), t.z());
					t = rotX(t.x(), t.y(), t.z());
					t = QVector3D(perspective(t.x(), t.y(), t.z()));

					double desiredLength = t.length();
					m_scaleX_wOutModifier = dichotomyScaleX(v1, v2, NONE, desiredLength, b, 0.00001, 64, 10);
				} else {
					//we invert the movement vector from the position of the decoration at click
					t = QVector3D(mousePos) - v1Proj;
					t = invperspective(t.x(), t.y(), m_clickPlan);
					t = invrotX(t.x(), t.y(), t.z());
					t = invrotY(t.x(), t.y(), t.z());
					t = invrotZ(t.x(), t.y(), t.z());
					t = invshear(t.x(), t.y(), t.z());

					t *= signX;

					m_scaleX_wOutModifier = t.x() / m_originalWidth2 / 2.;
				}

				//applies the shift modifier
				if (e->modifiers() & Qt::ShiftModifier) {
					double a_scaleX = fabs(m_scaleX_wOutModifier);

					m_currentArgs.setScaleY((m_scaleY_wOutModifier > 0) ? a_scaleX : -a_scaleX);
					m_currentArgs.setScaleX(m_scaleX_wOutModifier);
				} else
					m_currentArgs.setScaleX(m_scaleX_wOutModifier);

				newV1Proj = transformVector(v1);
				newV1Proj = QVector3D(perspective(newV1Proj.x(), newV1Proj.y(), newV1Proj.z()) + m_clickArgs.translate());
				t= v1Proj - newV1Proj;
				m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

				m_optWidget->translateXBox->setValue(m_currentArgs.translate().x());
				m_optWidget->translateYBox->setValue(m_currentArgs.translate().y());
				m_optWidget->scaleXBox->setValue(m_currentArgs.scaleX() * 100.);
				m_optWidget->scaleYBox->setValue(m_currentArgs.scaleY() * 100.);
				break;
			case TOPRIGHTSCALE:
			case BOTTOMRIGHTSCALE:
			case TOPLEFTSCALE:
			case BOTTOMLEFTSCALE:
				switch(m_function) {
				case TOPRIGHTSCALE:
					signY = -1;
					v1 = QVector3D(m_originalTopLeft.x(), m_originalBottomRight.y(), 0) - QVector3D(m_originalCenter);
					v2 = QVector3D(m_originalBottomRight.x(), m_originalTopLeft.y(),0) - QVector3D(m_originalCenter);
					v1Proj = QVector3D(m_clickBottomLeftProj);
					break;
				case BOTTOMRIGHTSCALE:
					v1 = QVector3D(m_originalTopLeft - m_originalCenter);
					v2 = QVector3D(m_originalBottomRight - m_originalCenter);
					v1Proj = QVector3D(m_clickTopLeftProj);
					break;
				case TOPLEFTSCALE:
					signX = -1;
					signY = -1;
					v1 = QVector3D(m_originalBottomRight - m_originalCenter);
					v2 = QVector3D(m_originalTopLeft - m_originalCenter);
					v1Proj = QVector3D(m_clickBottomRightProj);
					break;
				case BOTTOMLEFTSCALE:
					signX = -1;
					v1 = QVector3D(m_originalBottomRight.x(), m_originalTopLeft.y(), 0) - QVector3D(m_originalCenter);
					v2 = QVector3D(m_originalTopLeft.x(), m_originalBottomRight.y(), 0) - QVector3D(m_originalCenter);
					v1Proj = QVector3D(m_clickTopRightProj);
					break;
				default:
					break;
				}

				//we invert the movement vector from the position of the decoration at click
				t = QVector3D(mousePos) - v1Proj;
				t = invperspective(t.x(), t.y(), m_clickPlan);
				t = invrotX(t.x(), t.y(), t.z());
				t = invrotY(t.x(), t.y(), t.z());
				t = invrotZ(t.x(), t.y(), t.z());
				t = invshear(t.x(), t.y(), t.z());

				m_scaleX_wOutModifier = signX * t.x() / m_originalWidth2 / 2.;
				m_scaleY_wOutModifier = signY * t.y() / m_originalHeight2 / 2.;

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

				newV1Proj = transformVector(v1);
				newV1Proj = QVector3D(perspective(newV1Proj.x(), newV1Proj.y(), newV1Proj.z()) + m_clickArgs.translate());
				t= v1Proj - newV1Proj;
				m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

				m_optWidget->translateXBox->setValue(m_currentArgs.translate().x());
				m_optWidget->translateYBox->setValue(m_currentArgs.translate().y());
				m_optWidget->scaleXBox->setValue(m_currentArgs.scaleX() * 100.);
				m_optWidget->scaleYBox->setValue(m_currentArgs.scaleY() * 100.);
				break;
			case MOVECENTER:
				t = QVector3D(mousePos - m_currentArgs.translate());

				if (e->modifiers() & Qt::ShiftModifier) {
					if (e->modifiers() & Qt::ControlModifier) {
						//we apply the constraint before going to local space
						if (fabs(t.x()) >= fabs(t.y()))
							t.setY(0);
						else
							t.setX(0);
						t = invperspective(t.x(), t.y(), m_clickPlan);
						t = invTransformVector(t.x(), t.y(), t.z()); //go to local space
					} else {
						t = invperspective(t.x(), t.y(), m_clickPlan);
						t = invTransformVector(t.x(), t.y(), t.z()); //go to local space
						if (fabs(t.x()) >= fabs(t.y()))
							t.setY(0);
						else
							t.setX(0);
						//stay in local space because the center offset is taken in local space
					}
				} else {
					t = invperspective(t.x(), t.y(), m_clickPlan);
					t = invTransformVector(t.x(), t.y(), t.z());
				}

				//now we need to clip t in the rectangle of the tool
				t = QVector3D(clipInRect(t.toPointF(), QRectF(QPointF(- m_originalWidth2, - m_originalHeight2), QPointF(m_originalWidth2, m_originalHeight2))));

				m_currentArgs.setRotationCenterOffset(t.toPointF());

				if (m_rotCenterButtons->checkedId() >= 0 && m_rotCenterButtons->checkedId() < 9)
					m_rotCenterButtons->button(9)->setChecked(true); //uncheck the current checked button
				break;
			case TOPSHEAR:
				signX = -1;
			case BOTTOMSHEAR:
				t = QVector3D(mousePos - m_clickPoint);

				t = invperspective(t.x(), t.y(), m_clickPlan);
				t = invrotX(t.x(), t.y(), t.z());
				t = invrotY(t.x(), t.y(), t.z());
				t = invrotZ(t.x(), t.y(), t.z());
				//we do not use invShear because we want to use the clickArgs shear factors
				t.setY(t.y() - t.x() * m_clickArgs.shearY());
				t.setX(t.x() - t.y() * m_clickArgs.shearX());

				dx = signX * m_clickArgs.shearX() * m_currentArgs.scaleY() * m_originalHeight2; //get the dx pixels corresponding to the current shearX factor
				dx += t.x(); //add the horizontal movement
				m_currentArgs.setShearX(signX * dx / m_currentArgs.scaleY() / m_originalHeight2); // calculate the new shearX factor

				m_optWidget->shearXBox->setValue(m_currentArgs.shearX());
				break;
			case LEFTSHEAR:
				signY = -1;
			case RIGHTSHEAR:
				t = QVector3D(mousePos - m_clickPoint);
				t = invperspective(t.x(), t.y(), m_clickPlan);
				t = invrotX(t.x(), t.y(), t.z());
				t = invrotY(t.x(), t.y(), t.z());
				t = invrotZ(t.x(), t.y(), t.z());
				t.setY(t.y() - t.x() * m_clickArgs.shearY());
				t.setX(t.x() - t.y() * m_clickArgs.shearX());

				dy = signY *  m_clickArgs.shearY() * m_currentArgs.scaleX() * m_originalWidth2; //get the dx pixels corresponding to the current shearX factor
				dy += t.y(); //add the horizontal movement
				m_currentArgs.setShearY(signY * dy / m_currentArgs.scaleX() / m_originalWidth2); // calculate the new shearX factor

				m_optWidget->shearYBox->setValue(m_currentArgs.shearY());
				break;
			}
		}

        updateOutlineChanged();
    } else {
        setTransformFunction(mousePos, e->modifiers());
    }

    m_prevMousePos = mousePos;
}

void KisToolTransform::mouseReleaseEvent(KoPointerEvent */*e*/)
{
    m_selecting = false;

    if (m_actuallyMoveWhileSelected) {
		if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
			recalcOutline();
			transform();
		} else {
			if (m_imageTooBig) {
				restoreArgs(m_clickArgs);
				updateOutlineChanged();
			} else
				transform();

			m_scaleX_wOutModifier = m_currentArgs.scaleX();
			m_scaleY_wOutModifier = m_currentArgs.scaleY();
		}
    }
}

void KisToolTransform::recalcOutline()
{
	if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
		KisImageSP kisimage = image();
		QSizeF s(1 / kisimage->xRes(), 1 / kisimage->yRes());
		s = m_canvas->viewConverter()->documentToView(s);

		int nbPoints = m_currentArgs.pointsPerLine() * m_currentArgs.pointsPerLine();
		for (int i = 0; i < nbPoints; ++i) {
			m_viewTransfPoints[i] = m_canvas->viewConverter()->documentToView(QPointF(m_currentArgs.transfPoints()[i].x() / kisimage->xRes(), m_currentArgs.transfPoints()[i].y() / kisimage->yRes()));
			m_viewOrigPoints[i] = m_canvas->viewConverter()->documentToView(QPointF(m_currentArgs.origPoints()[i].x() / kisimage->xRes(), m_currentArgs.origPoints()[i].y() / kisimage->yRes()));
		}

		QTransform scaleTransform;
		scaleTransform.scale(s.width(), s.height());
		m_currImg = m_origImg->transformed(scaleTransform);
		QPointF origtopleft = m_canvas->viewConverter()->documentToView(QPointF(m_originalTopLeft.x() / kisimage->xRes(), m_originalTopLeft.y() / kisimage->yRes()));
		QPointF warptranslate;
		m_currImg = KisWarpTransformWorker::affineTransformation(&m_currImg, m_currentArgs.pointsPerLine(), m_currentArgs.pointsPerLine(), m_viewOrigPoints, m_viewTransfPoints, m_currentArgs.alpha(), origtopleft, &warptranslate);
		warptranslate = m_canvas->viewConverter()->viewToDocument(warptranslate);
		m_currentArgs.setPreviewPos(QPointF(warptranslate.x() * kisimage->xRes(), warptranslate.y() * kisimage->yRes()));

		m_topLeftProj = m_topLeft.toPointF();
		m_topRightProj = m_topRight.toPointF();
		m_bottomRightProj = m_bottomRight.toPointF();
		m_bottomLeftProj = m_bottomLeft.toPointF();
	} else {
		QVector3D t, v;
		QVector3D translate3D(m_currentArgs.translate());
		QVector3D d;

		m_sinaX = sin(m_currentArgs.aX());
		m_cosaX = cos(m_currentArgs.aX());
		m_sinaY = sin(m_currentArgs.aY());
		m_cosaY = cos(m_currentArgs.aY());
		m_sinaZ = sin(m_currentArgs.aZ());
		m_cosaZ = cos(m_currentArgs.aZ());

		QPointF prev_topLeft, prev_topRight, prev_bottomRight, prev_bottomLeft;
		KisImageSP kisimage = image();
		QSizeF s(1 / kisimage->xRes(), 1 / kisimage->yRes());
		s = m_canvas->viewConverter()->documentToView(s);

		v = QVector3D(m_originalTopLeft - m_originalCenter);
		QPointF tmp1 = m_originalTopLeft - m_originalCenter;
		t = transformVector(v);
		QVector3D temp = t;
		m_topLeft = t + translate3D;
		m_topLeftProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();
		t = QVector3D(perspective(t.x(), t.y(), t.z()));
		t = prev_transformVector(v, s.width(), s.height());
		prev_topLeft = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

		v = QVector3D(m_originalBottomRight.x() - m_originalCenter.x(), m_originalTopLeft.y() - m_originalCenter.y(), 0);
		t = transformVector(v);
		m_topRight = t + translate3D;
		m_topRightProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();
		t = prev_transformVector(v, s.width(), s.height());
		prev_topRight = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

		v = QVector3D(m_originalTopLeft.x() - m_originalCenter.x(), m_originalBottomRight.y() - m_originalCenter.y(), 0);
		t = transformVector(v);
		m_bottomLeft = t + translate3D;
		m_bottomLeftProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();
		t = prev_transformVector(v, s.width(), s.height());
		prev_bottomLeft = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

		v = QVector3D(m_originalBottomRight - m_originalCenter);
		t = transformVector(v);
		m_bottomRight = t + translate3D;
		m_bottomRightProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();
		t = prev_transformVector(v, s.width(), s.height());
		prev_bottomRight = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

		v = QVector3D(m_originalTopLeft.x() - m_originalCenter.x(), (m_originalTopLeft.y() + m_originalBottomRight.y()) / 2.0 - m_originalCenter.y(), 0);
		t = transformVector(v);
		m_middleLeft = t + translate3D;
		m_middleLeftProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

		v = QVector3D(m_originalBottomRight.x() - m_originalCenter.x(), (m_originalTopLeft.y() + m_originalBottomRight.y()) / 2.0 - m_originalCenter.y(), 0);
		t = transformVector(v);
		m_middleRight = t + translate3D;
		m_middleRightProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

		v = QVector3D((m_originalTopLeft.x() + m_originalBottomRight.x()) / 2.0 - m_originalCenter.x(), m_originalTopLeft.y() - m_originalCenter.y(), 0);
		t = transformVector(v);
		m_middleTop = t + translate3D;
		m_middleTopProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

		v = QVector3D((m_originalTopLeft.x() + m_originalBottomRight.x()) / 2.0 - m_originalCenter.x(), m_originalBottomRight.y() - m_originalCenter.y(), 0);
		t = transformVector(v);
		m_middleBottom = t + translate3D;
		m_middleBottomProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

		v = QVector3D(m_currentArgs.rotationCenterOffset());
		t = transformVector(v);
		m_rotationCenter = t + translate3D;
		m_rotationCenterProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

		QVector3D v1 = m_topRight - m_topLeft;
		QVector3D v2 = m_bottomLeft - m_topLeft;
		m_currentPlan = QVector3D::crossProduct(v1, v2);
		m_currentPlan.normalize();

		//check whether image is too big to be displayed or not
		QPointF minmaxZ = minMaxZ(m_topLeft, m_topRight, m_bottomRight, m_bottomLeft);
		if (minmaxZ.y() >= m_cameraPos.z() * 0.9) {
			m_imageTooBig = true;
			if (m_optWidget && m_optWidget->tooBigLabel)
				m_optWidget->tooBigLabel->show();
			return;
		} else {
			m_imageTooBig = false;
			if (m_optWidget && m_optWidget->tooBigLabel)
				m_optWidget->tooBigLabel->hide();
		}

		QMatrix4x4 m;

		m.scale(s.width(), s.height());
		m.rotate(180. * m_currentArgs.aX() / M_PI, QVector3D(1, 0, 0));
		m.rotate(180. * m_currentArgs.aY() / M_PI, QVector3D(0, 1, 0));
		m.rotate(180. * m_currentArgs.aZ() / M_PI, QVector3D(0, 0, 1));
		m_transform = QTransform();
		m_transform.shear(0, m_currentArgs.shearY());
		m_transform.shear(m_currentArgs.shearX(), 0);
		QMatrix4x4 tmp_matrix = QMatrix4x4(m_transform);
		tmp_matrix.optimize();
		m *= tmp_matrix;
		m.translate(- m_originalWidth2 * m_currentArgs.scaleX(), - m_originalHeight2 * m_currentArgs.scaleY(), 0);
		m.scale(m_currentArgs.scaleX(), m_currentArgs.scaleY());
		m_transform = m.toTransform(m_cameraPos.z());
		m_currImg = m_origImg->transformed(m_transform);
	}
}

void KisToolTransform::paint(QPainter& gc, const KoViewConverter &converter)
{
    QPen old = gc.pen();
    QPen pen(Qt::SolidLine);
    pen.setWidth(0);

    KisImageWSP kisimage = image();
    if (kisimage == NULL)
        return;

	if (m_currentArgs.mode() == ToolTransformArgs::FREE_TRANSFORM) {
		QSizeF newRefSize = converter.documentToView(QSizeF(1 / kisimage->xRes(), 1 / kisimage->yRes()));
		QPointF topleft = converter.documentToView(QPointF(m_topLeftProj.x() / kisimage->xRes(), m_topLeftProj.y() / kisimage->yRes()));
		QPointF topright = converter.documentToView(QPointF(m_topRightProj.x() / kisimage->xRes(), m_topRightProj.y() / kisimage->yRes()));
		QPointF bottomleft = converter.documentToView(QPointF(m_bottomLeftProj.x() / kisimage->xRes(), m_bottomLeftProj.y() / kisimage->yRes()));
		QPointF bottomright = converter.documentToView(QPointF(m_bottomRightProj.x() / kisimage->xRes(), m_bottomRightProj.y() / kisimage->yRes()));
		QPointF middleleft = converter.documentToView(QPointF(m_middleLeftProj.x() / kisimage->xRes(), m_middleLeftProj.y() / kisimage->yRes()));
		QPointF middleright = converter.documentToView(QPointF(m_middleRightProj.x() / kisimage->xRes(), m_middleRightProj.y() / kisimage->yRes()));
		QPointF middletop = converter.documentToView(QPointF(m_middleTopProj.x() / kisimage->xRes(), m_middleTopProj.y() / kisimage->yRes()));
		QPointF middlebottom = converter.documentToView(QPointF(m_middleBottomProj.x() / kisimage->xRes(), m_middleBottomProj.y() / kisimage->yRes()));
		QPointF origtopleft = converter.documentToView(QPointF(m_originalTopLeft.x() / kisimage->xRes(), m_originalTopLeft.y() / kisimage->yRes()));
		QPointF origbottomright = converter.documentToView(QPointF(m_originalBottomRight.x() / kisimage->xRes(), m_originalBottomRight.y() / kisimage->yRes()));

		QRectF handleRect(- m_handleRadius / 2., - m_handleRadius / 2., m_handleRadius, m_handleRadius);

		gc.setPen(pen);

		if (newRefSize != m_refSize) {
			//need to update m_scaledOrigSelectionImg and m_currentImg
			m_refSize = newRefSize;
			recalcOutline();
			m_scaledOrigSelectionImg = m_origSelectionImg->scaled(m_refSize.width() * m_origSelectionImg->width(), m_refSize.height() * m_origSelectionImg->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		if (m_optWidget && m_optWidget->hideDecorationsBox && m_optWidget->hideDecorationsBox->isChecked()) {
			if (m_imageTooBig)
				return;

			gc.setOpacity(0.6);
			QRectF bRect = boundRect(topleft, topright, bottomleft, bottomright);
			QPointF bRectCenter = bRect.center();
			gc.drawImage(QPointF(bRectCenter.x() - (double)m_currImg.width() / 2., bRectCenter.y() - (double)m_currImg.height() / 2.), m_currImg, QRectF(m_currImg.rect()));
		} else {
			gc.setOpacity(0.6);
			gc.drawImage(origtopleft, m_scaledOrigSelectionImg, QRectF(m_scaledOrigSelectionImg.rect()));

			if (m_imageTooBig)
				return;

			QRectF bRect = boundRect(topleft, topright, bottomleft, bottomright);
			QPointF bRectCenter = bRect.center();
			gc.drawImage(QPointF(bRectCenter.x() - (double)m_currImg.width() / 2., bRectCenter.y() - (double)m_currImg.height() / 2.), m_currImg, QRectF(m_currImg.rect()));
			gc.setOpacity(1.0);

			gc.drawRect(handleRect.translated(topleft));
			gc.drawRect(handleRect.translated(middletop));
			gc.drawRect(handleRect.translated(topright));
			gc.drawRect(handleRect.translated(middleright));
			gc.drawRect(handleRect.translated(bottomright));
			gc.drawRect(handleRect.translated(middlebottom));
			gc.drawRect(handleRect.translated(bottomleft));
			gc.drawRect(handleRect.translated(middleleft));

			gc.drawLine(topleft, topright);
			gc.drawLine(topright, bottomright);
			gc.drawLine(bottomright, bottomleft);
			gc.drawLine(bottomleft, topleft);

			QPointF rotationCenter = converter.documentToView(QPointF(m_rotationCenterProj.x() / kisimage->xRes(), m_rotationCenterProj.y() / kisimage->yRes()));
			QRectF rotationCenterRect(- m_rotationCenterRadius / 2., - m_rotationCenterRadius / 2., m_rotationCenterRadius, m_rotationCenterRadius);
			gc.drawEllipse(rotationCenterRect.translated(rotationCenter));
			gc.drawLine(QPointF(rotationCenter.x() - m_rotationCenterRadius / 2. - 2, rotationCenter.y()), QPointF(rotationCenter.x() + m_rotationCenterRadius / 2. + 2, rotationCenter.y()));
			gc.drawLine(QPointF(rotationCenter.x(), rotationCenter.y() - m_rotationCenterRadius / 2. - 2), QPointF(rotationCenter.x(), rotationCenter.y() + m_rotationCenterRadius / 2. + 2));
		}

		gc.setPen(old);
	} else if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
		QSizeF newRefSize = converter.documentToView(QSizeF(1 / kisimage->xRes(), 1 / kisimage->yRes()));
		QPointF topleft = converter.documentToView(QPointF(m_topLeftProj.x() / kisimage->xRes(), m_topLeftProj.y() / kisimage->yRes()));
		QPointF topright = converter.documentToView(QPointF(m_topRightProj.x() / kisimage->xRes(), m_topRightProj.y() / kisimage->yRes()));
		QPointF bottomleft = converter.documentToView(QPointF(m_bottomLeftProj.x() / kisimage->xRes(), m_bottomLeftProj.y() / kisimage->yRes()));
		QPointF bottomright = converter.documentToView(QPointF(m_bottomRightProj.x() / kisimage->xRes(), m_bottomRightProj.y() / kisimage->yRes()));
		QPointF origtopleft = converter.documentToView(QPointF(m_originalTopLeft.x() / kisimage->xRes(), m_originalTopLeft.y() / kisimage->yRes()));
		QPointF origbottomright = converter.documentToView(QPointF(m_originalBottomRight.x() / kisimage->xRes(), m_originalBottomRight.y() / kisimage->yRes()));

		QRectF handleRect(- m_handleRadius / 2., - m_handleRadius / 2., m_handleRadius, m_handleRadius);
		QRectF smallHandleRect(- m_handleRadius / 4., - m_handleRadius / 4., m_handleRadius / 2., m_handleRadius / 2.);

		gc.setPen(pen);

		if (newRefSize != m_refSize) {
			m_refSize = newRefSize;
			recalcOutline();
			m_scaledOrigSelectionImg = m_origSelectionImg->scaled(m_refSize.width() * m_origSelectionImg->width(), m_refSize.height() * m_origSelectionImg->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		if (m_optWidget && m_optWidget->hideDecorationsBox && !m_optWidget->hideDecorationsBox->isChecked()) {
			gc.setOpacity(0.6);
			gc.drawImage(origtopleft, m_scaledOrigSelectionImg, QRectF(m_scaledOrigSelectionImg.rect()));
		}

		gc.setOpacity(1.0);
		QPointF warptranslate = converter.documentToView(QPointF(m_currentArgs.previewPos().x() / kisimage->xRes(), m_currentArgs.previewPos().y() / kisimage->yRes()));
		gc.drawImage(QPointF(warptranslate), m_currImg, QRectF(m_currImg.rect()));

		int nbPoints = m_currentArgs.pointsPerLine() * m_currentArgs.pointsPerLine();
		for (int i = 0; i < nbPoints; ++i) {
			gc.drawEllipse(handleRect.translated(m_viewTransfPoints[i]));
		}
		gc.setBrush(Qt::SolidPattern);
		for (int i = 0; i < nbPoints; ++i) {
			gc.drawEllipse(smallHandleRect.translated(m_viewOrigPoints[i]));
		}
		pen.setStyle(Qt::DashLine);
		pen.setWidth(2);
		gc.setPen(pen);
		for (int i = 0; i < nbPoints; ++i) {
			gc.drawLine(m_viewTransfPoints[i], m_viewOrigPoints[i]);
		}
		
		gc.setBrush(Qt::NoBrush);
		gc.setPen(old);
	}
}

void KisToolTransform::transform()
{
    if (!image())
        return;

    setButtonBoxDisabled(false);
    TransformCmd *transaction = new TransformCmd(this, m_currentArgs, m_origSelection, m_originalTopLeft, m_originalBottomRight, m_origImg, m_origSelectionImg);

    if (image()->undoAdapter() != NULL)
        image()->undoAdapter()->addCommand(transaction);
}

void KisToolTransform::applyTransform()
{
    if (!image() || !currentNode()->paintDevice())
        return;

    KisCanvas2 *canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
    if (!canvas)
        return;

    QVector3D tmpCenter(m_originalCenter.x(), m_originalCenter.y(), 0);
    tmpCenter = scale(tmpCenter.x(), tmpCenter.y(), tmpCenter.z());
    tmpCenter = rotZ(tmpCenter.x(), tmpCenter.y(), tmpCenter.z());
    QPointF t = m_currentArgs.translate() - tmpCenter.toPointF();
    KoProgressUpdater* updater = canvas->view()->createProgressUpdater(KoProgressUpdater::Unthreaded);
    updater->start(100, i18n("Apply Transformation"));

    // This mementoes the current state of the active device.
    ApplyTransformCmd transaction(this, m_currentArgs.mode(), currentNode());

    ////Copy the original state back
    //QRect rc = m_origDevice->extent();
    //rc = rc.normalized();
    //currentNode()->paintDevice()->clear();
    //KisPainter gc(currentNode()->paintDevice());
    //gc.setCompositeOp(COMPOSITE_COPY);
    //gc.bitBlt(rc.topLeft(), m_origDevice, rc);
    //gc.end();

    // Also restore the original pixel selection (the shape selection will also be restored : see below)
    if (m_origSelection && !m_origSelection->isDeselected()) {
        if (currentSelection()) {
            //KoUpdaterPtr restorePixSelection = updater->startSubtask(5);
            //copy the pixel selection
            QRect rc = m_origSelection->selectedRect();
            rc = rc.normalized();
            currentSelection()->getOrCreatePixelSelection()->clear();
            KisPainter sgc(KisPaintDeviceSP(currentSelection()->getOrCreatePixelSelection()));
            sgc.setCompositeOp(COMPOSITE_COPY);
            //sgc.setProgress(restorePixSelection);
            sgc.bitBlt(rc.topLeft(), m_origSelection->getOrCreatePixelSelection(), rc);
            sgc.end();

            //if (m_origSelection->hasShapeSelection()) {
                //    KisShapeSelection* origShapeSelection = static_cast<KisShapeSelection*>(m_origSelection->shapeSelection());
                //    if (currentSelection()->hasShapeSelection()) {
                //        KisShapeSelection* currentShapeSelection = static_cast<KisShapeSelection*>(currentSelection()->shapeSelection());
                //        m_previousShapeSelection = static_cast<KisShapeSelection*>(currentShapeSelection->clone(currentSelection().data()));
                //    } else
                //        m_previousShapeSelection = static_cast<KisShapeSelection*>(origShapeSelection->clone(m_origSelection.data()));

                //    currentSelection()->setShapeSelection(origShapeSelection->clone(m_origSelection.data()));
                //}
        }
    } else if (currentSelection())
        currentSelection()->clear();

    // Perform the transform. Since we copied the original state back, this doesn't degrade
    // after many tweaks. Since we started the transaction before the copy back, the memento
    // has the previous state.
    if (m_origSelection) {
        KoUpdaterPtr copyPixels = updater->startSubtask(5);
        KoUpdaterPtr transformPixels = updater->startSubtask(10);
        KoUpdaterPtr copyBackPixels = updater->startSubtask(5);
        KoUpdaterPtr copyPixSelection = updater->startSubtask(5);
        KoUpdaterPtr transformPixSelection = updater->startSubtask(10);
        KoUpdaterPtr copyBackPixSelection = updater->startSubtask(5);
        KoUpdaterPtr perspectiveTransfPixels;
        KoUpdaterPtr perspectiveTransfPixSelection;
		if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
			perspectiveTransfPixels = updater->startSubtask(0);
			perspectiveTransfPixSelection = updater->startSubtask(0);
		} else {
			perspectiveTransfPixels = updater->startSubtask(10);
			perspectiveTransfPixSelection = updater->startSubtask(10);
		}

        //we copy the pixels of the selection into a tmpDevice before clearing them
        //we apply the transformation to the tmpDevice
        //and then we blit it into the currentNode's device
        KisPaintDeviceSP tmpDevice = new KisPaintDevice(currentNode()->paintDevice()->colorSpace());
        QRect selectRect = currentSelection()->selectedExactRect();
        KisPainter gc(tmpDevice, currentSelection());
        //gc.bitBlt(selectRect.topLeft(), m_origDevice, selectRect);
        gc.setProgress(copyPixels);
        gc.bitBlt(selectRect.topLeft(), currentNode()->paintDevice(), selectRect);
        gc.end();

		QRect tmpRc3 = boundRect(m_topLeftProj, m_topRightProj, m_bottomRightProj, m_bottomLeftProj).toRect();
		if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
			KisWarpTransformWorker worker(tmpDevice, m_currentArgs.pointsPerLine(), m_currentArgs.pointsPerLine(), m_currentArgs.origPoints(), m_currentArgs.transfPoints(), m_currentArgs.alpha(), transformPixels);
			worker.run();
		} else {
			KisTransformWorker worker(tmpDevice, m_currentArgs.scaleX(), m_currentArgs.scaleY(), m_currentArgs.shearX(), m_currentArgs.shearY(), m_originalCenter.x(), m_originalCenter.y(), m_currentArgs.aZ(), int(t.x()), int(t.y()), transformPixels, m_filter);
			worker.run();
			KisPerspectiveTransformWorker perspectiveWorker(tmpDevice, tmpRc3.united(tmpDevice->extent()), m_currentArgs.translate(), m_currentArgs.aX(), m_currentArgs.aY(), m_cameraPos.z(), perspectiveTransfPixels);
			perspectiveWorker.run();
		}

        currentNode()->paintDevice()->clearSelection(currentSelection());

        QRect tmpRc = tmpDevice->exactBounds();
        KisPainter painter(currentNode()->paintDevice());
        painter.setProgress(copyBackPixels);
        painter.bitBlt(tmpRc.topLeft(), tmpDevice, tmpRc);
        painter.end();

        //we do the same thing with the selection itself
        //we use a temporary device to apply the transformation
        //and then copy it back to the current device

        KisPixelSelectionSP pixelSelection = currentSelection()->getOrCreatePixelSelection();
        QRect pixelSelectRect = pixelSelection->selectedExactRect();

        KisPaintDeviceSP tmpDevice2 = new KisPaintDevice(pixelSelection->colorSpace());
        KisPainter gc2(tmpDevice2, currentSelection());
        gc2.setProgress(copyPixSelection);
        gc2.bitBlt(pixelSelectRect.topLeft(), pixelSelection, pixelSelectRect);
        gc2.end();

		if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
			KisWarpTransformWorker selectionWorker(tmpDevice2, m_currentArgs.pointsPerLine(), m_currentArgs.pointsPerLine(), m_currentArgs.origPoints(), m_currentArgs.transfPoints(), m_currentArgs.alpha(), transformPixSelection);
			selectionWorker.run();
		} else {
			KisTransformWorker selectionWorker(tmpDevice2, m_currentArgs.scaleX(), m_currentArgs.scaleY(), m_currentArgs.shearX(), m_currentArgs.shearY(), m_originalCenter.x(), m_originalCenter.y(), m_currentArgs.aZ(), (int)(t.x()), (int)(t.y()), transformPixSelection, m_filter);
			selectionWorker.run();
			KisPerspectiveTransformWorker perspectiveSelectionWorker(tmpDevice2, tmpRc3.united(tmpDevice2->extent()), m_currentArgs.translate(), m_currentArgs.aX(), m_currentArgs.aY(), m_cameraPos.z(), perspectiveTransfPixSelection);
			perspectiveSelectionWorker.run();
		}

        pixelSelection->clear();

        QRect tmpRc2 = tmpDevice2->exactBounds();
        KisPainter painter2(pixelSelection);
        painter2.setProgress(copyBackPixSelection);
        painter2.bitBlt(tmpRc2.topLeft(), tmpDevice2, tmpRc2);
        painter2.end();

        //Shape selection not transformed yet
        //if (m_origSelection->hasShapeSelection() && currentSelection()->hasShapeSelection() && m_previousShapeSelection) {
        //    KisShapeSelection* origShapeSelection = static_cast<KisShapeSelection*>(m_origSelection->shapeSelection());
        //    QList<KoShape *> origShapes = origShapeSelection->shapeManager()->shapes();
        //    KisShapeSelection* currentShapeSelection = static_cast<KisShapeSelection*>(currentSelection()->shapeSelection());
        //    QList<KoShape *> currentShapes = currentShapeSelection->shapeManager()->shapes();
        //    QList<KoShape *> previousShapes = m_previousShapeSelection->shapeManager()->shapes();
        //    QList<QTransform> m_oldMatrixList;
        //    QList<QTransform> m_newMatrixList;
        //    for (int i = 0; i < origShapes.size(); ++i) {
        //        KoShape *origShape = origShapes.at(i);
        //        QTransform origMatrix = origShape->transformation();
        //        QTransform previousMatrix = previousShapes.at(i)->transformation();

        //        QPointF center = origMatrix.map(QPointF(0.5 * origShape->size().width(), 0.5 * origShape->size().height()));
        //        QTransform rotateMatrix;
        //        rotateMatrix.translate(center.x(), center.y());
        //        rotateMatrix.rotate(180. * m_a / M_PI);
        //        rotateMatrix.translate(-center.x(), -center.y());

        //        m_oldMatrixList << previousMatrix;
        //        m_newMatrixList << origMatrix * rotateMatrix; // we apply the rotation on the original matrix
        //    }
        //    KoShapeTransformCommand* cmd = new KoShapeTransformCommand(currentShapes, m_oldMatrixList, m_newMatrixList, transaction.undoCommand());
        //    cmd->redo();
        //}
    } else {
		if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
			KoUpdaterPtr transformPixels = updater->startSubtask(40);
			KisWarpTransformWorker worker(currentNode()->paintDevice(), m_currentArgs.pointsPerLine(), m_currentArgs.pointsPerLine(), m_currentArgs.origPoints(), m_currentArgs.transfPoints(), m_currentArgs.alpha(), transformPixels);
			worker.run();
		} else {
			KoUpdaterPtr transformPixels = updater->startSubtask(40);
			KoUpdaterPtr perspectiveTransfPixels = updater->startSubtask(40);
			KisTransformWorker worker(currentNode()->paintDevice(), m_currentArgs.scaleX(), m_currentArgs.scaleY(), m_currentArgs.shearX(), m_currentArgs.shearY(), m_originalCenter.x(), m_originalCenter.y(), m_currentArgs.aZ(), int(t.x()), int(t.y()), transformPixels, m_filter);
			worker.run();
			QRect tmpRc3 = boundRect(m_topLeftProj, m_topRightProj, m_bottomRightProj, m_bottomLeftProj).toRect();
			KisPerspectiveTransformWorker perspectiveWorker(currentNode()->paintDevice(), tmpRc3.united(currentNode()->paintDevice()->extent()), m_currentArgs.translate(), m_currentArgs.aX(), m_currentArgs.aY(), m_cameraPos.z(), perspectiveTransfPixels);
			perspectiveWorker.run();
		}
    }

    //canvas update
    QRect previousRect = QRect( m_previousTopLeft, m_previousBottomRight );

    QRectF currRectF = boundRect(m_topLeftProj, m_topRightProj, m_bottomRightProj, m_bottomLeftProj);
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

    transaction.commit(image()->undoAdapter());
    updater->deleteLater();
}

void KisToolTransform::notifyCommandAdded(const QUndoCommand * command)
{
    const ApplyTransformCmdData * cmd1 = dynamic_cast<const ApplyTransformCmdData*>(command);
    const TransformCmd* cmd2 = dynamic_cast<const TransformCmd*>(command);

    if (cmd1 == 0 && cmd2 == 0) {
        // The last added command wasn't one of ours;
        // we should reset to the new state of the canvas.
        // In effect we should treat this as if the tool has been just activated
        initHandles(m_currentArgs.mode());
    }
}

void KisToolTransform::notifyCommandExecuted(const QUndoCommand * command)
{
    Q_UNUSED(command);
    const ApplyTransformCmdData * presentCmd1 = 0;
    const TransformCmd * presentCmd2 = 0;

    presentCmd1 = dynamic_cast<const ApplyTransformCmdData*>(image()->undoAdapter()->presentCommand());
    presentCmd2 = dynamic_cast<const TransformCmd*>(image()->undoAdapter()->presentCommand());

    if (presentCmd1 == 0 && presentCmd2 == 0) {
        updateCurrentOutline();
        // The command now on the top of the stack isn't one of ours
        // We should treat this as if the tool has been just activated
        initHandles(m_currentArgs.mode());

        setButtonBoxDisabled(true);
    } else {
        if (presentCmd1 != 0) {
            //we have undone a transformation just after an "apply transformation" : we just to to reinit the handles
            initHandles(presentCmd1->mode());

            setButtonBoxDisabled(true);
        } else {
            //the present command (on top of a stack) is a simple transform : we ask for its arguments
            presentCmd2->transformArgs(m_currentArgs);
            m_origSelection = presentCmd2->origSelection(m_originalTopLeft, m_originalBottomRight);
            presentCmd2->origPreviews(m_origImg, m_origSelectionImg);

            m_originalCenter = (m_originalTopLeft + m_originalBottomRight) / 2;
            m_originalHeight2 = m_originalCenter.y() - m_originalTopLeft.y();
            m_originalWidth2 = m_originalCenter.x() - m_originalTopLeft.x();
            m_previousTopLeft = m_originalTopLeft;
            m_previousBottomRight = m_originalBottomRight;
            m_scaleX_wOutModifier = m_currentArgs.scaleX();
            m_scaleY_wOutModifier = m_currentArgs.scaleY();
            m_refSize = QSizeF(0, 0); //will force the recalc of current QImages in recalcOutline

            setButtonBoxDisabled(false);
			if (m_optWidget && m_optWidget->warpButton) {
				if (m_currentArgs.mode() == ToolTransformArgs::WARP)
					m_optWidget->warpButton->setChecked(true);
				else
					m_optWidget->warpButton->setChecked(false);
			}
        }

        updateOutlineChanged();
        refreshSpinBoxes();
    }
}

void KisToolTransform::slotSetFilter(const KoID &filterID)
{
    m_filter = KisFilterStrategyRegistry::instance()->value(filterID.id());
}

QWidget* KisToolTransform::createOptionWidget() {
    m_optWidget = new WdgToolTransform(0);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setObjectName(toolId() + " option widget");

    m_optWidget->cmbFilter->clear();
    m_optWidget->cmbFilter->setIDList(KisFilterStrategyRegistry::instance()->listKeys());

    m_optWidget->cmbFilter->setCurrent("Bicubic");
    connect(m_optWidget->cmbFilter, SIGNAL(activated(const KoID &)),
            this, SLOT(slotSetFilter(const KoID &)));

    QPixmap rotateX_Pixmap, rotateY_Pixmap, rotateZ_Pixmap;
    rotateX_Pixmap.loadFromData(rotateX_PNG, rotateX_PNG_len, "png");
    rotateY_Pixmap.loadFromData(rotateY_PNG, rotateY_PNG_len, "png");
    rotateZ_Pixmap.loadFromData(rotateZ_PNG, rotateZ_PNG_len, "png");
    m_optWidget->label_rotateX->setPixmap(rotateX_Pixmap);
    m_optWidget->label_rotateY->setPixmap(rotateY_Pixmap);
    m_optWidget->label_rotateZ->setPixmap(rotateZ_Pixmap);

    KoID filterID = m_optWidget->cmbFilter->currentItem();
    m_filter = KisFilterStrategyRegistry::instance()->value(filterID.id());

    m_rotCenterButtons = new QButtonGroup(0);
    //we set the ids to match m_handleDir
    m_rotCenterButtons->addButton(m_optWidget->middleRightButton, 0);
    m_rotCenterButtons->addButton(m_optWidget->topRightButton, 1);
    m_rotCenterButtons->addButton(m_optWidget->middleTopButton, 2);
    m_rotCenterButtons->addButton(m_optWidget->topLeftButton, 3);
    m_rotCenterButtons->addButton(m_optWidget->middleLeftButton, 4);
    m_rotCenterButtons->addButton(m_optWidget->bottomLeftButton, 5);
    m_rotCenterButtons->addButton(m_optWidget->middleBottomButton, 6);
    m_rotCenterButtons->addButton(m_optWidget->bottomRightButton, 7);
    m_rotCenterButtons->addButton(m_optWidget->centerButton, 8);

    QToolButton *auxButton = new QToolButton(0);
    auxButton->setCheckable(true);
    auxButton->setAutoExclusive(true);
    auxButton->hide(); //a convenient button for when no button is checked in the group
    m_rotCenterButtons->addButton(auxButton, 9);

    connect(m_rotCenterButtons, SIGNAL(buttonPressed(int)), this, SLOT(setRotCenter(int)));
    connect(m_optWidget->scaleXBox, SIGNAL(valueChanged(double)), this, SLOT(setScaleX(double)));
    connect(m_optWidget->scaleYBox, SIGNAL(valueChanged(double)), this, SLOT(setScaleY(double)));
    connect(m_optWidget->shearXBox, SIGNAL(valueChanged(double)), this, SLOT(setShearX(double)));
    connect(m_optWidget->shearYBox, SIGNAL(valueChanged(double)), this, SLOT(setShearY(double)));
    connect(m_optWidget->translateXBox, SIGNAL(valueChanged(double)), this, SLOT(setTranslateX(double)));
    connect(m_optWidget->translateYBox, SIGNAL(valueChanged(double)), this, SLOT(setTranslateY(double)));
    connect(m_optWidget->aXBox, SIGNAL(valueChanged(double)), this, SLOT(setAX(double)));
    connect(m_optWidget->aYBox, SIGNAL(valueChanged(double)), this, SLOT(setAY(double)));
    connect(m_optWidget->aZBox, SIGNAL(valueChanged(double)), this, SLOT(setAZ(double)));
    connect(m_optWidget->aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(keepAspectRatioChanged(bool)));

    connect(m_optWidget->buttonBox, SIGNAL(clicked(QAbstractButton *)), this, SLOT(buttonBoxClicked(QAbstractButton *)));
    KisCanvas2 *canvas = dynamic_cast<KisCanvas2 *>(m_canvas);
    if (canvas)
        connect(m_optWidget->hideDecorationsBox, SIGNAL(stateChanged(int)), canvas, SLOT(updateCanvas()));
    setButtonBoxDisabled(true);

    connect(m_optWidget->scaleXBox, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_optWidget->scaleYBox, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_optWidget->shearXBox, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_optWidget->shearYBox, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_optWidget->translateXBox, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_optWidget->translateYBox, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_optWidget->aXBox, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_optWidget->aYBox, SIGNAL(editingFinished()), this, SLOT(editingFinished()));
    connect(m_optWidget->aZBox, SIGNAL(editingFinished()), this, SLOT(editingFinished()));

	connect(m_optWidget->warpButton, SIGNAL(toggled(bool)), this, SLOT(warpButtonToggled(bool)));

    refreshSpinBoxes();

    m_optWidget->tooBigLabel->hide();

    return m_optWidget;
}

QWidget* KisToolTransform::optionWidget()
{
    return m_optWidget;
}

void KisToolTransform::setRotCenter(int id)
{
    if (!m_selecting) {
        if (id < 9) {
            double i = m_handleDir[id].x();
            double j = m_handleDir[id].y();

            m_currentArgs.setRotationCenterOffset(QPointF(i * m_originalWidth2, j * m_originalHeight2));
            updateOutlineChanged();

            m_boxValueChanged = true;
        }
    }
}

void KisToolTransform::setScaleX(double scaleX)
{
    if (!m_selecting) {
        //the spinbox has been modified directly
        m_currentArgs.setScaleX(scaleX / 100.);

        if (m_optWidget->aspectButton->keepAspectRatio() && m_optWidget->scaleXBox->value() != m_optWidget->scaleYBox->value())
            m_optWidget->scaleYBox->setValue(m_optWidget->scaleXBox->value());

        updateOutlineChanged();

        m_boxValueChanged = true;
    } else {
        //the scale factor has been modified by mouse movement : we set the aspect ratio button manually
        if (m_currentArgs.scaleX() == m_currentArgs.scaleY())
            m_optWidget->aspectButton->setKeepAspectRatio(true);
        else
            m_optWidget->aspectButton->setKeepAspectRatio(false);
    }
}

void KisToolTransform::setScaleY(double scaleY)
{
    if (!m_selecting) {
        //the spinbox has been modified directly
        m_currentArgs.setScaleY(scaleY / 100.);

        if (m_optWidget->aspectButton->keepAspectRatio() && m_optWidget->scaleXBox->value() != m_optWidget->scaleYBox->value())
            m_optWidget->scaleXBox->setValue(m_optWidget->scaleYBox->value());

        updateOutlineChanged();

        m_boxValueChanged = true;
    } else {
        //the scale factor has been modified by mouse movement : we set the aspect ratio button manually
        if (m_currentArgs.scaleX() == m_currentArgs.scaleY())
            m_optWidget->aspectButton->setKeepAspectRatio(true);
        else
            m_optWidget->aspectButton->setKeepAspectRatio(false);
    }
}

void KisToolTransform::setShearX(double shearX)
{
    if (!m_selecting) {
        m_currentArgs.setShearX(shearX);
        updateOutlineChanged();

        m_boxValueChanged = true;
    }
}

void KisToolTransform::setShearY(double shearY)
{
    if (!m_selecting) {
        m_currentArgs.setShearY(shearY);
        updateOutlineChanged();

        m_boxValueChanged = true;
    }
}

void KisToolTransform::setAX(double aX)
{
    if (!m_selecting) {
        m_currentArgs.setAX(degreeToRadian(aX));
        updateOutlineChanged();

        m_boxValueChanged = true;
    }
}

void KisToolTransform::setAY(double aY)
{
    if (!m_selecting) {
        m_currentArgs.setAY(degreeToRadian(aY));
        updateOutlineChanged();

        m_boxValueChanged = true;
    }
}

void KisToolTransform::setAZ(double aZ)
{
    if (!m_selecting) {
        m_currentArgs.setAZ(degreeToRadian(aZ));
        updateOutlineChanged();

        m_boxValueChanged = true;
    }
}

void KisToolTransform::setTranslateX(double translateX)
{
    if (!m_selecting) {
        m_currentArgs.setTranslate(QPointF(translateX, m_currentArgs.translate().y()));
        updateOutlineChanged();

        m_boxValueChanged = true;
    }
}

void KisToolTransform::setTranslateY(double translateY)
{
    if (!m_selecting) {
        m_currentArgs.setTranslate(QPointF(m_currentArgs.translate().x(), translateY));
        updateOutlineChanged();

        m_boxValueChanged = true;
    }
}

void KisToolTransform::buttonBoxClicked(QAbstractButton *button)
{
    if (m_optWidget == 0 || m_optWidget->buttonBox == 0)
        return;

    QAbstractButton *applyButton = m_optWidget->buttonBox->button(QDialogButtonBox::Apply);
    QAbstractButton *resetButton = m_optWidget->buttonBox->button(QDialogButtonBox::Reset);

    if (button == applyButton) {
        QApplication::setOverrideCursor(KisCursor::waitCursor());
        applyTransform();
        initHandles(m_currentArgs.mode());
        QApplication::restoreOverrideCursor();

        setButtonBoxDisabled(true);
    } else if (button == resetButton) {
		if (m_currentArgs.mode() == ToolTransformArgs::FREE_TRANSFORM) {
			initFreeTransform();
		} else
			initWarpTransform();

		transform(); //commit the reset to the undo stack
        updateOutlineChanged();

        setButtonBoxDisabled(true);
    }
}

void KisToolTransform::keepAspectRatioChanged(bool keep)
{
    if (keep) {
        if (m_optWidget->scaleXBox->value() > m_optWidget->scaleYBox->value())
            m_optWidget->scaleYBox->setValue(m_optWidget->scaleXBox->value());
        else if (m_optWidget->scaleYBox->value() > m_optWidget->scaleXBox->value())
            m_optWidget->scaleXBox->setValue(m_optWidget->scaleYBox->value());
    }
}

void KisToolTransform::editingFinished()
{
    if (m_boxValueChanged) {
        updateCurrentOutline();

        transform();

        m_scaleX_wOutModifier = m_currentArgs.scaleX();
        m_scaleY_wOutModifier = m_currentArgs.scaleY();

        m_boxValueChanged = false;
    }
}

void KisToolTransform::warpButtonToggled(bool checked)
{
	if (checked)
		initHandles(ToolTransformArgs::WARP);
	else
		initHandles(ToolTransformArgs::FREE_TRANSFORM);

	recalcOutline();
	updateCurrentOutline();
}

#include "kis_tool_transform.moc"
