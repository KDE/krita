/*
 *  kis_tool_transform.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2010 Marc Pegon <pe.marc@free.fr>
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
#include "tool_transform_commands.h"


#include <math.h>
#include <limits>

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
#include <kis_system_locker.h>

#include <KoShapeTransformCommand.h>

#include "kis_canvas_resource_provider.h"
#include "widgets/kis_progress_widget.h"

KisToolTransform::KisToolTransform(KoCanvasBase * canvas)
        : KisTool(canvas, KisCursor::rotateCursor())
        , m_isActive(false)
{
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    Q_ASSERT(m_canvas);

    setObjectName("tool_transform");
    useCursor(KisCursor::selectCursor());
    m_eyePos = QVector3D(0, 0, -1024);
    m_cameraPos = QVector3D(0, 0, 1024);
    m_optWidget = 0;
    m_scaleCursors[0] = KisCursor::sizeHorCursor();
    m_scaleCursors[1] = KisCursor::sizeBDiagCursor();
    m_scaleCursors[2] = KisCursor::sizeVerCursor();
    m_scaleCursors[3] = KisCursor::sizeFDiagCursor();
    m_scaleCursors[4] = KisCursor::sizeHorCursor();
    m_scaleCursors[5] = KisCursor::sizeBDiagCursor();
    m_scaleCursors[6] = KisCursor::sizeVerCursor();
    m_scaleCursors[7] = KisCursor::sizeFDiagCursor();
    QPixmap shearPixmap;
    shearPixmap.load(KStandardDirs::locate("data", "calligra/icons/cursor_shear.png"));
    m_shearCursors[7] = QCursor(shearPixmap.transformed(QTransform().rotate(45)));
    m_shearCursors[6] = QCursor(shearPixmap.transformed(QTransform().rotate(90)));
    m_shearCursors[5] = QCursor(shearPixmap.transformed(QTransform().rotate(135)));
    m_shearCursors[4] = QCursor(shearPixmap.transformed(QTransform().rotate(180)));
    m_shearCursors[3] = QCursor(shearPixmap.transformed(QTransform().rotate(225)));
    m_shearCursors[2] = QCursor(shearPixmap.transformed(QTransform().rotate(270)));
    m_shearCursors[1] = QCursor(shearPixmap.transformed(QTransform().rotate(315)));
    m_shearCursors[0] = QCursor(shearPixmap);
    m_defaultPointsPerLine = 3;
    m_imageTooBig = false;
    m_origDevice = 0;
    m_origSelection = 0;
    m_handleRadius = 12;
    m_rotationCenterRadius = 12;
    m_maxRadius = (m_handleRadius > m_rotationCenterRadius) ? m_handleRadius : m_rotationCenterRadius;
}

KisToolTransform::~KisToolTransform()
{
    m_viewOrigPoints.clear();
    m_viewTransfPoints.clear();
}

QRectF KisToolTransform::boundRect(QPointF P0, QPointF P1, QPointF P2, QPointF P3)
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

QPointF KisToolTransform::minMaxZ(QVector3D P0, QVector3D P1, QVector3D P2, QVector3D P3)
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
    // M_PI / 8 to get the correct octant

    // we want an angle in [0; 2 * Pi[
    angle = fmod(angle, 2. * M_PI);
    if (angle < 0)
            angle += 2 * M_PI;

    int octant = (int)(angle * 4. / M_PI);

    return octant;
}

void KisToolTransform::storeArgs(ToolTransformArgs &args)
{
    args = m_currentArgs;
}

void KisToolTransform::restoreArgs(const ToolTransformArgs &args)
{
    m_currentArgs = args;
}

QRectF KisToolTransform::calcWarpBoundRect()
{
    int nbPoints = m_viewOrigPoints.size();
    QRectF res;

    if (nbPoints == 0) {
        res = m_transaction.originalRect();
    }
    else if (nbPoints == 1) {
        res = m_transaction.originalRect();
        res.translate(m_currentArgs.transfPoints()[0] - m_currentArgs.origPoints()[0]);
        res |= QRectF(m_currentArgs.origPoints()[0], QSizeF(1, 1));
    }
    else {
        res = QRectF(m_currentArgs.transfPoints()[0], m_currentArgs.transfPoints()[0]);

        for (int i = 0; i < nbPoints; ++i) {
            if ( m_currentArgs.transfPoints()[i].x() < res.left() )
                res.setLeft(m_currentArgs.transfPoints()[i].x());
            else if ( m_currentArgs.transfPoints()[i].x() > res.right() )
                res.setRight(m_currentArgs.transfPoints()[i].x());
            if ( m_currentArgs.transfPoints()[i].y() < res.top() )
                res.setTop(m_currentArgs.transfPoints()[i].y());
            else if ( m_currentArgs.transfPoints()[i].y() > res.bottom() )
                res.setBottom(m_currentArgs.transfPoints()[i].y());

            if ( m_currentArgs.origPoints()[i].x() < res.left() )
                res.setLeft(m_currentArgs.origPoints()[i].x());
            else if ( m_currentArgs.origPoints()[i].x() > res.right() )
                res.setRight(m_currentArgs.origPoints()[i].x());
            if ( m_currentArgs.origPoints()[i].y() < res.top() )
                res.setTop(m_currentArgs.origPoints()[i].y());
            else if ( m_currentArgs.origPoints()[i].y() > res.bottom() )
                res.setBottom(m_currentArgs.origPoints()[i].y());
        }

        if (!m_currentArgs.defaultPoints()) {
            QRectF r = m_transaction.originalRect();

            QPointF topLeft = KisWarpTransformWorker::transformMath(m_currentArgs.warpType(), r.topLeft(), m_currentArgs.origPoints(), m_currentArgs.transfPoints(), m_currentArgs.alpha());
            QPointF topRight = KisWarpTransformWorker::transformMath(m_currentArgs.warpType(), r.topRight(), m_currentArgs.origPoints(), m_currentArgs.transfPoints(), m_currentArgs.alpha());
            QPointF bottomRight = KisWarpTransformWorker::transformMath(m_currentArgs.warpType(), r.bottomRight(), m_currentArgs.origPoints(), m_currentArgs.transfPoints(), m_currentArgs.alpha());
            QPointF bottomLeft = KisWarpTransformWorker::transformMath(m_currentArgs.warpType(), r.bottomLeft(), m_currentArgs.origPoints(), m_currentArgs.transfPoints(), m_currentArgs.alpha());
            res |= boundRect(topLeft, topRight, bottomRight, bottomLeft);
        }
    }

    return res;
}

inline QPointF KisToolTransform::imageToThumb(const QPointF &pt, bool useFlakeOptimization)
{
    return useFlakeOptimization ? imageToFlake(pt) : m_thumbToImageTransform.inverted().map(pt);
}

void KisToolTransform::recalcOutline()
{
    QRectF scaleRect(0.0, 0.0, 1.0, 1.0);
    scaleRect = imageToFlake(scaleRect);


    if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
        QTransform scaleTransform;
        scaleTransform.scale(scaleRect.width(), scaleRect.height());

        if (m_viewOrigPoints.size() != m_currentArgs.origPoints().size()) {
            m_viewOrigPoints.resize(m_currentArgs.origPoints().size());
            m_viewTransfPoints.resize(m_currentArgs.transfPoints().size());
        }


        QTransform resultTransform = m_thumbToImageTransform * scaleTransform;
        qreal scaleX = resultTransform.m11();
        qreal scaleY = resultTransform.m22();
        bool useFlakeOptimization = scaleX < 1.0 && scaleY < 1.0;


        QVector<QPointF> thumbOrigPoints(m_viewOrigPoints.size());
        QVector<QPointF> thumbTransfPoints(m_viewOrigPoints.size());

        for (int i = 0; i < m_viewTransfPoints.size(); ++i) {
            m_viewTransfPoints[i] = imageToFlake(m_currentArgs.transfPoints()[i]);
            m_viewOrigPoints[i] = imageToFlake(m_currentArgs.origPoints()[i]);

            thumbOrigPoints[i] = imageToThumb(m_currentArgs.origPoints()[i], useFlakeOptimization);
            thumbTransfPoints[i] = imageToThumb(m_currentArgs.transfPoints()[i], useFlakeOptimization);
        }

        m_paintingOffset = m_transaction.originalTopLeft();

        if (!m_origImg.isNull() && !m_transaction.editWarpPoints()) {
            QPointF origTLInFlake = imageToThumb(m_transaction.originalTopLeft(), useFlakeOptimization);

            if (useFlakeOptimization) {
                m_currImg = m_origImg.transformed(m_thumbToImageTransform * scaleTransform);
                m_paintingTransform = QTransform();
            } else {
                m_currImg = m_origImg;
                m_paintingTransform = m_thumbToImageTransform * scaleTransform;

            }

            m_currImg = KisWarpTransformWorker::transformation(m_currentArgs.warpType(), &m_currImg, thumbOrigPoints, thumbTransfPoints, m_currentArgs.alpha(), origTLInFlake, &m_paintingOffset);
        } else {
            m_currImg = m_origImg;
            m_paintingOffset = m_transaction.originalTopLeft();
            m_paintingTransform = m_thumbToImageTransform * scaleTransform;
        }


        QRectF r(calcWarpBoundRect());
        m_topLeftProj = r.topLeft();
        m_topRightProj = r.topRight();
        m_bottomRightProj = r.bottomRight();
        m_bottomLeftProj = r.bottomLeft();
    }
    else {
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

        v = QVector3D(m_transaction.originalTopLeft() - m_transaction.originalCenter());
        t = transformVector(v);
        m_topLeft = t + translate3D;
        m_topLeftProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();
        t = QVector3D(perspective(t.x(), t.y(), t.z()));
        t = transformVector_preview(v, scaleRect.width(), scaleRect.height());
        prev_topLeft = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

        v = QVector3D(m_transaction.originalTopRight() - m_transaction.originalCenter());
        t = transformVector(v);
        m_topRight = t + translate3D;
        m_topRightProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();
        t = transformVector_preview(v, scaleRect.width(), scaleRect.height());
        prev_topRight = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

        v = QVector3D(m_transaction.originalBottomLeft() - m_transaction.originalCenter());
        t = transformVector(v);
        m_bottomLeft = t + translate3D;
        m_bottomLeftProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();
        t = transformVector_preview(v, scaleRect.width(), scaleRect.height());
        prev_bottomLeft = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

        v = QVector3D(m_transaction.originalBottomRight() - m_transaction.originalCenter());
        t = transformVector(v);
        m_bottomRight = t + translate3D;
        m_bottomRightProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();
        t = transformVector_preview(v, scaleRect.width(), scaleRect.height());
        prev_bottomRight = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

        v = QVector3D(m_transaction.originalMiddleLeft() - m_transaction.originalCenter());
        t = transformVector(v);
        m_middleLeft = t + translate3D;
        m_middleLeftProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

        v = QVector3D(m_transaction.originalMiddleRight() - m_transaction.originalCenter());
        t = transformVector(v);
        m_middleRight = t + translate3D;
        m_middleRightProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

        v = QVector3D(m_transaction.originalMiddleTop() - m_transaction.originalCenter());
        t = transformVector(v);
        m_middleTop = t + translate3D;
        m_middleTopProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

        v = QVector3D(m_transaction.originalMiddleBottom() - m_transaction.originalCenter());
        t = transformVector(v);
        m_middleBottom = t + translate3D;
        m_middleBottomProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

        v = QVector3D(m_currentArgs.rotationCenterOffset());
        t = transformVector(v);
        m_rotationCenter = t + translate3D;
        m_rotationCenterProj = perspective(t.x(), t.y(), t.z()) + m_currentArgs.translate();

        QVector3D v1 = m_topRight - m_topLeft;
        QVector3D v2 = m_bottomLeft - m_topLeft;
        m_currentPlane = QVector3D::crossProduct(v1, v2);
        m_currentPlane.normalize();

        // check whether image is too big to be displayed or not
        QPointF minmaxZ = minMaxZ(m_topLeft, m_topRight, m_bottomRight, m_bottomLeft);
        if (minmaxZ.y() >= m_cameraPos.z() * 0.9) {
            m_imageTooBig = true;
            if (m_optWidget && m_optWidget->tooBigLabelWidget)
                m_optWidget->tooBigLabelWidget->show();
            return;
        }
        else {
            m_imageTooBig = false;
            if (m_optWidget && m_optWidget->tooBigLabelWidget)
                m_optWidget->tooBigLabelWidget->hide();
        }


        QTransform TS = QTransform::fromTranslate(m_transaction.originalCenter().x(), m_transaction.originalCenter().y());
        QTransform S; S.shear(0, m_currentArgs.shearY()); S.shear(m_currentArgs.shearX(), 0);
        QTransform SC = QTransform::fromScale(m_currentArgs.scaleX(), m_currentArgs.scaleY());
        QTransform base = TS.inverted() * S * TS * SC;

        QPointF intermCenter = base.map(m_transaction.originalCenter());
        QTransform TR = QTransform::fromTranslate(intermCenter.x(), intermCenter.y());

        QMatrix4x4 m;
        m.rotate(180. * m_currentArgs.aX() / M_PI, QVector3D(1, 0, 0));
        m.rotate(180. * m_currentArgs.aY() / M_PI, QVector3D(0, 1, 0));
        m.rotate(180. * m_currentArgs.aZ() / M_PI, QVector3D(0, 0, 1));
        QTransform result = base * TR.inverted() * m.toTransform(m_cameraPos.z()) * TR;

        QPointF translation = m_currentArgs.translate() - result.map(m_transaction.originalCenter());
        QTransform T = QTransform::fromTranslate(translation.x(), translation.y());
        m_transform = result * T * QTransform::fromScale(scaleRect.width(), scaleRect.height());

        QTransform tl = QTransform::fromTranslate(m_transaction.originalTopLeft().x(), m_transaction.originalTopLeft().y());
        m_paintingTransform = tl.inverted() * m_thumbToImageTransform * tl * m_transform;
        m_paintingOffset = m_transaction.originalTopLeft();
    }
}

QPointF KisToolTransform::imageToFlake(const QPointF &pt)
{
    const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();
    return converter->imageToDocument(converter->documentToFlake(pt));
}

QPointF KisToolTransform::flakeToImage(const QPointF &pt)
{
    const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();
    return converter->flakeToDocument(converter->documentToImage(pt));
}

QRectF KisToolTransform::imageToFlake(const QRectF &rc)
{
    const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();
    return converter->imageToDocument(converter->documentToFlake(rc));
}

QRectF KisToolTransform::flakeToImage(const QRectF &rc)
{
    const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();
    return converter->flakeToDocument(converter->documentToImage(rc));
}

void KisToolTransform::outlineChanged()
{
    if (m_imageTooBig) {
        recalcOutline();
        m_canvas->updateCanvas();
        return;
    }

    KisImageSP kisimage = image();
    double maxRadiusX = m_canvas->viewConverter()->viewToDocumentX(m_maxRadius);
    double maxRadiusY = m_canvas->viewConverter()->viewToDocumentY(m_maxRadius);
    // get the smallest rectangle containing the previous frame (we need to use the 4 points because the rectangle
    // given by m_topLeft, .., m_bottomLeft can be rotated)
    QRectF oldRectF = boundRect(m_topLeftProj, m_topRightProj, m_bottomRightProj, m_bottomLeftProj);
    // we convert it to the right scale
    QRect oldRect = QRect( QPoint(oldRectF.left() / kisimage->xRes(), oldRectF.top() / kisimage->yRes()), QPoint(oldRectF.right() / kisimage->xRes(), oldRectF.bottom() / kisimage->yRes()) );

    recalcOutline(); // computes new m_topLeft, .., m_bottomLeft points
    QRectF newRectF = boundRect(m_topLeftProj, m_topRightProj, m_bottomRightProj, m_bottomLeftProj);
    QRect newRect = QRect( QPoint(newRectF.left() / kisimage->xRes(), newRectF.top() / kisimage->yRes()), QPoint(newRectF.right() / kisimage->xRes(), newRectF.bottom() / kisimage->yRes()) );

    // the rectangle to update is the union of the old rectangle et the new one
    newRect = oldRect.united(newRect);

    // we need to add adjust the rectangle because of the handles
    newRect.adjust(- maxRadiusX, - maxRadiusY, maxRadiusX, maxRadiusY);
    m_canvas->updateCanvas(newRect);
}

void KisToolTransform::paint(QPainter& gc, const KoViewConverter &converter)
{
    KisImageWSP kisimage = image();
    if (!kisimage) return;

    QPen oldPen = gc.pen();
    QPen pen[2];
    pen[1].setColor(Qt::lightGray);

    QSizeF newRefSize = imageToFlake(QRectF(0.0, 0.0, 1.0, 1.0)).size();

    if (newRefSize != m_refSize) {
        // need to update m_currentImg
        m_refSize = newRefSize;
        recalcOutline();
    }

    if (m_currentArgs.mode() == ToolTransformArgs::FREE_TRANSFORM) {
        pen[0].setWidth(1);
        pen[1].setWidth(2);

        QPointF topleft = imageToFlake(m_topLeftProj);
        QPointF topright = imageToFlake(m_topRightProj);
        QPointF bottomleft = imageToFlake(m_bottomLeftProj);
        QPointF bottomright = imageToFlake(m_bottomRightProj);
        QPointF middleleft = imageToFlake(m_middleLeftProj);
        QPointF middleright = imageToFlake(m_middleRightProj);
        QPointF middletop = imageToFlake(m_middleTopProj);
        QPointF middlebottom = imageToFlake(m_middleBottomProj);

        QRectF handleRect(- m_handleRadius / 2., - m_handleRadius / 2., m_handleRadius, m_handleRadius);

        gc.save();

        if (m_optWidget && m_optWidget->showDecorationsBox->isChecked()) {
            gc.setOpacity(0.3);
            gc.fillPath(m_selectionPath, Qt::black);
        }

        gc.setOpacity(0.9);
        gc.setTransform(m_paintingTransform, true);
        gc.drawImage(m_paintingOffset, m_origImg);

        gc.restore();

        for (int i = 1; i >= 0; --i) {
            gc.setPen(pen[i]);
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
        }

        QPointF rotationCenter = converter.documentToView(QPointF(m_rotationCenterProj.x() / kisimage->xRes(), m_rotationCenterProj.y() / kisimage->yRes()));
        QRectF rotationCenterRect(- m_rotationCenterRadius / 2., - m_rotationCenterRadius / 2., m_rotationCenterRadius, m_rotationCenterRadius);

        for (int i = 1; i >= 0; --i) {
            gc.setPen(pen[i]);
            gc.drawEllipse(rotationCenterRect.translated(rotationCenter));
            gc.drawLine(QPointF(rotationCenter.x() - m_rotationCenterRadius / 2. - 2, rotationCenter.y()), QPointF(rotationCenter.x() + m_rotationCenterRadius / 2. + 2, rotationCenter.y()));
            gc.drawLine(QPointF(rotationCenter.x(), rotationCenter.y() - m_rotationCenterRadius / 2. - 2), QPointF(rotationCenter.x(), rotationCenter.y() + m_rotationCenterRadius / 2. + 2));
        }


    }
    else if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
        pen[0].setWidth(2);
        pen[1].setWidth(3);

        QRectF handleRect(- m_handleRadius / 2., - m_handleRadius / 2., m_handleRadius, m_handleRadius);
        QRectF smallHandleRect(- m_handleRadius / 4., - m_handleRadius / 4., m_handleRadius / 2., m_handleRadius / 2.);

        gc.save();

        if (m_optWidget && m_optWidget->showDecorationsBox->isChecked()) {
            gc.setOpacity(0.3);
            gc.fillPath(m_selectionPath, Qt::black);
        }

        gc.setOpacity(1.0);

        gc.setTransform(m_paintingTransform, true);
        gc.drawImage(m_paintingOffset, m_currImg);

        gc.restore();

        for (int j = 1; j >= 0; --j) {
            gc.setPen(pen[j]);
            for (int i = 0; i < m_viewTransfPoints.size(); ++i) {
                gc.drawEllipse(handleRect.translated(m_viewTransfPoints[i]));
            }
        }
        gc.setPen(pen[1]);
        for (int i = 0; i < m_viewOrigPoints.size(); ++i) {
            gc.drawEllipse(smallHandleRect.translated(m_viewOrigPoints[i]));
        }
        gc.setPen(pen[0]);
        gc.setBrush(Qt::SolidPattern);
        for (int i = 0; i < m_viewOrigPoints.size(); ++i) {
            gc.drawEllipse(smallHandleRect.translated(m_viewOrigPoints[i]));
        }
        pen[1].setWidth(2);
        gc.setPen(pen[1]);
        for (int i = 0; i < m_viewOrigPoints.size(); ++i) {
            gc.drawLine(m_viewTransfPoints[i], m_viewOrigPoints[i]);
        }
        pen[0].setStyle(Qt::DashLine);
        gc.setPen(pen[0]);
        for (int i = 0; i < m_viewOrigPoints.size(); ++i) {
            gc.drawLine(m_viewTransfPoints[i], m_viewOrigPoints[i]);
        }
    }

    gc.setPen(oldPen);
}

void KisToolTransform::setFunctionalCursor()
{
    QPointF dir_vect;
    int rotOctant;

    if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
        if (m_cursorOverPoint)
            useCursor(KisCursor::pointingHandCursor());
        else
            useCursor(KisCursor::arrowCursor());
    }
    else {
        switch (m_function) {
        case MOVE:
            useCursor(KisCursor::moveCursor());
            break;
        case ROTATE:
            useCursor(KisCursor::rotateCursor());
            break;
        case PERSPECTIVE:
            //TODO: find another cursor for perspective
            useCursor(KisCursor::rotateCursor());
            break;
        case RIGHTSCALE:
            dir_vect = m_middleRightProj - m_currentArgs.translate();
            rotOctant = octant(dir_vect.x(), dir_vect.y());
            useCursor(m_scaleCursors[rotOctant]);
            break;
        case TOPSCALE:
            dir_vect = m_middleTopProj - m_currentArgs.translate();
            rotOctant = octant(dir_vect.x(), dir_vect.y());
            useCursor(m_scaleCursors[rotOctant]);
            break;
        case LEFTSCALE:
            dir_vect = m_middleLeftProj - m_currentArgs.translate();
            rotOctant = octant(dir_vect.x(), dir_vect.y());
            useCursor(m_scaleCursors[rotOctant]);
            break;
        case BOTTOMSCALE:
            dir_vect = m_middleBottomProj - m_currentArgs.translate();
            rotOctant = octant(dir_vect.x(), dir_vect.y());
            useCursor(m_scaleCursors[rotOctant]);
            break;
        case TOPRIGHTSCALE:
        case BOTTOMLEFTSCALE:
            useCursor(KisCursor::sizeBDiagCursor());
            break;
        case TOPLEFTSCALE:
        case BOTTOMRIGHTSCALE:
            useCursor(KisCursor::sizeFDiagCursor());
            break;
        case MOVECENTER:
            useCursor(KisCursor::handCursor());
            break;
        case BOTTOMSHEAR:
            dir_vect = m_bottomRightProj - m_bottomLeftProj;
            rotOctant = octant(dir_vect.x(), dir_vect.y());
            useCursor(m_shearCursors[rotOctant]);
            break;
        case RIGHTSHEAR:
            dir_vect = m_bottomRightProj - m_topRightProj;
            rotOctant = octant(dir_vect.x(), dir_vect.y());
            useCursor(m_shearCursors[rotOctant]);
            break;
        case TOPSHEAR:
            dir_vect = m_topRightProj - m_topLeftProj;
            rotOctant = octant(dir_vect.x(), dir_vect.y());
            useCursor(m_shearCursors[rotOctant]);
            break;
        case LEFTSHEAR:
            dir_vect = m_bottomLeftProj - m_topLeftProj;
            rotOctant = octant(dir_vect.x(), dir_vect.y());
            useCursor(m_shearCursors[rotOctant]);
            break;
        }
    }
}

void KisToolTransform::switchPoints(QPointF *p1, QPointF *p2)
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
        int nbPoints = m_viewOrigPoints.size();
        for (int i = 0; i < nbPoints; ++i) {
            if (distsq(mousePos, m_currentArgs.transfPoints()[i]) <= handleRadiusSq) {
                m_cursorOverPoint = true;
                m_pointUnderCursor = i;
                break;
            }
        }
    }
    else {
        if (modifiers & Qt::ControlModifier) {
            m_function = PERSPECTIVE;
            setFunctionalCursor();
            return;
        }

        QPointF topLeft, topRight, bottomLeft, bottomRight;
        QPointF tmp;

        // depending on the scale factor, we need to exchange left<->right and top<->bottom
        if (m_currentArgs.scaleX() > 0) {
            topLeft = m_topLeftProj;
            bottomLeft = m_bottomLeftProj;
            topRight = m_topRightProj;
            bottomRight = m_bottomRightProj;
        }
        else {
            topLeft = m_topRightProj;
            bottomLeft = m_bottomRightProj;
            topRight = m_topLeftProj;
            bottomRight = m_bottomLeftProj;
        }
        if (m_currentArgs.scaleY() < 0) {
            switchPoints(&topLeft, &bottomLeft);
            switchPoints(&topRight, &bottomRight);
        }

        // depending on the x & y rotations, we also need to exchange left<->right ant top<->bottom
        // first we constraint the angles in [0, 2*M_PI[
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
            QRectF originalRect = m_transaction.originalRect();

            // we check for shearing only if we aren't near a handle (for scaling) or the rotation center
            QVector3D v = QVector3D(mousePos - m_currentArgs.translate());
            v = invperspective(v.x(), v.y(), m_currentPlane);
            QPointF t = invTransformVector(v).toPointF();
            t += originalRect.center();

            if (t.x() >= originalRect.left() && t.x() <= originalRect.right()) {
                if (fabs(t.y() - originalRect.top()) <= handleRadius)
                    m_function = TOPSHEAR;
                if (fabs(t.y() - originalRect.bottom()) <= handleRadius)
                    m_function = BOTTOMSHEAR;
            }
            if (t.y() >= originalRect.top() && t.y() <= originalRect.bottom()) {
                if (fabs(t.x() - originalRect.left()) <= handleRadius)
                    m_function = LEFTSHEAR;
                if (fabs(t.x() - originalRect.right()) <= handleRadius)
                    m_function = RIGHTSHEAR;
            }
        }
    }

    setFunctionalCursor();
}

void KisToolTransform::mousePressEvent(KoPointerEvent *event)
{
    if (!PRESS_CONDITION_OM(event, KisTool::HOVER_MODE, Qt::LeftButton, Qt::ControlModifier)) {

        KisTool::mousePressEvent(event);
        return;
    }

    KisImageWSP kisimage = image();

    if (!currentNode() || !currentNode()->paintDevice())
        return;

    setMode(KisTool::PAINT_MODE);
    if (kisimage && currentNode()->paintDevice() && event->button() == Qt::LeftButton) {
        QPointF mousePos = QPointF(event->point.x() * kisimage->xRes(), event->point.y() * kisimage->yRes());
        if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
            if (!m_cursorOverPoint) {
                if (m_transaction.editWarpPoints()) {
                    QVector<QPointF> origPoints = m_currentArgs.origPoints();
                    QVector<QPointF> transfPoints = m_currentArgs.transfPoints();
                    origPoints.append(mousePos);
                    transfPoints.append(mousePos);
                    m_currentArgs.setPoints(origPoints, transfPoints);
                    m_viewTransfPoints.resize(origPoints.size());
                    m_viewOrigPoints.resize(origPoints.size());
                    outlineChanged();
                    m_cursorOverPoint = true;
                    m_pointUnderCursor = origPoints.size() - 1;
                    setFunctionalCursor();
                }
                else {
                    setMode(KisTool::HOVER_MODE);
                }
            }
        }
        else {
            if (m_function == ROTATE) {
                QVector3D clickoffset(mousePos - m_rotationCenterProj);
                clickoffset = invperspective(clickoffset.x(), clickoffset.y(), m_currentPlane);
                clickoffset = invrotX(clickoffset.x(), clickoffset.y(), clickoffset.z());
                clickoffset = invrotY(clickoffset.x(), clickoffset.y(), clickoffset.z());
                m_clickangle = atan2(-clickoffset.y(), clickoffset.x());
            }
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
    m_clickPlane = m_currentPlane;
    storeArgs(m_clickArgs);
}

void KisToolTransform::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Meta) {
        m_function = PERSPECTIVE;
        setFunctionalCursor();
    }

    KisTool::keyPressEvent(event);
}

void KisToolTransform::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        if (!nodeEditable()) {
            return;
        }

        QApplication::setOverrideCursor(KisCursor::waitCursor());
        applyTransform();
        initTransform(m_currentArgs.mode());
        QApplication::restoreOverrideCursor();
    }

    if (event->key() == Qt::Key_Meta) {
        setTransformFunction(m_prevMousePos, event->modifiers());

        if (mode() == KisTool::PAINT_MODE) {
            // if mode is HOVER_MODE the transformation has already
            // been comitted to the undo stack when mouse button was released
            if (m_imageTooBig) {
                restoreArgs(m_clickArgs);
                outlineChanged();
            }
            else {
                transform();
            }

            setMode(KisTool::HOVER_MODE);
        }
    }

    updateApplyResetAvailability();
    KisTool::keyReleaseEvent(event);
}

void KisToolTransform::resourceChanged(int key, const QVariant& res)
{
    KisTool::resourceChanged(key, res);
    if(m_isActive && key == KisCanvasResourceProvider::CurrentKritaNode) {
        initTransform(m_currentArgs.mode());
    }
}

/* A sort of gradient descent method is used to find the correct scale
   factors when scaling up/down the image from one of its corners,
   when there is a perspective projection.
   gradientDescent_f(scaleX, scaleY) is the function we want to make equal to zero.
 */
