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

#include <QPointF>
#include <QPainter>

#include "kis_coordinates_converter.h"
#include "tool_transform_args.h"
#include "transform_transaction_properties.h"
#include "krita_utils.h"
#include "kis_cursor.h"
#include "kis_transform_utils.h"
#include "kis_algebra_2d.h"


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
          drawConnectionLines(true),
          drawOrigPoints(true),
          drawTransfPoints(true),
          closeOnStartPointClick(false),
          clipOriginalPointsPosition(true),
          pointWasDragged(false)
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
        INSIDE_POLYGON,
        OUTSIDE_POLYGON,
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

    void recalculateTransformations();
    inline QPointF imageToThumb(const QPointF &pt, bool useFlakeOptimization);

    bool shouldCloseTheCage() const;
    QVector<QPointF*> getSelectedPoints(QPointF *center, bool limitToSelectedOnly = false) const;
};

KisWarpTransformStrategy::KisWarpTransformStrategy(const KisCoordinatesConverter *converter,
                                                   ToolTransformArgs &currentArgs,
                                                   TransformTransactionProperties &transaction)
    : m_d(new Private(this, converter, currentArgs, transaction))
{
}

KisWarpTransformStrategy::~KisWarpTransformStrategy()
{
}

void KisWarpTransformStrategy::setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive)
{
    Q_UNUSED(perspectiveModifierActive);

    double handleRadiusSq = pow2(KisTransformUtils::effectiveHandleGrabRadius(m_d->converter));

    bool cursorOverPoint = false;
    m_d->pointIndexUnderCursor = -1;

    const QVector<QPointF> &points = m_d->currentArgs.transfPoints();
    for (int i = 0; i < points.size(); ++i) {
        if (kisSquareDistance(mousePos, points[i]) <= handleRadiusSq) {
            cursorOverPoint = true;
            m_d->pointIndexUnderCursor = i;
            break;
        }
    }

    if (cursorOverPoint) {
        m_d->mode = perspectiveModifierActive &&
            !m_d->transaction.editWarpPoints() ?
            Private::MULTIPLE_POINT_SELECTION : Private::OVER_POINT;

    } else if (!m_d->transaction.editWarpPoints()) {
        QPolygonF polygon(m_d->currentArgs.transfPoints());
        bool insidePolygon = polygon.boundingRect().contains(mousePos);
        m_d->mode = insidePolygon ? Private::INSIDE_POLYGON : Private::OUTSIDE_POLYGON;
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
    case Private::INSIDE_POLYGON:
        cursor = KisCursor::moveCursor();
        break;
    case Private::OUTSIDE_POLYGON:
        cursor = KisCursor::rotateCursor();
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

void KisWarpTransformStrategy::drawConnectionLines(QPainter &gc,
                                                   const QVector<QPointF> &origPoints,
                                                   const QVector<QPointF> &transfPoints,
                                                   bool isEditingPoints)
{
    Q_UNUSED(isEditingPoints);

    QPen antsPen;
    QPen outlinePen;

    KritaUtils::initAntsPen(&antsPen, &outlinePen);

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

    gc.setOpacity(0.9);
    gc.setTransform(m_d->paintingTransform, true);
    gc.drawImage(m_d->paintingOffset, m_d->transformedImage);

    gc.restore();


    gc.save();
    gc.setTransform(m_d->handlesTransform, true);

    // draw connecting lines
    if (m_d->drawConnectionLines) {
        gc.setOpacity(0.5);

        drawConnectionLines(gc,
                            m_d->currentArgs.origPoints(),
                            m_d->currentArgs.transfPoints(),
                            m_d->transaction.editWarpPoints());
    }

    // draw handles
    {
        const int numPoints = m_d->currentArgs.origPoints().size();

        QPen mainPen(Qt::black);
        QPen outlinePen(Qt::white);

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
            foreach (const QPointF *pt, selectedPoints) {
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
    const bool isEditingPoints = m_d->transaction.editWarpPoints();
    bool retval = false;

    if (m_d->mode == Private::OVER_POINT ||
        m_d->mode == Private::MULTIPLE_POINT_SELECTION ||
        m_d->mode == Private::INSIDE_POLYGON ||
        m_d->mode == Private::OUTSIDE_POLYGON) {

        retval = true;

    } else if (isEditingPoints) {
        QPointF newPos = m_d->clipOriginalPointsPosition ?
            KisTransformUtils::clipInRect(pt, m_d->transaction.originalRect()) :
            pt;

        m_d->currentArgs.refOriginalPoints().append(newPos);
        m_d->currentArgs.refTransformedPoints().append(newPos);

        m_d->mode = Private::OVER_POINT;
        m_d->pointIndexUnderCursor = m_d->currentArgs.origPoints().size() - 1;

        m_d->recalculateTransformations();
        emit requestCanvasUpdate();

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
        m_d->pointsInAction << m_d->pointIndexUnderCursor;
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
        foreach (int index, pointsInAction) {
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

void KisWarpTransformStrategy::continuePrimaryAction(const QPointF &pt, bool specialModifierActve)
{
    Q_UNUSED(specialModifierActve);

    // toplevel code switches to HOVER mode if nothing is selected
    KIS_ASSERT_RECOVER_RETURN(m_d->mode == Private::INSIDE_POLYGON ||
                              m_d->mode == Private::OUTSIDE_POLYGON||
                              (m_d->mode == Private::OVER_POINT &&
                               m_d->pointIndexUnderCursor >= 0 &&
                               m_d->pointsInAction.size() == 1) ||
                              (m_d->mode == Private::MULTIPLE_POINT_SELECTION &&
                               m_d->pointIndexUnderCursor >= 0 &&
                               m_d->pointsInAction.size() >= 1));

    if (m_d->mode == Private::OVER_POINT) {
        if (m_d->transaction.editWarpPoints()) {
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
    } else if (m_d->mode == Private::INSIDE_POLYGON) {
        QPointF center;
        QVector<QPointF*> selectedPoints = m_d->getSelectedPoints(&center);

        QPointF diff = pt - m_d->lastMousePos;

        QVector<QPointF*>::iterator it = selectedPoints.begin();
        QVector<QPointF*>::iterator end = selectedPoints.end();
        for (; it != end; ++it) {
            **it += diff;
        }
    } else if (m_d->mode == Private::OUTSIDE_POLYGON) {
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
    }


    m_d->lastMousePos = pt;
    m_d->recalculateTransformations();
    emit requestCanvasUpdate();
}

bool KisWarpTransformStrategy::Private::shouldCloseTheCage() const
{
    return transaction.editWarpPoints() &&
        closeOnStartPointClick &&
        pointIndexUnderCursor == 0 &&
        currentArgs.origPoints().size() > 2 &&
        !pointWasDragged;
}

bool KisWarpTransformStrategy::acceptsClicks() const
{
    return m_d->shouldCloseTheCage();
}

bool KisWarpTransformStrategy::endPrimaryAction()
{
    if (m_d->shouldCloseTheCage()) {
        m_d->transaction.setEditWarpPoints(false);
        return false;
    }

    return m_d->currentArgs.defaultPoints() || !m_d->transaction.editWarpPoints();
}

inline QPointF KisWarpTransformStrategy::Private::imageToThumb(const QPointF &pt, bool useFlakeOptimization)
{
    return useFlakeOptimization ? converter->imageToDocument(converter->documentToFlake((pt))) : q->thumbToImageTransform().inverted().map(pt);
}

void KisWarpTransformStrategy::Private::recalculateTransformations()
{
    QTransform scaleTransform = KisTransformUtils::imageToFlakeTransform(converter);

    QTransform resultTransform = q->thumbToImageTransform() * scaleTransform;
    qreal scale = KisTransformUtils::scaleFromAffineMatrix(resultTransform);
    bool useFlakeOptimization = scale < 1.0;

    QVector<QPointF> thumbOrigPoints(currentArgs.numPoints());
    QVector<QPointF> thumbTransfPoints(currentArgs.numPoints());

    for (int i = 0; i < currentArgs.numPoints(); ++i) {
        thumbOrigPoints[i] = imageToThumb(currentArgs.origPoints()[i], useFlakeOptimization);
        thumbTransfPoints[i] = imageToThumb(currentArgs.transfPoints()[i], useFlakeOptimization);
    }

    paintingOffset = transaction.originalTopLeft();

    if (!q->originalImage().isNull() && !transaction.editWarpPoints()) {
        QPointF origTLInFlake = imageToThumb(transaction.originalTopLeft(), useFlakeOptimization);

        if (useFlakeOptimization) {
            transformedImage = q->originalImage().transformed(q->thumbToImageTransform() * scaleTransform);
            paintingTransform = QTransform();
        } else {
            transformedImage = q->originalImage();
            paintingTransform = q->thumbToImageTransform() * scaleTransform;

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
        paintingTransform = q->thumbToImageTransform() * scaleTransform;
    }

    handlesTransform = scaleTransform;
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
