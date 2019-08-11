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

#include "KoPointerEvent.h"

#include "kis_coordinates_converter.h"
#include "tool_transform_args.h"
#include "transform_transaction_properties.h"
#include "krita_utils.h"
#include "kis_cursor.h"
#include "kis_transform_utils.h"
#include "kis_algebra_2d.h"
#include "kis_liquify_paint_helper.h"
#include "kis_liquify_transform_worker.h"
#include "KoCanvasResourceProvider.h"


struct KisLiquifyTransformStrategy::Private
{
    Private(KisLiquifyTransformStrategy *_q,
            const KisCoordinatesConverter *_converter,
            ToolTransformArgs &_currentArgs,
            TransformTransactionProperties &_transaction,
            const KoCanvasResourceProvider *_manager)
        : manager(_manager),
          q(_q),
          converter(_converter),
          currentArgs(_currentArgs),
          transaction(_transaction),
          helper(_converter),
          recalculateOnNextRedraw(false)
    {
    }

    const KoCanvasResourceProvider *manager;

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

    // size-gesture-related
    QPointF lastMouseWidgetPos;
    QPointF startResizeImagePos;
    QPoint startResizeGlobalCursorPos;

    KisLiquifyPaintHelper helper;

    bool recalculateOnNextRedraw;

    void recalculateTransformations();
    inline QPointF imageToThumb(const QPointF &pt, bool useFlakeOptimization);
};

KisLiquifyTransformStrategy::KisLiquifyTransformStrategy(const KisCoordinatesConverter *converter,
                                                         ToolTransformArgs &currentArgs,
                                                         TransformTransactionProperties &transaction,
                                                         const KoCanvasResourceProvider *manager)

    : m_d(new Private(this, converter, currentArgs, transaction, manager))
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

    if (m_d->recalculateOnNextRedraw) {
        m_d->recalculateTransformations();
        m_d->recalculateOnNextRedraw = false;
    }

    gc.save();

    gc.setOpacity(m_d->transaction.basePreviewOpacity());
    gc.setTransform(m_d->paintingTransform, true);
    gc.drawImage(m_d->paintingOffset, m_d->transformedImage);

    gc.restore();
}

void KisLiquifyTransformStrategy::externalConfigChanged()
{
    if (!m_d->currentArgs.liquifyWorker()) return;
    m_d->recalculateTransformations();
}

bool KisLiquifyTransformStrategy::acceptsClicks() const
{
    return true;
}

bool KisLiquifyTransformStrategy::beginPrimaryAction(KoPointerEvent *event)
{
    m_d->helper.configurePaintOp(*m_d->currentArgs.liquifyProperties(), m_d->currentArgs.liquifyWorker());
    m_d->helper.startPaint(event, m_d->manager);

    m_d->recalculateTransformations();

    return true;
}

void KisLiquifyTransformStrategy::continuePrimaryAction(KoPointerEvent *event)
{
    m_d->helper.continuePaint(event);

    // the updates should be compressed
    m_d->recalculateOnNextRedraw = true;
    emit requestCanvasUpdate();
}

bool KisLiquifyTransformStrategy::endPrimaryAction(KoPointerEvent *event)
{
    if (m_d->helper.endPaint(event)) {
        m_d->recalculateTransformations();
        emit requestCanvasUpdate();
    }

    return true;
}

void KisLiquifyTransformStrategy::hoverActionCommon(KoPointerEvent *event)
{
    m_d->helper.hoverPaint(event);
}

void KisLiquifyTransformStrategy::activateAlternateAction(KisTool::AlternateAction action)
{
    if (action == KisTool::PickFgNode || action == KisTool::PickBgNode ||
        action == KisTool::PickFgImage || action == KisTool::PickBgImage) {

        KisLiquifyProperties *props = m_d->currentArgs.liquifyProperties();
        props->setReverseDirection(!props->reverseDirection());
        emit requestUpdateOptionWidget();
    }
}