double KisToolTransform::gradientDescent_f(QVector3D v1, QVector3D v2, QVector3D desired, double scaleX, double scaleY) {
    v1 = QVector3D(v1.x() * scaleX, v1.y() * scaleY, v1.z());
    v1 = shear(v1.x(), v1.y(), v1.z());
    v1 = rotZ(v1.x(), v1.y(), v1.z());
    v1 = rotY(v1.x(), v1.y(), v1.z());
    v1 = rotX(v1.x(), v1.y(), v1.z());
    if (v1.z() > m_cameraPos.z()) {
        return std::numeric_limits<double>::max();
    }
    v1 = QVector3D(perspective(v1.x(), v1.y(), v1.z()));

    v2 = QVector3D(v2.x() * scaleX, v2.y() * scaleY, v2.z());
    v2 = shear(v2.x(), v2.y(), v2.z());
    v2 = rotZ(v2.x(), v2.y(), v2.z());
    v2 = rotY(v2.x(), v2.y(), v2.z());
    v2 = rotX(v2.x(), v2.y(), v2.z());
    if (v2.z() > m_cameraPos.z()) {
        return std::numeric_limits<double>::max();
    }
    v2 = QVector3D(perspective(v2.x(), v2.y(), v2.z()));

    QVector3D v(v2 - v1 - desired);

    return v.lengthSquared();
}

