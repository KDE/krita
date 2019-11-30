/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_warp_transform_strategy.h"

#include <algorithm>

#include <QPointF>
#include <QPainter>

#include "kis_coordinates_converter.h"
#include "tool_transform_args.h"
#include "transform_transaction_properties.h"
#include "kis_painting_tweaks.h"
#include "kis_cursor.h"
#include "kis_transform_utils.h"
#include "kis_algebra_2d.h"
#include "KisHandlePainterHelper.h"
#include "kis_signal_compressor.h"



struct KisWarpTransformStrategy::Private
{
    Private(KisWarpTransformStrategy *_q,
            const KisCoordinatesConverter *_converter,
            ToolTransformArgs &_currentArgs,
            TransformTransactionProperties &_transaction)
        : q(_q),
          converter(_converter),
          currentArgs(_currentArgs),
          transaction(_transaction),
          lastNumPoints(0),
          drawConnectionLines(false), // useful while developing
          drawOrigPoints(false),
          drawTransfPoints(true),
          closeOnStartPointClick(false),
          clipOriginalPointsPosition(true),
          pointWasDragged(false),
          recalculateSignalCompressor(40, KisSignalCompressor::FIRST_ACTIVE)
    {
    }

    KisWarpTransformStrategy * const q;

    /// standard members ///

    const KisCoordinatesConverter *converter;

    //////
    ToolTransformArgs &currentArgs;
    //////
    TransformTransactionProperties &transaction;

    QTransform paintingTransform;
    QPointF paintingOffset;

    QTransform handlesTransform;

    /// custom members ///

    QImage transformedImage;

    int pointIndexUnderCursor;

    enum Mode {
        OVER_POINT = 0,
        MULTIPLE_POINT_SELECTION,
        MOVE_MODE,
        ROTATE_MODE,
        SCALE_MODE,
        NOTHING
    };
    Mode mode;

    QVector<int> pointsInAction;
    int lastNumPoints;

    bool drawConnectionLines;
    bool drawOrigPoints;
    bool drawTransfPoints;
    bool closeOnStartPointClick;
    bool clipOriginalPointsPosition;
    QPointF pointPosOnClick;
    bool pointWasDragged;

    QPointF lastMousePos;

    // cage transform also uses this logic. This helps this class know what transform type we are using
    TransformType transformType = TransformType::WARP_TRANSFORM;
    KisSignalCompressor recalculateSignalCompressor;

    void recalculateTransformations();
    inline QPointF imageToThumb(const QPointF &pt, bool useFlakeOptimization);

    bool shouldCloseTheCage() const;
    QVector<QPointF*> getSelectedPoints(QPointF *center, bool limitToSelectedOnly = false) const;
};

KisWarpTransformStrategy::KisWarpTransformStrategy(const KisCoordinatesConverter *converter,
                                                   ToolTransformArgs &currentArgs,
                                                   TransformTransactionProperties &transaction)
    : KisSimplifiedActionPolicyStrategy(converter),
      m_d(new Private(this, converter, currentArgs, transaction))
{
    connect(&m_d->recalculateSignalCompressor, SIGNAL(timeout()),
            SLOT(recalculateTransformations()));
}

KisWarpTransformStrategy::~KisWarpTransformStrategy()
{
}

void KisWarpTransformStrategy::setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive)
{
    double handleRadius = KisTransformUtils::effectiveHandleGrabRadius(m_d->converter);

    bool cursorOverPoint = false;
    m_d->pointIndexUnderCursor = -1;

    KisTransformUtils::HandleChooser<Private::Mode>
        handleChooser(mousePos, Private::NOTHING);

    const QVector<QPointF> &points = m_d->currentArgs.transfPoints();
    for (int i = 0; i < points.size(); ++i) {
        if (handleChooser.addFunction(points[i],
                                      handleRadius, Private::NOTHING)) {

            cursorOverPoint = true;
            m_d->pointIndexUnderCursor = i;
        }
    }

    if (cursorOverPoint) {
        m_d->mode = perspectiveModifierActive &&
            !m_d->currentArgs.isEditingTransformPoints() ?
            Private::MULTIPLE_POINT_SELECTION : Private::OVER_POINT;

    } else if (!m_d->currentArgs.isEditingTransformPoints()) {
        QPolygonF polygon(m_d->currentArgs.transfPoints());
        bool insidePolygon = polygon.boundingRect().contains(mousePos);
        m_d->mode = insidePolygon ? Private::MOVE_MODE :
            !perspectiveModifierActive ? Private::ROTATE_MODE :
            Private::SCALE_MODE;
    } else {
        m_d->mode = Private::NOTHING;
    }
}