void KisLiquifyTransformStrategy::deactivateAlternateAction(KisTool::AlternateAction action)
{
    if (action == KisTool::PickFgNode || action == KisTool::PickBgNode ||
        action == KisTool::PickFgImage || action == KisTool::PickBgImage) {

        KisLiquifyProperties *props = m_d->currentArgs.liquifyProperties();
        props->setReverseDirection(!props->reverseDirection());
        emit requestUpdateOptionWidget();
    }
}

bool KisLiquifyTransformStrategy::beginAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    if (action == KisTool::ChangeSize) {
        QPointF widgetPoint = m_d->converter->documentToWidget(event->point);
        m_d->lastMouseWidgetPos = widgetPoint;
        m_d->startResizeImagePos = m_d->converter->documentToImage(event->point);
        m_d->startResizeGlobalCursorPos = QCursor::pos();
        return true;
    } else if (action == KisTool::PickFgNode || action == KisTool::PickBgNode ||
               action == KisTool::PickFgImage || action == KisTool::PickBgImage) {

        return beginPrimaryAction(event);
    }


    return false;
}

void KisLiquifyTransformStrategy::continueAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    if (action == KisTool::ChangeSize) {
        QPointF widgetPoint = m_d->converter->documentToWidget(event->point);

        QPointF diff = widgetPoint - m_d->lastMouseWidgetPos;

        KisLiquifyProperties *props = m_d->currentArgs.liquifyProperties();
        const qreal linearizedOffset = diff.x() / KisTransformUtils::scaleFromAffineMatrix(m_d->converter->imageToWidgetTransform());
        const qreal newSize = qBound(props->minSize(), props->size() + linearizedOffset, props->maxSize());
        props->setSize(newSize);
        m_d->currentArgs.saveLiquifyTransformMode();

        m_d->lastMouseWidgetPos = widgetPoint;

        emit requestCursorOutlineUpdate(m_d->startResizeImagePos);
    } else if (action == KisTool::PickFgNode || action == KisTool::PickBgNode ||
               action == KisTool::PickFgImage || action == KisTool::PickBgImage) {

        return continuePrimaryAction(event);
    }
}

bool KisLiquifyTransformStrategy::endAlternateAction(KoPointerEvent *event, KisTool::AlternateAction action)
{
    Q_UNUSED(event);

    if (action == KisTool::ChangeSize) {
        QCursor::setPos(m_d->startResizeGlobalCursorPos);
        return true;
    } else if (action == KisTool::PickFgNode || action == KisTool::PickBgNode ||
               action == KisTool::PickFgImage || action == KisTool::PickBgImage) {
        return endPrimaryAction(event);
    }

    return false;
}

inline QPointF KisLiquifyTransformStrategy::Private::imageToThumb(const QPointF &pt, bool useFlakeOptimization)
{
    return useFlakeOptimization ? converter->imageToDocument(converter->documentToFlake((pt))) : q->thumbToImageTransform().inverted().map(pt);
}

void KisLiquifyTransformStrategy::Private::recalculateTransformations()
{
    KIS_ASSERT_RECOVER_RETURN(currentArgs.liquifyWorker());

    QTransform scaleTransform = KisTransformUtils::imageToFlakeTransform(converter);

    QTransform resultThumbTransform = q->thumbToImageTransform() * scaleTransform;
    qreal scale = KisTransformUtils::scaleFromAffineMatrix(resultThumbTransform);
    bool useFlakeOptimization = scale < 1.0 &&
        !KisTransformUtils::thumbnailTooSmall(resultThumbTransform, q->originalImage().rect());

    paintingOffset = transaction.originalTopLeft();
    if (!q->originalImage().isNull()) {
        if (useFlakeOptimization) {
            transformedImage = q->originalImage().transformed(resultThumbTransform);
            paintingTransform = QTransform();
        } else {
            transformedImage = q->originalImage();
            paintingTransform = resultThumbTransform;
        }

        QTransform imageToRealThumbTransform =
            useFlakeOptimization ?
            scaleTransform :
            q->thumbToImageTransform().inverted();

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
        paintingTransform = resultThumbTransform;
    }

    handlesTransform = scaleTransform;
}