/* Approximation of the 1st partial derivative of f at point (scaleX, scaleY) */
double KisToolTransform::gradientDescent_partialDeriv1_f(QVector3D v1, QVector3D v2, QVector3D desired, double scaleX, double scaleY, double epsilon) {
    return (gradientDescent_f(v1,v2,desired,scaleX+epsilon,scaleY) - gradientDescent_f(v1,v2,desired,scaleX-epsilon,scaleY)) / (2 * epsilon);
}

/* Approximation of the 2nd partial derivative of f at point (scaleX, scaleY) */
double KisToolTransform::gradientDescent_partialDeriv2_f(QVector3D v1, QVector3D v2, QVector3D desired, double scaleX, double scaleY, double epsilon) {
    return (gradientDescent_f(v1,v2,desired,scaleX,scaleY+epsilon) - gradientDescent_f(v1,v2,desired,scaleX,scaleY-epsilon)) / (2 * epsilon);
}

/* The gradient descent
   When scaling up/down from the top right corner :
   v1 is the vector from center to top right corner
   v2 is the vector from center to bottom left corner
   Let T(v) be the vector obtained after transforming v1 (scale, shear, rotations, perspective)
   We Want T(v2)-T(v1) to match the vector "desired"
   x0 and y0 are the first scale factos to be tested
   epsilon is the precision we want to obtain normsquared(T(v2)-T(v1)-desired) <= epsilon
   gradStep, nbIt1, nbIt2 are technical parameters to adjust the algorithm
   epsilon_deriv is the precision for the approximation of the derivative of gradientDescent_f
   x_min, y_min are the obtained scale factors
   1 is returned if correct scale factors have been found, or else 0 (in that case x_min, y_min are unchanged
*/
int KisToolTransform::gradientDescent(QVector3D v1, QVector3D v2, QVector3D desired, double x0, double y0, double epsilon, double gradStep, int nbIt1, int nbIt2, double epsilon_deriv, double *x_min, double *y_min) {
   double val = gradientDescent_f(v1, v2, desired, x0, y0);
   double derivX, derivY;
   double x1, y1;
   int exit;
   double step;
   for (int i = 0; i < nbIt1 && val > epsilon; ++i) {
      step = gradStep;
      derivX = gradientDescent_partialDeriv1_f(v1, v2, desired, x0, y0, epsilon_deriv);
      derivY = gradientDescent_partialDeriv2_f(v1, v2, desired, x0, y0, epsilon_deriv);
      if (derivX == 0 && derivY == 0) {
          // might happen if f is not computable around x0, y0
          x0 /= 2;
          y0 /= 2;
          continue;
      }

      int j = 0;
      exit = 0;
      do {
         if (j > nbIt2) {
            exit = 1;
            break;
         }
         x1 = x0 - step * derivX;
         y1 = y0 - step * derivY;

         if (gradientDescent_f(v1, v2, desired, x1, y1) >= val) {
            step /= 2;
         }
         else {
            break;
         }
         ++j;
      } while(1);
      if (exit) {
         break;
      }
      else {
         x0 = x1;
         y0 = y1;
         val = gradientDescent_f(v1, v2, desired, x0, y0);
      }
   }

   if (val <= epsilon) {
      *x_min = x0;
      *y_min = y0;

      return 1;
   }
   else {
      return 0;
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
double KisToolTransform::dichotomyScaleX(QVector3D v1, QVector3D v2, DICHO_DROP flag, double desired, double b, double precision, double maxIterations1, double maxIterations2)
{
    double a = 0;
    int i = 0;
    double currentLength;
    double currentScaleX;
    bool b_found = false;
    QVector3D t1, t2;
    // first find b so that the length when currentScaleY = b is >= desired
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
            t1 = invperspective(t1.x(), t1.y(), m_clickPlane);
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
        }
        else {
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
                t1 = invperspective(t1.x(), t1.y(), m_clickPlane);
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
    }
    else
        return 1;
}

double KisToolTransform::dichotomyScaleY(QVector3D v1, QVector3D v2, DICHO_DROP flag, double desired, double b, double precision, double maxIterations1, double maxIterations2)
{
    double a = 0;
    int i = 0;
    double currentLength;
    double currentScaleY;
    bool b_found = false;
    QVector3D t1, t2;
    // first find b so that the length when currentScaleY = b is >= desired
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
            t1 = invperspective(t1.x(), t1.y(), m_clickPlane);
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
        }
        else {
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
                t1 = invperspective(t1.x(), t1.y(), m_clickPlane);
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
    }
    else
        return 1;
}