QCursor KisWarpTransformStrategy::getCurrentCursor() const
{
    QCursor cursor;

    switch (m_d->mode) {
    case Private::OVER_POINT:
        cursor = KisCursor::pointingHandCursor();
        break;
    case Private::MULTIPLE_POINT_SELECTION:
        cursor = KisCursor::crossCursor();
        break;
    case Private::MOVE_MODE:
        cursor = KisCursor::moveCursor();
        break;
    case Private::ROTATE_MODE:
        cursor = KisCursor::rotateCursor();
        break;
    case Private::SCALE_MODE:
        cursor = KisCursor::sizeVerCursor();
        break;
    case Private::NOTHING:
        cursor = KisCursor::arrowCursor();
        break;
    }

    return cursor;
}

void KisWarpTransformStrategy::overrideDrawingItems(bool drawConnectionLines,
                                                    bool drawOrigPoints,
                                                    bool drawTransfPoints)
{
    m_d->drawConnectionLines = drawConnectionLines;
    m_d->drawOrigPoints = drawOrigPoints;
    m_d->drawTransfPoints = drawTransfPoints;
}

void KisWarpTransformStrategy::setCloseOnStartPointClick(bool value)
{
    m_d->closeOnStartPointClick = value;
}

void KisWarpTransformStrategy::setClipOriginalPointsPosition(bool value)
{
    m_d->clipOriginalPointsPosition = value;
}

void KisWarpTransformStrategy::setTransformType(TransformType type) {
    m_d->transformType = type;
}

void KisWarpTransformStrategy::drawConnectionLines(QPainter &gc,
                                                   const QVector<QPointF> &origPoints,
                                                   const QVector<QPointF> &transfPoints,
                                                   bool isEditingPoints)
{
    Q_UNUSED(isEditingPoints);

    QPen antsPen;
    QPen outlinePen;

    KisPaintingTweaks::initAntsPen(&antsPen, &outlinePen);

    const int numPoints = origPoints.size();

    for (int i = 0; i < numPoints; ++i) {
        gc.setPen(outlinePen);
        gc.drawLine(transfPoints[i], origPoints[i]);
        gc.setPen(antsPen);
        gc.drawLine(transfPoints[i], origPoints[i]);
    }
}

