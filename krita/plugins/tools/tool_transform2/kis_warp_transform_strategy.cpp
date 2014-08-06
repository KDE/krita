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


struct KisWarpTransformStrategy::Private
{
    Private(KisWarpTransformStrategy *_q,
            const KisCoordinatesConverter *_converter,
            ToolTransformArgs &_currentArgs,
            TransformTransactionProperties &_transaction)
        : q(_q),
          converter(_converter),
          currentArgs(_currentArgs),
          transaction(_transaction)
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

    bool cursorOverPoint;
    int pointIndexUnderCursor;

    void recalculateTransformations();
    inline QPointF imageToThumb(const QPointF &pt, bool useFlakeOptimization);
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

    m_d->cursorOverPoint = false;
    m_d->pointIndexUnderCursor = -1;

    const QVector<QPointF> &points = m_d->currentArgs.transfPoints();
    for (int i = 0; i < points.size(); ++i) {
        if (kisSquareDistance(mousePos, points[i]) <= handleRadiusSq) {
            m_d->cursorOverPoint = true;
            m_d->pointIndexUnderCursor = i;
            break;
        }
    }
}

QCursor KisWarpTransformStrategy::getCurrentCursor() const
{
    if (m_d->cursorOverPoint) {
        return KisCursor::pointingHandCursor();
    } else {
        return KisCursor::arrowCursor();
    }
}

void KisWarpTransformStrategy::paint(QPainter &gc)
{
    // Draw handles

    int numPoints = m_d->currentArgs.origPoints().size();

    gc.save();

    gc.setOpacity(0.9);
    gc.setTransform(m_d->paintingTransform, true);
    gc.drawImage(m_d->paintingOffset, m_d->transformedImage);

    gc.restore();


    gc.save();
    gc.setTransform(m_d->handlesTransform, true);

    // draw connecting lines
    {
        QPen antsPen;
        QPen outlinePen;

        KritaUtils::initAntsPen(&antsPen, &outlinePen);

        gc.setOpacity(0.5);

        for (int i = 0; i < numPoints; ++i) {
            gc.setPen(outlinePen);
            gc.drawLine(m_d->currentArgs.transfPoints()[i], m_d->currentArgs.origPoints()[i]);
            gc.setPen(antsPen);
            gc.drawLine(m_d->currentArgs.transfPoints()[i], m_d->currentArgs.origPoints()[i]);
        }
    }

    // draw handles themselves
    {
        QPen mainPen(Qt::black);
        QPen outlinePen(Qt::white);

        qreal handlesExtraScale = KisTransformUtils::scaleFromAffineMatrix(m_d->handlesTransform);

        qreal dstIn = 8 / handlesExtraScale;
        qreal dstOut = 10 / handlesExtraScale;
        qreal srcIn = 6 / handlesExtraScale;
        qreal srcOut = 6 / handlesExtraScale;

        QRectF handleRect1(-0.5 * dstIn, -0.5 * dstIn, dstIn, dstIn);
        QRectF handleRect2(-0.5 * dstOut, -0.5 * dstOut, dstOut, dstOut);

        gc.setOpacity(1.0);

        for (int i = 0; i < numPoints; ++i) {
            gc.setPen(outlinePen);
            gc.drawEllipse(handleRect2.translated(m_d->currentArgs.transfPoints()[i]));
            gc.setPen(mainPen);
            gc.drawEllipse(handleRect1.translated(m_d->currentArgs.transfPoints()[i]));
        }

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
    gc.restore();
}

void KisWarpTransformStrategy::externalConfigChanged()
{
    m_d->recalculateTransformations();
}

bool KisWarpTransformStrategy::beginPrimaryAction(const QPointF &pt)
{
    if (m_d->cursorOverPoint) return true;

    bool hasSomethingToDrag = m_d->transaction.editWarpPoints();

    if (hasSomethingToDrag) {

        m_d->currentArgs.refOriginalPoints().append(pt);
        m_d->currentArgs.refTransformedPoints().append(pt);

        m_d->cursorOverPoint = true;
        m_d->pointIndexUnderCursor = m_d->currentArgs.origPoints().size() - 1;

        m_d->recalculateTransformations();
        emit requestCanvasUpdate();
    }

    return hasSomethingToDrag;
}

void KisWarpTransformStrategy::continuePrimaryAction(const QPointF &pt, bool specialModifierActve)
{
    Q_UNUSED(specialModifierActve);

    // toplevel code switches to HOVER mode if nothing is selected
    KIS_ASSERT_RECOVER_RETURN(m_d->pointIndexUnderCursor >= 0);

    if (m_d->transaction.editWarpPoints()) {
        QPointF newPos = KisTransformUtils::clipInRect(pt, m_d->transaction.originalRect());

        m_d->currentArgs.origPoint(m_d->pointIndexUnderCursor) = newPos;
        m_d->currentArgs.transfPoint(m_d->pointIndexUnderCursor) = newPos;
    }
    else {
        m_d->currentArgs.transfPoint(m_d->pointIndexUnderCursor) = pt;
    }

    m_d->recalculateTransformations();
    emit requestCanvasUpdate();
}

bool KisWarpTransformStrategy::endPrimaryAction()
{
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

        transformedImage = KisWarpTransformWorker::transformation(currentArgs.warpType(), &transformedImage, thumbOrigPoints, thumbTransfPoints, currentArgs.alpha(), origTLInFlake, &paintingOffset);
    } else {
        transformedImage = q->originalImage();
        paintingOffset = imageToThumb(transaction.originalTopLeft(), false);
        paintingTransform = q->thumbToImageTransform() * scaleTransform;
    }

    handlesTransform = scaleTransform;
}