inline QPointF KisToolTransform::clipInRect(QPointF p, QRectF r)
{
    QPointF center = r.center();
    QPointF t = p - center;
    r.translate(- center);

    if (t.y() != 0) {
        if (t.x() != 0) {
            double slope = t.y() / t.x();

            if (t.x() < r.left()) {
                t.setY(r.left() * slope);
                t.setX(r.left());
            }
            else if (t.x() > r.right()) {
                t.setY(r.right() * slope);
                t.setX(r.right());
            }

            if (t.y() < r.top()) {
                t.setX(r.top() / slope);
                t.setY(r.top());
            }
            else if (t.y() > r.bottom()) {
                t.setX(r.bottom() / slope);
                t.setY(r.bottom());
            }
        }
        else {
            if (t.y() < r.top())
                t.setY(r.top());
            else if (t.y() > r.bottom())
                t.setY(r.bottom());
        }
    }
    else {
        if (t.x() < r.left())
            t.setX(r.left());
        else if (t.x() > r.right())
            t.setX(r.right());
    }

    t += center;

    return t;
}

void KisToolTransform::mouseMoveEvent(KoPointerEvent *event)
{
    KisImageWSP kisimage = image();
    QPointF mousePos = QPointF(event->point.x() * kisimage->xRes(), event->point.y() * kisimage->yRes());

    if (!MOVE_CONDITION(event, KisTool::PAINT_MODE)) {
        setTransformFunction(mousePos, event->modifiers());
        KisTool::mouseMoveEvent(event);
        return;
    }

    double dx, dy;

    m_actuallyMoveWhileSelected = true;

    if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
        QPointF *currPoint;
        QPointF *viewCurrPoint;
        if (m_transaction.editWarpPoints()) {
            currPoint = &m_currentArgs.origPoint(m_pointUnderCursor);
            viewCurrPoint = &m_viewOrigPoints[m_pointUnderCursor];
            *currPoint = clipInRect(mousePos, m_transaction.originalRect());
            m_currentArgs.transfPoint(m_pointUnderCursor) = *currPoint;
        }
        else {
            currPoint = &m_currentArgs.transfPoint(m_pointUnderCursor);
            viewCurrPoint = &m_viewTransfPoints[m_pointUnderCursor];
            *currPoint = mousePos;
        }
        *viewCurrPoint = imageToFlake(*currPoint);

        if (currPoint->x() < m_clickTopLeftProj.x()) {
            m_topLeft.setX(currPoint->x());
            m_bottomLeft.setX(currPoint->x());
        }
        else if (currPoint->x() > m_clickBottomRightProj.x()) {
            m_topRight.setX(currPoint->x());
            m_bottomRight.setX(currPoint->x());
        }

        if (currPoint->y() < m_clickTopLeftProj.y()) {
            m_topLeft.setY(currPoint->y());
            m_topRight.setY(currPoint->y());
        }
        else if (currPoint->y() > m_clickBottomRightProj.y()) {
            m_bottomLeft.setY(currPoint->y());
            m_bottomRight.setY(currPoint->y());
        }
    }
    else {
        int signY = 1, signX = 1;
        QVector3D t(0,0,0);
        QVector3D v1(0, 0, 0), v2(0, 0, 0), v3(0, 0, 0), v4(0, 0, 0);
        QVector3D v1Proj(0, 0, 0), newV1Proj(0, 0, 0);
        QVector3D v2Proj(0, 0, 0), v3Proj(0, 0, 0), v4Proj(0, 0, 0);

        switch (m_function) {
        case MOVE:
            t = QVector3D(mousePos - m_clickPoint);
            if (event->modifiers() & Qt::ShiftModifier) {
                if (event->modifiers() & Qt::ControlModifier) {
                    t = invperspective(t.x(), t.y(), m_currentPlane);
                    t = invTransformVector(t.x(), t.y(), t.z()); // go to local/object space
                    if (fabs(t.x()) >= fabs(t.y()))
                        t.setY(0);
                    else
                        t.setX(0);
                    t = transformVector(t.x(), t.y(), t.z()); // go back to global space
                    t = QVector3D(perspective(t.x(), t.y(), t.z()));
                }
                else {
                    if (fabs(t.x()) >= fabs(t.y()))
                        t.setY(0);
                    else
                        t.setX(0);
                }
            }

            m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

            updateOptionWidget();
            break;
        case ROTATE:
        {
            t = invperspective(mousePos.x() - m_clickRotationCenterProj.x(), mousePos.y() - m_clickRotationCenterProj.y(), m_clickPlane);
            t = invrotX(t.x(), t.y(), t.z());
            t = invrotY(t.x(), t.y(), t.z());
            double theta = m_clickangle - atan2(- t.y(), t.x());
            if (event->modifiers() & Qt::ShiftModifier) {
                int quotient = theta * 12 / M_PI;
                theta = quotient * M_PI / 12;
            }

            m_currentArgs.setAZ(m_clickArgs.aZ() + theta);

            // we want the rotation center projection to be unchanged after rotation
            // thus we calculate its new projection, and add the opposite translation to the current translation vector
            m_cosaZ = cos(m_currentArgs.aZ()); // update the cos/sin for transformation
            m_sinaZ = sin(m_currentArgs.aZ());
            QVector3D rotationCenterProj = transformVector(QVector3D(m_currentArgs.rotationCenterOffset()));
            rotationCenterProj = QVector3D(perspective(rotationCenterProj.x(), rotationCenterProj.y(), rotationCenterProj.z()) + m_clickArgs.translate());
            t= QVector3D(m_clickRotationCenterProj) - rotationCenterProj;

            m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

            updateOptionWidget();
        }
        break;
        case PERSPECTIVE:
        {
            t = QVector3D(mousePos.x() - m_clickPoint.x(), mousePos.y() - m_clickPoint.y(), 0);
            double thetaX = - t.y() * M_PI / m_transaction.originalHalfHeight() / 2 / fabs(m_currentArgs.scaleY());

            if (event->modifiers() & Qt::ShiftModifier) {
                int quotient = thetaX * 12 / M_PI;
                thetaX = quotient * M_PI / 12;
            }

            m_currentArgs.setAX(m_clickArgs.aX() + thetaX);
            m_cosaX = cos(m_currentArgs.aX()); // update the cos/sin for transformation
            m_sinaX = sin(m_currentArgs.aX());
            t = invrotX(t.x(), t.y(), t.z());
            double thetaY = t.x() * M_PI / m_transaction.originalHalfWidth() / 2 / fabs(m_currentArgs.scaleX());

            if (event->modifiers() & Qt::ShiftModifier) {
                int quotient = thetaY * 12 / M_PI;
                thetaY = quotient * M_PI / 12;
            }

            m_currentArgs.setAY(m_clickArgs.aY() + thetaY);


            // we want the rotation center projection to be unchanged after rotation
            // thus we calculate it's new projection, and add the opposite translation to the current translation vector
            m_cosaY = cos(m_currentArgs.aY());
            m_sinaY = sin(m_currentArgs.aY());
            QVector3D rotationCenterProj = transformVector(QVector3D(m_currentArgs.rotationCenterOffset()));
            rotationCenterProj = QVector3D(perspective(rotationCenterProj.x(), rotationCenterProj.y(), rotationCenterProj.z()) + m_clickArgs.translate());
            t= QVector3D(m_clickRotationCenterProj) - rotationCenterProj;

            m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

            updateOptionWidget();
        }
        break;
        case TOPSCALE:
        case BOTTOMSCALE:
            if (m_function == TOPSCALE) {
                signY = -1;
                // we want the result of v transformed to be equal to v1Proj (i.e. here we want the projection of the middle bottom point to be unchanged)
                v1 = QVector3D(m_transaction.originalMiddleBottom()) - QVector3D(m_transaction.originalCenter());
                v2 = QVector3D(m_transaction.originalMiddleTop()) - QVector3D(m_transaction.originalCenter());
                // we save the projection, and we'll adjust the translation at the end
                v1Proj = QVector3D(m_clickMiddleBottomProj);
            }
            else {
                signY = 1;

                v1 = QVector3D(m_transaction.originalMiddleTop()) - QVector3D(m_transaction.originalCenter());
                v2 = QVector3D(m_transaction.originalMiddleBottom()) - QVector3D(m_transaction.originalCenter());

                v1Proj = QVector3D(m_clickMiddleTopProj);
            }

            // first we need to get the scaleY factor
            // we cannot find it directly when there is perspective : we calculate the desired length, and search by dichotomy the corresponding scale factor
            if (m_clickArgs.aX() || m_clickArgs.aY()) {
                // there is a perspective projection
                // we have to find the scale factor the hard way

                t = QVector3D(mousePos) - v1Proj;

                double tmp = t.y();
                t = invperspective(t.x(), t.y(), m_clickPlane);
                if ( t.y() * tmp < 0) {
                    // the invert perspective changed the orientation
                    // we need to flip it
                    signY *= -1;
                }
                t = invrotX(t.x(), t.y(), t.z());
                t = invrotY(t.x(), t.y(), t.z());
                t = invrotZ(t.x(), t.y(), t.z());
                t = invshear(t.x(), t.y(), t.z());

                double b;
                if (signY * t.y() < 0)
                    b = - fabs(t.y() / m_transaction.originalHalfHeight() / 2.);
                else
                    b = fabs(t.y() / m_transaction.originalHalfHeight() / 2.);

                // we keep the vertical component only
                t = QVector3D(0, t.y(), 0);

                // and we transform the vector again : that way, we can get the desired length
                // (the distance between the middle top and middle bottom)
                t = shear(t.x(), t.y(), t.z());
                t = rotZ(t.x(), t.y(), t.z());
                t = rotY(t.x(), t.y(), t.z());
                t = rotX(t.x(), t.y(), t.z());
                t = QVector3D(perspective(t.x(), t.y(), t.z()));

                double desiredLength = t.length();
                m_scaleY_wOutModifier = dichotomyScaleY(v1, v2, NONE, desiredLength, b, 0.05, 64, 10);
            }
            else {
                // we invert the movement vector from the position of the decoration at click
                t = QVector3D(mousePos) - v1Proj;
                t = invperspective(t.x(), t.y(), m_clickPlane);
                t = invrotX(t.x(), t.y(), t.z());
                t = invrotY(t.x(), t.y(), t.z());
                t = invrotZ(t.x(), t.y(), t.z());
                t = invshear(t.x(), t.y(), t.z());

                t *= signY;

                m_scaleY_wOutModifier = t.y() / m_transaction.originalHalfHeight() / 2.;
            }

            // applies the shift modifier
            if (event->modifiers() & Qt::ShiftModifier) {
                double a_scaleY = fabs(m_scaleY_wOutModifier);

                m_currentArgs.setScaleX((m_scaleX_wOutModifier > 0) ? a_scaleY : -a_scaleY);
                m_currentArgs.setScaleY(m_scaleY_wOutModifier);
            }
            else
                m_currentArgs.setScaleY(m_scaleY_wOutModifier);

            newV1Proj = transformVector(v1);
            newV1Proj = QVector3D(perspective(newV1Proj.x(), newV1Proj.y(), newV1Proj.z()) + m_clickArgs.translate());
            t= v1Proj - newV1Proj;
            m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

            updateOptionWidget();
            break;
        case LEFTSCALE:
        case RIGHTSCALE:
            if (m_function == LEFTSCALE) {
                signX = -1;

                v1 = QVector3D(m_transaction.originalMiddleRight()) - QVector3D(m_transaction.originalCenter());
                v2 = QVector3D(m_transaction.originalMiddleLeft()) - QVector3D(m_transaction.originalCenter());

                v1Proj = QVector3D(m_clickMiddleRightProj);
            }
            else {
                signX = 1;

                v1 = QVector3D(m_transaction.originalMiddleLeft()) - QVector3D(m_transaction.originalCenter());
                v2 = QVector3D(m_transaction.originalMiddleRight()) - QVector3D(m_transaction.originalCenter());

                v1Proj = QVector3D(m_clickMiddleLeftProj);
            }

            if (m_clickArgs.aX() || m_clickArgs.aY()) {
                t = QVector3D(mousePos) - v1Proj;
                double tmp = t.x();
                t = invperspective(t.x(), t.y(), m_clickPlane);
                if ( t.x() * tmp < 0)
                    signX *= -1;
                t = invrotX(t.x(), t.y(), t.z());
                t = invrotY(t.x(), t.y(), t.z());
                t = invrotZ(t.x(), t.y(), t.z());
                t = invshear(t.x(), t.y(), t.z());

                double b;
                if (signX * t.x() < 0)
                    b = - fabs(t.x() / m_transaction.originalHalfWidth() / 2.);
                else
                    b = fabs(t.x() / m_transaction.originalHalfWidth() / 2.);

                t = QVector3D(t.x(), 0, 0);

                t = shear(t.x(), t.y(), t.z());
                t = rotZ(t.x(), t.y(), t.z());
                t = rotY(t.x(), t.y(), t.z());
                t = rotX(t.x(), t.y(), t.z());
                t = QVector3D(perspective(t.x(), t.y(), t.z()));

                double desiredLength = t.length();
                m_scaleX_wOutModifier = dichotomyScaleX(v1, v2, NONE, desiredLength, b, 0.05, 64, 10);
            }
            else {
                // we invert the movement vector from the position of the decoration at click
                t = QVector3D(mousePos) - v1Proj;
                t = invperspective(t.x(), t.y(), m_clickPlane);
                t = invrotX(t.x(), t.y(), t.z());
                t = invrotY(t.x(), t.y(), t.z());
                t = invrotZ(t.x(), t.y(), t.z());
                t = invshear(t.x(), t.y(), t.z());

                t *= signX;

                m_scaleX_wOutModifier = t.x() / m_transaction.originalHalfWidth() / 2.;
            }

            // applies the shift modifier
            if (event->modifiers() & Qt::ShiftModifier) {
                double a_scaleX = fabs(m_scaleX_wOutModifier);

                m_currentArgs.setScaleY((m_scaleY_wOutModifier > 0) ? a_scaleX : -a_scaleX);
                m_currentArgs.setScaleX(m_scaleX_wOutModifier);
            }
            else
                m_currentArgs.setScaleX(m_scaleX_wOutModifier);

            newV1Proj = transformVector(v1);
            newV1Proj = QVector3D(perspective(newV1Proj.x(), newV1Proj.y(), newV1Proj.z()) + m_clickArgs.translate());
            t= v1Proj - newV1Proj;
            m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

            updateOptionWidget();
            break;
        case TOPRIGHTSCALE:
        case BOTTOMRIGHTSCALE:
        case TOPLEFTSCALE:
        case BOTTOMLEFTSCALE:
            switch(m_function) {
            case TOPRIGHTSCALE:
                signY = -1;

                v1 = QVector3D(m_transaction.originalBottomLeft()) - QVector3D(m_transaction.originalCenter());
                v2 = QVector3D(m_transaction.originalTopRight()) - QVector3D(m_transaction.originalCenter());

                v1Proj = QVector3D(m_clickBottomLeftProj);
                break;
            case BOTTOMRIGHTSCALE:

                v1 = QVector3D(m_transaction.originalTopLeft()) - QVector3D(m_transaction.originalCenter());
                v2 = QVector3D(m_transaction.originalBottomRight()) - QVector3D(m_transaction.originalCenter());

                v1Proj = QVector3D(m_clickTopLeftProj);
                break;
            case TOPLEFTSCALE:
                signX = -1;
                signY = -1;

                v1 = QVector3D(m_transaction.originalBottomRight()) - QVector3D(m_transaction.originalCenter());
                v2 = QVector3D(m_transaction.originalTopLeft()) - QVector3D(m_transaction.originalCenter());

                v1Proj = QVector3D(m_clickBottomRightProj);
                break;
            case BOTTOMLEFTSCALE:
                signX = -1;

                v1 = QVector3D(m_transaction.originalTopRight()) - QVector3D(m_transaction.originalCenter());
                v2 = QVector3D(m_transaction.originalBottomLeft()) - QVector3D(m_transaction.originalCenter());

                v1Proj = QVector3D(m_clickTopRightProj);
                break;
            default:
                break;
            }

            // we invert the movement vector from the position of the decoration at click
            if (m_clickArgs.aX() || m_clickArgs.aY()) {
                t = QVector3D(mousePos) - v1Proj;

                QVector3D desired(t);

                t = invperspective(t.x(), t.y(), m_clickPlane);
                t = invrotX(t.x(), t.y(), t.z());
                t = invrotY(t.x(), t.y(), t.z());
                t = invrotZ(t.x(), t.y(), t.z());
                t = invshear(t.x(), t.y(), t.z());

                double initScaleX = signX * t.x() / m_transaction.originalHalfWidth() / 2.;
                double initScaleY = signY * t.y() / m_transaction.originalHalfHeight() / 2.;

                if (!gradientDescent(v1, v2, desired, initScaleX, initScaleY, 0.05, 1, 1000, 100, 0.001, &m_scaleX_wOutModifier, &m_scaleY_wOutModifier)) {
                    m_scaleX_wOutModifier = initScaleX;
                    m_scaleY_wOutModifier = initScaleY;
                }
            }
            else {
                t = QVector3D(mousePos) - v1Proj;
                t = invperspective(t.x(), t.y(), m_clickPlane);
                t = invrotX(t.x(), t.y(), t.z());
                t = invrotY(t.x(), t.y(), t.z());
                t = invrotZ(t.x(), t.y(), t.z());
                t = invshear(t.x(), t.y(), t.z());

                m_scaleX_wOutModifier = signX * t.x() / m_transaction.originalHalfWidth() / 2.;
                m_scaleY_wOutModifier = signY * t.y() / m_transaction.originalHalfHeight() / 2.;
            }

            // applies the shift modifier
            if (event->modifiers() & Qt::ShiftModifier) {
                double a_scaleX = fabs(m_scaleX_wOutModifier);
                double a_scaleY = fabs(m_scaleY_wOutModifier);

                if (a_scaleX > a_scaleY) {
                    m_currentArgs.setScaleY((m_scaleY_wOutModifier > 0) ? a_scaleX : -a_scaleX);
                    m_currentArgs.setScaleX(m_scaleX_wOutModifier);
                }
                else {
                    m_currentArgs.setScaleX((m_scaleX_wOutModifier > 0) ? a_scaleY : -a_scaleY);
                    m_currentArgs.setScaleY(m_scaleY_wOutModifier);
                }
            }
            else {
                m_currentArgs.setScaleX(m_scaleX_wOutModifier);
                m_currentArgs.setScaleY(m_scaleY_wOutModifier);
            }

            newV1Proj = transformVector(v1);
            newV1Proj = QVector3D(perspective(newV1Proj.x(), newV1Proj.y(), newV1Proj.z()) + m_clickArgs.translate());
            t= v1Proj - newV1Proj;
            m_currentArgs.setTranslate(m_clickArgs.translate() + t.toPointF());

            updateOptionWidget();
            break;
        case MOVECENTER:
            t = QVector3D(mousePos - m_currentArgs.translate());

            if (event->modifiers() & Qt::ShiftModifier) {
                if (event->modifiers() & Qt::ControlModifier) {
                    // we apply the constraint before going to local space
                    if (fabs(t.x()) >= fabs(t.y()))
                        t.setY(0);
                    else
                        t.setX(0);
                    t = invperspective(t.x(), t.y(), m_clickPlane);
                    t = invTransformVector(t.x(), t.y(), t.z()); // go to local space
                }
                else {
                    t = invperspective(t.x(), t.y(), m_clickPlane);
                    t = invTransformVector(t.x(), t.y(), t.z()); // go to local space
                    if (fabs(t.x()) >= fabs(t.y()))
                        t.setY(0);
                    else
                        t.setX(0);
                    // stay in local space because the center offset is taken in local space
                }
            }
            else {
                t = invperspective(t.x(), t.y(), m_clickPlane);
                t = invTransformVector(t.x(), t.y(), t.z());
            }

            // now we need to clip t in the rectangle of the tool
            t = QVector3D(clipInRect(t.toPointF(), m_transaction.originalRect().translated(-m_transaction.originalCenter())));

            m_currentArgs.setRotationCenterOffset(t.toPointF());

            m_optWidget->resetRotationCenterButtons();
            break;
        case TOPSHEAR:
            signX = -1;
        case BOTTOMSHEAR:
            t = QVector3D(mousePos - m_clickPoint);

            t = invperspective(t.x(), t.y(), m_clickPlane);
            t = invrotX(t.x(), t.y(), t.z());
            t = invrotY(t.x(), t.y(), t.z());
            t = invrotZ(t.x(), t.y(), t.z());
            // we do not use invShear because we want to use the clickArgs shear factors
            t.setY(t.y() - t.x() * m_clickArgs.shearY());
            t.setX(t.x() - t.y() * m_clickArgs.shearX());

            dx = signX * m_clickArgs.shearX() * m_currentArgs.scaleY() * m_transaction.originalHalfHeight(); // get the dx pixels corresponding to the current shearX factor
            dx += t.x(); // add the horizontal movement
            m_currentArgs.setShearX(signX * dx / m_currentArgs.scaleY() / m_transaction.originalHalfHeight()); // calculate the new shearX factor

            m_optWidget->shearXBox->setValue(m_currentArgs.shearX());
            break;
        case LEFTSHEAR:
            signY = -1;
        case RIGHTSHEAR:
            t = QVector3D(mousePos - m_clickPoint);
            t = invperspective(t.x(), t.y(), m_clickPlane);
            t = invrotX(t.x(), t.y(), t.z());
            t = invrotY(t.x(), t.y(), t.z());
            t = invrotZ(t.x(), t.y(), t.z());
            t.setY(t.y() - t.x() * m_clickArgs.shearY());
            t.setX(t.x() - t.y() * m_clickArgs.shearX());

            dy = signY *  m_clickArgs.shearY() * m_currentArgs.scaleX() * m_transaction.originalHalfWidth(); // get the dx pixels corresponding to the current shearX factor
            dy += t.y(); // add the horizontal movement
            m_currentArgs.setShearY(signY * dy / m_currentArgs.scaleX() / m_transaction.originalHalfWidth()); // calculate the new shearX factor

            m_optWidget->shearYBox->setValue(m_currentArgs.shearY());
            break;
        }
    }

    outlineChanged();
    m_prevMousePos = mousePos;
}