void KisWarpTransformStrategy::paint(QPainter &gc)
{
    // Draw preview image

    gc.save();

    gc.setOpacity(m_d->transaction.basePreviewOpacity());
    gc.setTransform(m_d->paintingTransform, true);
    gc.drawImage(m_d->paintingOffset, m_d->transformedImage);

    gc.restore();


    gc.save();
    gc.setTransform(m_d->handlesTransform, true);

    if (m_d->drawConnectionLines) {
        gc.setOpacity(0.5);

        drawConnectionLines(gc,
                            m_d->currentArgs.origPoints(),
                            m_d->currentArgs.transfPoints(),
                            m_d->currentArgs.isEditingTransformPoints());
    }


    QPen mainPen(Qt::black);
    QPen outlinePen(Qt::white);

    // draw handles
    {
        const int numPoints = m_d->currentArgs.origPoints().size();



        qreal handlesExtraScale = KisTransformUtils::scaleFromAffineMatrix(m_d->handlesTransform);

        qreal dstIn = 8 / handlesExtraScale;
        qreal dstOut = 10 / handlesExtraScale;
        qreal srcIn = 6 / handlesExtraScale;
        qreal srcOut = 6 / handlesExtraScale;

        QRectF handleRect1(-0.5 * dstIn, -0.5 * dstIn, dstIn, dstIn);
        QRectF handleRect2(-0.5 * dstOut, -0.5 * dstOut, dstOut, dstOut);

        if (m_d->drawTransfPoints) {
            gc.setOpacity(1.0);

            for (int i = 0; i < numPoints; ++i) {
                gc.setPen(outlinePen);
                gc.drawEllipse(handleRect2.translated(m_d->currentArgs.transfPoints()[i]));
                gc.setPen(mainPen);
                gc.drawEllipse(handleRect1.translated(m_d->currentArgs.transfPoints()[i]));
            }

            QPointF center;
            QVector<QPointF*> selectedPoints = m_d->getSelectedPoints(&center, true);

            QBrush selectionBrush = selectedPoints.size() > 1 ? Qt::red : Qt::black;

            QBrush oldBrush = gc.brush();
            gc.setBrush(selectionBrush);
            Q_FOREACH (const QPointF *pt, selectedPoints) {
                gc.drawEllipse(handleRect1.translated(*pt));
            }
            gc.setBrush(oldBrush);

        }

        if (m_d->drawOrigPoints) {
            QPainterPath inLine;
            inLine.moveTo(-0.5 * srcIn,            0);
            inLine.lineTo( 0.5 * srcIn,            0);
            inLine.moveTo(           0, -0.5 * srcIn);
            inLine.lineTo(           0,  0.5 * srcIn);

            QPainterPath outLine;
            outLine.moveTo(-0.5 * srcOut, -0.5 * srcOut);
            outLine.lineTo( 0.5 * srcOut, -0.5 * srcOut);
            outLine.lineTo( 0.5 * srcOut,  0.5 * srcOut);
            outLine.lineTo(-0.5 * srcOut,  0.5 * srcOut);
            outLine.lineTo(-0.5 * srcOut, -0.5 * srcOut);

            gc.setOpacity(0.5);

            for (int i = 0; i < numPoints; ++i) {
                gc.setPen(outlinePen);
                gc.drawPath(outLine.translated(m_d->currentArgs.origPoints()[i]));
                gc.setPen(mainPen);
                gc.drawPath(inLine.translated(m_d->currentArgs.origPoints()[i]));
            }
        }

    }

    // draw grid lines only if we are using the GRID mode. Also only use this logic for warp, not cage transforms
    if (m_d->currentArgs.warpCalculation() == KisWarpTransformWorker::WarpCalculation::GRID &&
        m_d->transformType == TransformType::WARP_TRANSFORM ) {

    // see how many rows we have. we are only going to do lines up to 6 divisions/
    // it is almost impossible to use with 6 even.
    const int numPoints = m_d->currentArgs.origPoints().size();

    // grid is always square, so get the square root to find # of rows
    int rowsInWarp = sqrt(m_d->currentArgs.origPoints().size());


        KisHandlePainterHelper handlePainter(&gc);
        handlePainter.setHandleStyle(KisHandleStyle::primarySelection());

        // draw horizontal lines
        for (int i = 0; i < numPoints; i++) {
            if (i != 0 &&  i % rowsInWarp == rowsInWarp -1) {
                // skip line if it is the last in the row
            } else {
                handlePainter.drawConnectionLine(m_d->currentArgs.transfPoints()[i], m_d->currentArgs.transfPoints()[i+1]  );
            }
        }

        // draw vertical lines
        for (int i = 0; i < numPoints; i++) {

            if ( (numPoints - i - 1) < rowsInWarp ) {
                // last row doesn't need to draw vertical lines
            } else {
                handlePainter.drawConnectionLine(m_d->currentArgs.transfPoints()[i], m_d->currentArgs.transfPoints()[i+rowsInWarp] );
            }
        }

    } // end if statement

    gc.restore();
}

void KisWarpTransformStrategy::externalConfigChanged()
{
    if (m_d->lastNumPoints != m_d->currentArgs.transfPoints().size()) {
        m_d->pointsInAction.clear();
    }

    m_d->recalculateTransformations();
}

bool KisWarpTransformStrategy::beginPrimaryAction(const QPointF &pt)
{
    const bool isEditingPoints = m_d->currentArgs.isEditingTransformPoints();
    bool retval = false;

    if (m_d->mode == Private::OVER_POINT ||
        m_d->mode == Private::MULTIPLE_POINT_SELECTION ||
        m_d->mode == Private::MOVE_MODE ||
        m_d->mode == Private::ROTATE_MODE ||
        m_d->mode == Private::SCALE_MODE) {

        retval = true;

    } else if (isEditingPoints) {
        QPointF newPos = m_d->clipOriginalPointsPosition ?
            KisTransformUtils::clipInRect(pt, m_d->transaction.originalRect()) :
            pt;

        m_d->currentArgs.refOriginalPoints().append(newPos);
        m_d->currentArgs.refTransformedPoints().append(newPos);

        m_d->mode = Private::OVER_POINT;
        m_d->pointIndexUnderCursor = m_d->currentArgs.origPoints().size() - 1;

        m_d->recalculateSignalCompressor.start();

        retval = true;
    }

    if (m_d->mode == Private::OVER_POINT) {
        m_d->pointPosOnClick =
            m_d->currentArgs.transfPoints()[m_d->pointIndexUnderCursor];
        m_d->pointWasDragged = false;

        m_d->pointsInAction.clear();
        m_d->pointsInAction << m_d->pointIndexUnderCursor;
        m_d->lastNumPoints = m_d->currentArgs.transfPoints().size();
    } else if (m_d->mode == Private::MULTIPLE_POINT_SELECTION) {
        QVector<int>::iterator it =
            std::find(m_d->pointsInAction.begin(),
                      m_d->pointsInAction.end(),
                      m_d->pointIndexUnderCursor);

        if (it != m_d->pointsInAction.end()) {
            m_d->pointsInAction.erase(it);
        } else {
            m_d->pointsInAction << m_d->pointIndexUnderCursor;
        }

        m_d->lastNumPoints = m_d->currentArgs.transfPoints().size();
    }

    m_d->lastMousePos = pt;
    return retval;
}

