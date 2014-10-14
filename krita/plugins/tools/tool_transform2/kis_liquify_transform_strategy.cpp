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

#include "kis_liquify_transform_strategy.h"

#include <algorithm>

#include <QPointF>
#include <QPainter>

#include "kis_coordinates_converter.h"
#include "tool_transform_args.h"
#include "transform_transaction_properties.h"
#include "krita_utils.h"
#include "kis_cursor.h"
#include "kis_transform_utils.h"
#include "kis_algebra_2d.h"
#include "kis_liquify_paint_helper.h"
#include "kis_liquify_transform_worker.h"


struct KisLiquifyTransformStrategy::Private
{
    Private(KisLiquifyTransformStrategy *_q,
            const KisCoordinatesConverter *_converter,
            ToolTransformArgs &_currentArgs,
            TransformTransactionProperties &_transaction)
        : q(_q),
          converter(_converter),
          currentArgs(_currentArgs),
          transaction(_transaction),
          helper(_converter)
    {
    }

    KisLiquifyTransformStrategy * const q;

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
    QPointF lastMousePos;
    KisLiquifyPaintHelper helper;

    void recalculateTransformations();
    inline QPointF imageToThumb(const QPointF &pt, bool useFlakeOptimization);
};

KisLiquifyTransformStrategy::KisLiquifyTransformStrategy(const KisCoordinatesConverter *converter,
                                                         ToolTransformArgs &currentArgs,
                                                         TransformTransactionProperties &transaction)

    : KisTransformStrategyBase(converter),
      m_d(new Private(this, converter, currentArgs, transaction))
{
}

KisLiquifyTransformStrategy::~KisLiquifyTransformStrategy()
{
}

QPainterPath KisLiquifyTransformStrategy::getCursorOutline() const
{
    return m_d->helper.brushOutline(*m_d->currentArgs.liquifyProperties());
}

void KisLiquifyTransformStrategy::setTransformFunction(const QPointF &mousePos, bool perspectiveModifierActive)
{
    Q_UNUSED(mousePos);
    Q_UNUSED(perspectiveModifierActive);
}

QCursor KisLiquifyTransformStrategy::getCurrentCursor() const
{
    return Qt::BlankCursor;
}

void KisLiquifyTransformStrategy::paint(QPainter &gc)
{
    // Draw preview image

    gc.save();

    gc.setOpacity(m_d->transaction.basePreviewOpacity());
    gc.setTransform(m_d->paintingTransform, true);
    gc.drawImage(m_d->paintingOffset, m_d->transformedImage);

    gc.restore();
}

void KisLiquifyTransformStrategy::externalConfigChanged()
{
    m_d->recalculateTransformations();
}

bool KisLiquifyTransformStrategy::acceptsClicks() const
{
    return true;
}

bool KisLiquifyTransformStrategy::beginPrimaryAction(KoPointerEvent *event)
{
    m_d->helper.configurePaintOp(*m_d->currentArgs.liquifyProperties(), m_d->currentArgs.liquifyWorker());
    m_d->helper.startPaint(event);

    m_d->recalculateTransformations();

    return true;
}

void KisLiquifyTransformStrategy::continuePrimaryAction(KoPointerEvent *event, bool specialModifierActve)
{
    Q_UNUSED(specialModifierActve);

    m_d->helper.continuePaint(event);

    m_d->recalculateTransformations();
    emit requestCanvasUpdate();
}

bool KisLiquifyTransformStrategy::endPrimaryAction(KoPointerEvent *event)
{
    m_d->helper.endPaint(event);

    return true;
}

void KisLiquifyTransformStrategy::hoverPrimaryAction(KoPointerEvent *event)
{
    m_d->helper.hoverPaint(event);
}

inline QPointF KisLiquifyTransformStrategy::Private::imageToThumb(const QPointF &pt, bool useFlakeOptimization)
{
    return useFlakeOptimization ? converter->imageToDocument(converter->documentToFlake((pt))) : q->thumbToImageTransform().inverted().map(pt);
}

void KisLiquifyTransformStrategy::Private::recalculateTransformations()
{
    QTransform scaleTransform = KisTransformUtils::imageToFlakeTransform(converter);

    QTransform resultTransform = q->thumbToImageTransform() * scaleTransform;
    qreal scale = KisTransformUtils::scaleFromAffineMatrix(resultTransform);
    bool useFlakeOptimization = scale < 1.0;

    paintingOffset = transaction.originalTopLeft();
    if (!q->originalImage().isNull()) {
        if (useFlakeOptimization) {
            transformedImage = q->originalImage().transformed(q->thumbToImageTransform() * scaleTransform);
            paintingTransform = QTransform();
        } else {
            transformedImage = q->originalImage();
            paintingTransform = q->thumbToImageTransform() * scaleTransform;
        }

        QTransform imageToRealThumbTransform =
            useFlakeOptimization ?
            scaleTransform :
            QTransform();

        QPointF origTLInFlake =
            imageToRealThumbTransform.map(transaction.originalTopLeft());

        transformedImage =
            currentArgs.liquifyWorker()->runOnQImage(transformedImage,
                                                     origTLInFlake,
                                                     imageToRealThumbTransform,
                                                     &paintingOffset);
    } else {
        transformedImage = q->originalImage();
        paintingOffset = imageToThumb(transaction.originalTopLeft(), false);
        paintingTransform = q->thumbToImageTransform() * scaleTransform;
    }

    handlesTransform = scaleTransform;
}