void KisToolTransform::mouseReleaseEvent(KoPointerEvent *event)
{
    if (!RELEASE_CONDITION(event, KisTool::PAINT_MODE, Qt::LeftButton)) {
        KisTool::mouseReleaseEvent(event);
        return;
    }

    setMode(KisTool::HOVER_MODE);

    if (m_actuallyMoveWhileSelected) {
        if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
            if (m_currentArgs.defaultPoints() || !m_transaction.editWarpPoints())
                transform();
            recalcOutline();
        }
        else {
            if (m_imageTooBig) {
                restoreArgs(m_clickArgs);
                outlineChanged();
            }
            else
                transform();

            m_scaleX_wOutModifier = m_currentArgs.scaleX();
            m_scaleY_wOutModifier = m_currentArgs.scaleY();
        }
    }
    updateApplyResetAvailability();
}

void KisToolTransform::initFreeTransform()
{
    // FIXME: probably, topleft should be aligned?
    m_currentArgs = ToolTransformArgs(ToolTransformArgs::FREE_TRANSFORM, m_transaction.originalCenter(), QPointF(),0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, KisWarpTransformWorker::RIGID_TRANSFORM, 1.0, true);
    m_scaleX_wOutModifier = m_currentArgs.scaleX();
    m_scaleY_wOutModifier = m_currentArgs.scaleY();

    m_refSize = QSizeF(0, 0);
}