QVector<QPointF*> KisWarpTransformStrategy::Private::getSelectedPoints(QPointF *center, bool limitToSelectedOnly) const
{
    QVector<QPointF> &points = currentArgs.refTransformedPoints();

    QRectF boundingRect;
    QVector<QPointF*> selectedPoints;
    if (limitToSelectedOnly || pointsInAction.size() > 1) {
        Q_FOREACH (int index, pointsInAction) {
            selectedPoints << &points[index];
            KisAlgebra2D::accumulateBounds(points[index], &boundingRect);
        }
    } else {
        QVector<QPointF>::iterator it = points.begin();
        QVector<QPointF>::iterator end = points.end();
        for (; it != end; ++it) {
            selectedPoints << &(*it);
            KisAlgebra2D::accumulateBounds(*it, &boundingRect);
        }
    }

    *center = boundingRect.center();
    return selectedPoints;
}

void KisWarpTransformStrategy::continuePrimaryAction(const QPointF &pt, bool shiftModifierActve, bool altModifierActive)
{
    Q_UNUSED(shiftModifierActve);
    Q_UNUSED(altModifierActive);

    // toplevel code switches to HOVER mode if nothing is selected
    KIS_ASSERT_RECOVER_RETURN(m_d->mode == Private::MOVE_MODE ||
                              m_d->mode == Private::ROTATE_MODE ||
                              m_d->mode == Private::SCALE_MODE ||
                              (m_d->mode == Private::OVER_POINT &&
                               m_d->pointIndexUnderCursor >= 0 &&
                               m_d->pointsInAction.size() == 1) ||
                              (m_d->mode == Private::MULTIPLE_POINT_SELECTION &&
                               m_d->pointIndexUnderCursor >= 0));

    if (m_d->mode == Private::OVER_POINT) {
        if (m_d->currentArgs.isEditingTransformPoints()) {
            QPointF newPos = m_d->clipOriginalPointsPosition ?
                KisTransformUtils::clipInRect(pt, m_d->transaction.originalRect()) :
                pt;
            m_d->currentArgs.origPoint(m_d->pointIndexUnderCursor) = newPos;
            m_d->currentArgs.transfPoint(m_d->pointIndexUnderCursor) = newPos;
        } else {
            m_d->currentArgs.transfPoint(m_d->pointIndexUnderCursor) = pt;
        }


        const qreal handleRadiusSq = pow2(KisTransformUtils::effectiveHandleGrabRadius(m_d->converter));
        qreal dist =
            kisSquareDistance(
                m_d->currentArgs.transfPoint(m_d->pointIndexUnderCursor),
                m_d->pointPosOnClick);

        if (dist > handleRadiusSq) {
            m_d->pointWasDragged = true;
        }
    } else if (m_d->mode == Private::MOVE_MODE) {
        QPointF center;
        QVector<QPointF*> selectedPoints = m_d->getSelectedPoints(&center);

        QPointF diff = pt - m_d->lastMousePos;

        QVector<QPointF*>::iterator it = selectedPoints.begin();
        QVector<QPointF*>::iterator end = selectedPoints.end();
        for (; it != end; ++it) {
            **it += diff;
        }
    } else if (m_d->mode == Private::ROTATE_MODE) {
        QPointF center;
        QVector<QPointF*> selectedPoints = m_d->getSelectedPoints(&center);

        QPointF oldDirection = m_d->lastMousePos - center;
        QPointF newDirection = pt - center;

        qreal rotateAngle = KisAlgebra2D::angleBetweenVectors(oldDirection, newDirection);
        QTransform R;
        R.rotateRadians(rotateAngle);

        QTransform t =
            QTransform::fromTranslate(-center.x(), -center.y()) *
            R *
            QTransform::fromTranslate(center.x(), center.y());

        QVector<QPointF*>::iterator it = selectedPoints.begin();
        QVector<QPointF*>::iterator end = selectedPoints.end();
        for (; it != end; ++it) {
            **it = t.map(**it);
        }
    } else if (m_d->mode == Private::SCALE_MODE) {
        QPointF center;
        QVector<QPointF*> selectedPoints = m_d->getSelectedPoints(&center);

        QPolygonF polygon(m_d->currentArgs.origPoints());
        QSizeF maxSize = polygon.boundingRect().size();
        qreal maxDimension = qMax(maxSize.width(), maxSize.height());

        qreal scale = 1.0 - (pt - m_d->lastMousePos).y() / maxDimension;

        QTransform t =
            QTransform::fromTranslate(-center.x(), -center.y()) *
            QTransform::fromScale(scale, scale) *
            QTransform::fromTranslate(center.x(), center.y());

        QVector<QPointF*>::iterator it = selectedPoints.begin();
        QVector<QPointF*>::iterator end = selectedPoints.end();
        for (; it != end; ++it) {
            **it = t.map(**it);
        }
    }

    m_d->lastMousePos = pt;
    m_d->recalculateSignalCompressor.start();

}