//FIXME: remove this duplicated function
void KisToolTransform::setDefaultWarpPoints(int pointsPerLine)
{
    if (pointsPerLine < 0)
        pointsPerLine = m_defaultPointsPerLine;

    int nbPoints = pointsPerLine * pointsPerLine;
    QVector<QPointF> origPoints(nbPoints);
    QVector<QPointF> transfPoints(nbPoints);
    qreal gridSpaceX, gridSpaceY;

    if (nbPoints == 1) {
        //there is actually no grid
        origPoints[0] = m_transaction.originalCenter();
        transfPoints[0] = m_transaction.originalCenter();
    }
    else if (nbPoints > 1) {
        gridSpaceX = m_transaction.originalRect().width() / (pointsPerLine - 1);
        gridSpaceY = m_transaction.originalRect().height() / (pointsPerLine - 1);
        double y = m_transaction.originalRect().top();
        for (int i = 0; i < pointsPerLine; ++i) {
            double x = m_transaction.originalRect().left();
            for (int j = 0 ; j < pointsPerLine; ++j) {
                origPoints[i * pointsPerLine + j] = QPointF(x, y);
                transfPoints[i * pointsPerLine + j] = QPointF(x, y);
                x += gridSpaceX;
            }
            y += gridSpaceY;
        }
    }

    m_currentArgs.setDefaultPoints(true);
    m_currentArgs.setPoints(origPoints, transfPoints);
    m_currentArgs.setPointsPerLine(pointsPerLine);
}

void KisToolTransform::initWarpTransform()
{
    m_currentArgs = ToolTransformArgs(ToolTransformArgs::WARP, m_transaction.originalCenter(), QPointF(0, 0), 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, KisWarpTransformWorker::RIGID_TRANSFORM, 1.0, true);

    setDefaultWarpPoints();
    m_refSize = QSizeF(0, 0);
}

void KisToolTransform::updateSelectionPath()
{
    m_selectionPath = QPainterPath();

    QVector<QPolygon> selectionOutline;
    KisSelectionSP selection = currentSelection();

    if (selection) {
        selectionOutline = selection->outline();
    } else {
        KisPaintDeviceSP dev = currentNode()->paintDevice();
        selectionOutline << dev->exactBounds();
    }

    const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();
    QTransform i2f = converter->imageToDocumentTransform() * converter->documentToFlakeTransform();

    foreach(const QPolygon &polygon, selectionOutline) {
        QPolygon p = i2f.map(polygon);

        m_selectionPath.addPolygon(p);
    }
}

void KisToolTransform::initTransform(ToolTransformArgs::TransformMode mode)
{
    int x, y, w, h;

    if (!currentNode())
        return;

    KisPaintDeviceSP dev = currentNode()->paintDevice();

    KisSelectionSP selection = currentSelection();
    if (selection) {
        QRect r = selection->selectedExactRect();
        m_origSelection = new KisSelection();
        KisPixelSelectionSP origPixelSelection = new KisPixelSelection(*selection->getOrCreatePixelSelection().data());
        m_origSelection->setPixelSelection(origPixelSelection);
        r.getRect(&x, &y, &w, &h);
}
    else if (dev) {
        // we take all of the paintDevice
	QRect rc;
	rc = dev->exactBounds();
	x = rc.x();
	y = rc.y();
	w = rc.width();
	h = rc.height();
        m_origSelection = 0;
    }
    else {
        m_origSelection = 0;
        x = 0;
        y = 0;
        w = 0;
        h = 0;
    }

    m_transaction = TransformTransactionProperties(QRectF(x,y,w,h), &m_currentArgs);

    if (mode == ToolTransformArgs::WARP)
        initWarpTransform();
    else
        initFreeTransform();

    initThumbnailImage();
    updateSelectionPath();

    outlineChanged();
    updateOptionWidget();
    updateApplyResetAvailability();
}

void KisToolTransform::initThumbnailImage()
{
    m_transform = QTransform();
    m_origImg = QImage();
    m_currImg = QImage();

    KisPaintDeviceSP dev;
    KisSelectionSP selection = currentSelection();

    if (!currentNode() || !(dev = currentNode()->paintDevice())) {
        return;
    }

    KisPaintDeviceSP tmp = dev;

    QRect srcRect(m_transaction.originalRect().toAlignedRect());

    if (selection) {
        tmp = new KisPaintDevice(dev->colorSpace());
        KisPainter gc(tmp);
        gc.setSelection(selection);
        gc.bitBlt(srcRect.topLeft(), dev, srcRect);
    }

    const int maxSize = 2000;

    int x, y, w, h;
    srcRect.getRect(&x, &y, &w, &h);

    if (w > maxSize || h > maxSize) {
        qreal scale = qreal(maxSize) / (w > h ? w : h);
        QTransform scaleTransform = QTransform::fromScale(scale, scale);

        QRect thumbRect = scaleTransform.mapRect(m_transaction.originalRect()).toAlignedRect();

        m_origImg = tmp->createThumbnail(thumbRect.width(),
                                         thumbRect.height(),
                                         srcRect,
                                         KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation);
        m_thumbToImageTransform = scaleTransform.inverted();

    } else {
        m_origImg = tmp->convertToQImage(0, x, y, w, h, KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation);
        m_thumbToImageTransform = QTransform();
    }

    m_currImg = m_origImg; // create a shallow copy
}

void KisToolTransform::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    KisTool::activate(toolActivation, shapes);

    if (currentNode()) {
        image()->undoAdapter()->setCommandHistoryListener(this);

        const ApplyTransformCmdData * presentCmd1 = 0;
        const TransformCmd* presentCmd2 = 0;

        if (image()->undoAdapter()->presentCommand()) {
            presentCmd1 = dynamic_cast<const ApplyTransformCmdData*>(image()->undoAdapter()->presentCommand());
            presentCmd2 = dynamic_cast<const TransformCmd*>(image()->undoAdapter()->presentCommand());
        }

        if (presentCmd1 == 0 && presentCmd2 == 0) {
            initTransform(ToolTransformArgs::FREE_TRANSFORM);
        }
        else {
            // One of our commands is on top
            // We should ask for tool args and orig selection
            if (presentCmd1 != 0) {
                //we are just after an "apply" command
                //we just need to init the handles
                initTransform(presentCmd1->mode());
            }
            else {
                presentCmd2->transformArgs(m_currentArgs);

                int nbPoints = m_currentArgs.origPoints().size();
                m_viewOrigPoints.resize(nbPoints);
                m_viewTransfPoints.resize(nbPoints);

                QPoint p1, p2;
                m_origSelection = presentCmd2->origSelection(p1, p2);
                m_transaction = TransformTransactionProperties(QRect(p1, p2), &m_currentArgs);

                initThumbnailImage();
                updateSelectionPath();

                updateOptionWidget();
                updateApplyResetAvailability();
            }

            m_scaleX_wOutModifier = m_currentArgs.scaleX();
            m_scaleY_wOutModifier = m_currentArgs.scaleY();
            m_refSize = QSizeF(0, 0); //will force the recalc of image in recalcOutline

            outlineChanged();
            m_canvas->updateCanvas();
        }
    }
    else {
        updateOptionWidget();
    }
    m_isActive = true;
}

void KisToolTransform::deactivate()
{
    KisImageWSP kisimage = image();

    if (kisimage) {
        m_canvas->updateCanvas();
        kisimage->undoAdapter()->removeCommandHistoryListener(this);
    }

    KisTool::deactivate();
    m_isActive = false;
}

void KisToolTransform::transform()
{
    if (!image())
        return;

    TransformCmd *transaction = new TransformCmd(this, m_currentArgs, m_origSelection, m_transaction.originalTopLeftAligned(), m_transaction.originalBottomRightAligned());

    if (image()->undoAdapter() != NULL)
        image()->undoAdapter()->addCommand(transaction);
}