bool KisWarpTransformStrategy::Private::shouldCloseTheCage() const
{
    return currentArgs.isEditingTransformPoints() &&
        closeOnStartPointClick &&
        pointIndexUnderCursor == 0 &&
        currentArgs.origPoints().size() > 2 &&
        !pointWasDragged;
}

bool KisWarpTransformStrategy::acceptsClicks() const
{
    return m_d->shouldCloseTheCage() ||
        m_d->currentArgs.isEditingTransformPoints();
}

bool KisWarpTransformStrategy::endPrimaryAction()
{
    if (m_d->shouldCloseTheCage()) {
        m_d->currentArgs.setEditingTransformPoints(false);
    }

    return true;
}

inline QPointF KisWarpTransformStrategy::Private::imageToThumb(const QPointF &pt, bool useFlakeOptimization)
{
    return useFlakeOptimization ? converter->imageToDocument(converter->documentToFlake((pt))) : q->thumbToImageTransform().inverted().map(pt);
}

void KisWarpTransformStrategy::Private::recalculateTransformations()
{
    QTransform scaleTransform = KisTransformUtils::imageToFlakeTransform(converter);

    QTransform resultThumbTransform = q->thumbToImageTransform() * scaleTransform;
    qreal scale = KisTransformUtils::scaleFromAffineMatrix(resultThumbTransform);
    bool useFlakeOptimization = scale < 1.0 &&
        !KisTransformUtils::thumbnailTooSmall(resultThumbTransform, q->originalImage().rect());

    QVector<QPointF> thumbOrigPoints(currentArgs.numPoints());
    QVector<QPointF> thumbTransfPoints(currentArgs.numPoints());

    for (int i = 0; i < currentArgs.numPoints(); ++i) {
        thumbOrigPoints[i] = imageToThumb(currentArgs.origPoints()[i], useFlakeOptimization);
        thumbTransfPoints[i] = imageToThumb(currentArgs.transfPoints()[i], useFlakeOptimization);
    }

    paintingOffset = transaction.originalTopLeft();

    if (!q->originalImage().isNull() && !currentArgs.isEditingTransformPoints()) {
        QPointF origTLInFlake = imageToThumb(transaction.originalTopLeft(), useFlakeOptimization);

        if (useFlakeOptimization) {
            transformedImage = q->originalImage().transformed(resultThumbTransform);
            paintingTransform = QTransform();
        } else {
            transformedImage = q->originalImage();
            paintingTransform = resultThumbTransform;

        }

        transformedImage = q->calculateTransformedImage(currentArgs,
                                                        transformedImage,
                                                        thumbOrigPoints,
                                                        thumbTransfPoints,
                                                        origTLInFlake,
                                                        &paintingOffset);
    } else {
        transformedImage = q->originalImage();
        paintingOffset = imageToThumb(transaction.originalTopLeft(), false);
        paintingTransform = resultThumbTransform;
    }

    handlesTransform = scaleTransform;
    emit q->requestCanvasUpdate();
}

QImage KisWarpTransformStrategy::calculateTransformedImage(ToolTransformArgs &currentArgs,
                                                           const QImage &srcImage,
                                                           const QVector<QPointF> &origPoints,
                                                           const QVector<QPointF> &transfPoints,
                                                           const QPointF &srcOffset,
                                                           QPointF *dstOffset)
{
    return KisWarpTransformWorker::transformQImage(
        currentArgs.warpType(),
        origPoints, transfPoints,
        currentArgs.alpha(),
        srcImage,
        srcOffset, dstOffset);
}

#include "moc_kis_warp_transform_strategy.cpp"