void KisToolTransform::transformDevice(KisPaintDeviceSP device,
                                       KoUpdaterPtr warpUpdater,
                                       KoUpdaterPtr affineUpdater,
                                       KoUpdaterPtr perspectiveUpdater)
{
    if (m_currentArgs.mode() == ToolTransformArgs::WARP) {
        KisWarpTransformWorker selectionWorker(m_currentArgs.warpType(),
                                               device,
                                               m_currentArgs.origPoints(),
                                               m_currentArgs.transfPoints(),
                                               m_currentArgs.alpha(),
                                               warpUpdater);
        selectionWorker.run();
    } else {
        QVector3D transformedCenter(m_transaction.originalCenter());
        transformedCenter = scale(transformedCenter.x(), transformedCenter.y(), transformedCenter.z());
        transformedCenter = rotZ(transformedCenter.x(), transformedCenter.y(), transformedCenter.z());
        QPointF translation = m_currentArgs.translate() - transformedCenter.toPointF();

        KisTransformWorker selectionWorker(device,
                                           m_currentArgs.scaleX(),
                                           m_currentArgs.scaleY(),
                                           m_currentArgs.shearX(),
                                           m_currentArgs.shearY(),
                                           m_transaction.originalCenter().x(),
                                           m_transaction.originalCenter().y(),
                                           m_currentArgs.aZ(),
                                           (int)(translation.x()),
                                           (int)(translation.y()),
                                           affineUpdater,
                                           m_currentArgs.filter());
        selectionWorker.run();

        KisPerspectiveTransformWorker perspectiveSelectionWorker(device,
                                                                 m_currentArgs.translate(),
                                                                 m_currentArgs.aX(),
                                                                 m_currentArgs.aY(),
                                                                 m_cameraPos.z(),
                                                                 perspectiveUpdater);
        perspectiveSelectionWorker.run();
    }
}

void KisToolTransform::applyTransform()
{
    if (!image() || !currentNode()->paintDevice() || currentNode()->systemLocked())
        return;

    if (!nodeEditable()) {
        return;
    }

    KisSystemLocker locker(currentNode());

    KoProgressUpdater* updater = m_canvas->view()->createProgressUpdater(KoProgressUpdater::Unthreaded);
    updater->start(100, i18n("Apply Transformation"));

    KisUndoAdapter *undoAdapter = image()->undoAdapter();
    undoAdapter->beginMacro(i18n("Apply transformation"));

    // This mementoes the current state of the active device.
    ApplyTransformCmd transaction(this, m_currentArgs.mode(), currentNode());

    // Also restore the original pixel selection (the shape selection will also be restored : see below)
    if (m_origSelection) {
        if (currentSelection()) {
            // copy the pixel selection
            QRect rc = m_origSelection->selectedRect();
            rc = rc.normalized();
            currentSelection()->getOrCreatePixelSelection()->clear();
            KisPainter sgc(KisPaintDeviceSP(currentSelection()->getOrCreatePixelSelection()));
            sgc.setCompositeOp(COMPOSITE_COPY);
            sgc.bitBlt(rc.topLeft(), m_origSelection->getOrCreatePixelSelection(), rc);
            sgc.end();

        }
    }
    else if (currentSelection())
        currentSelection()->clear();

    // Perform the transform. Since we copied the original state back, this doesn't degrade
    // after many tweaks. Since we started the transaction before the copy back, the memento
    // has the previous state.
    if (!currentNode()->inherits("KisMask") && m_origSelection) {
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
        }
        else {
            perspectiveTransfPixels = updater->startSubtask(10);
            perspectiveTransfPixSelection = updater->startSubtask(10);
        }

        // we copy the pixels of the selection into a tmpDevice before clearing them
        // we apply the transformation to the tmpDevice
        // and then we blit it into the currentNode's device
        KisPaintDeviceSP tmpDevice = new KisPaintDevice(currentNode()->paintDevice()->colorSpace());
        QRect selectRect = currentSelection()->selectedExactRect();
        KisPainter gc(tmpDevice, currentSelection());
        gc.setProgress(copyPixels);
        gc.bitBlt(selectRect.topLeft(), currentNode()->paintDevice(), selectRect);
        gc.end();

        transformDevice(tmpDevice, transformPixels, transformPixels, perspectiveTransfPixels);

        currentNode()->paintDevice()->clearSelection(currentSelection());

        QRect tmpRc = tmpDevice->extent();
        KisPainter painter(currentNode()->paintDevice());
        painter.setProgress(copyBackPixels);
        painter.bitBlt(tmpRc.topLeft(), tmpDevice, tmpRc);
        painter.end();

        // we do the same thing with the selection itself
        // we use a temporary device to apply the transformation
        // and then copy it back to the current device

        KisPixelSelectionSP pixelSelection = currentSelection()->getOrCreatePixelSelection();
        QRect pixelSelectRect = pixelSelection->selectedExactRect();

        KisPaintDeviceSP tmpDevice2 = new KisPaintDevice(pixelSelection->colorSpace());
        KisPainter gc2(tmpDevice2, currentSelection());
        gc2.setProgress(copyPixSelection);
        gc2.bitBlt(pixelSelectRect.topLeft(), pixelSelection, pixelSelectRect);
        gc2.end();

        transformDevice(tmpDevice2, transformPixSelection, transformPixSelection, perspectiveTransfPixSelection);

        pixelSelection->clear();

        QRect tmpRc2 = tmpDevice2->exactBounds();
        KisPainter painter2(pixelSelection);
        painter2.setProgress(copyBackPixSelection);
        painter2.bitBlt(tmpRc2.topLeft(), tmpDevice2, tmpRc2);
        painter2.end();

    }
    else {
        transformDevice(currentNode()->paintDevice(), updater->startSubtask(40), updater->startSubtask(40), updater->startSubtask(40));
    }

    transaction.commit(undoAdapter);
    undoAdapter->endMacro();

    updater->deleteLater();
    currentNode()->setDirty();

    m_canvas->view()->selectionManager()->selectionChanged();

    if (currentSelection() && currentSelection()->hasShapeSelection())
        m_canvas->view()->selectionManager()->shapeSelectionChanged();
}

void KisToolTransform::notifyCommandAdded(const KUndo2Command * command)
{
    const ApplyTransformCmdData * cmd1 = dynamic_cast<const ApplyTransformCmdData*>(command);
    const TransformCmd* cmd2 = dynamic_cast<const TransformCmd*>(command);

    if (currentNode()) {
        if (cmd1 == 0 && cmd2 == 0) {
            // The last added command wasn't one of ours;
            // we should reset to the new state of the canvas.
            // In effect we should treat this as if the tool has been just activated
            initTransform(m_currentArgs.mode());
        }
    }
    else {
        updateOptionWidget();
    }
}

void KisToolTransform::notifyCommandExecuted(const KUndo2Command * command)
{
    Q_UNUSED(command);
    const ApplyTransformCmdData * presentCmd1 = 0;
    const TransformCmd * presentCmd2 = 0;

    presentCmd1 = dynamic_cast<const ApplyTransformCmdData*>(image()->undoAdapter()->presentCommand());
    presentCmd2 = dynamic_cast<const TransformCmd*>(image()->undoAdapter()->presentCommand());

    if (currentNode()) {
        if (presentCmd1 == 0 && presentCmd2 == 0) {
            // The command now on the top of the stack isn't one of ours
            // We should treat this as if the tool has been just activated
            initTransform(m_currentArgs.mode());

            outlineChanged();
        }
        else {
            if (presentCmd1 != 0) {
                // we have undone a transformation just after an "apply transformation" : we just to to reinit the handles
                initTransform(presentCmd1->mode());
            }
            else {
                // the present command (on top of a stack) is a simple transform : we ask for its arguments

                presentCmd2->transformArgs(m_currentArgs);

                int nbPoints = m_currentArgs.origPoints().size();
                m_viewOrigPoints.resize(nbPoints);
                m_viewTransfPoints.resize(nbPoints);

                QPoint p1, p2;
                m_origSelection = presentCmd2->origSelection(p1, p2);
                m_transaction = TransformTransactionProperties(QRect(p1, p2), &m_currentArgs);

                initThumbnailImage();
                updateSelectionPath();

                m_scaleX_wOutModifier = m_currentArgs.scaleX();
                m_scaleY_wOutModifier = m_currentArgs.scaleY();
                m_refSize = QSizeF(0, 0); // will force the recalc of current QImages in recalcOutline

                updateOptionWidget();
                updateApplyResetAvailability();
            }

            outlineChanged();
        }
    }
    else {
        updateOptionWidget();
    }
}

QWidget* KisToolTransform::createOptionWidget() {
    m_optWidget = new KisToolTransformConfigWidget(&m_transaction, m_canvas, 0);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setObjectName(toolId() + " option widget");

    connect(m_optWidget, SIGNAL(sigConfigChanged()),
            this, SLOT(slotUiChangedConfig()));

    connect(m_optWidget, SIGNAL(sigApplyTransform()),
            this, SLOT(slotApplyTransform()));

    connect(m_optWidget, SIGNAL(sigResetTransform()),
            this, SLOT(slotResetTransform()));

    connect(m_optWidget, SIGNAL(sigEditingFinished()),
            this, SLOT(slotEditingFinished()));

    updateOptionWidget();

    return m_optWidget;
}

void KisToolTransform::updateOptionWidget()
{
    if (!m_optWidget) return;

    if (!currentNode()) {
        m_optWidget->setEnabled(false);
        return;
    }
    else {
        m_optWidget->setEnabled(true);
        m_optWidget->updateConfig(m_currentArgs);
    }
}

void KisToolTransform::updateApplyResetAvailability()
{
    if (m_optWidget) {
        m_optWidget->setApplyResetDisabled(m_currentArgs.isIdentity(m_transaction.originalCenter()));
    }
}

void KisToolTransform::slotUiChangedConfig()
{
    if (mode() == KisTool::PAINT_MODE) return;

    outlineChanged();
    updateApplyResetAvailability();
}

void KisToolTransform::slotApplyTransform()
{
    if (!nodeEditable()) return;

    QApplication::setOverrideCursor(KisCursor::waitCursor());
    applyTransform();
    initTransform(m_currentArgs.mode());
    QApplication::restoreOverrideCursor();
}

void KisToolTransform::slotResetTransform()
{
    initTransform(m_currentArgs.mode());
    slotEditingFinished();
}

void KisToolTransform::slotEditingFinished()
{
    transform();

    m_scaleX_wOutModifier = m_currentArgs.scaleX();
    m_scaleY_wOutModifier = m_currentArgs.scaleY();
}


#include "kis_tool_transform.moc"
