/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisReferenceImagesDecoration.h"

#include "KoShapeManager.h"

#include "kis_algebra_2d.h"
#include "KisDocument.h"
#include "KisReferenceImagesLayer.h"

struct KisReferenceImagesDecoration::Private {
    struct Buffer
    {
        /// Top left corner of the buffer relative to the viewport
        QPointF position;
        QImage image;

        QRectF bounds() const
        {
            return QRectF(position, image.size());
        }
    };

    KisReferenceImagesDecoration *q;

    KisWeakSharedPtr<KisReferenceImagesLayer> layer;
    Buffer buffer;
    QTransform previousTransform;
    QSizeF previousViewSize;

    explicit Private(KisReferenceImagesDecoration *q)
        : q(q)
    {}

    void updateBufferByImageCoordinates(const QRectF &dirtyImageRect)
    {
        QRectF dirtyWidgetRect = q->view()->viewConverter()->imageToWidget(dirtyImageRect);
        updateBuffer(dirtyWidgetRect, dirtyImageRect);
    }

    void updateBufferByWidgetCoordinates(const QRectF &dirtyWidgetRect)
    {
        QRectF dirtyImageRect = q->view()->viewConverter()->widgetToImage(dirtyWidgetRect);
        updateBuffer(dirtyWidgetRect, dirtyImageRect);
    }

private:
    void updateBuffer(QRectF widgetRect, QRectF imageRect)
    {
        layer->updateTransformations(q->view()->canvasBase());

        qreal devicePixelRatioF = q->view()->devicePixelRatioF();
        if (buffer.image.isNull() || !buffer.bounds().contains(widgetRect)) {
            const QRectF boundingWidgetRect = layer->boundingRect();
            widgetRect = boundingWidgetRect.intersected(q->view()->rect());

            if (widgetRect.isNull()) return;

            buffer.position = widgetRect.topLeft();
            // to ensure that buffer is big enough for all the pixels on high dpi displays
            // BUG 411118
            buffer.image = QImage((widgetRect.size()*devicePixelRatioF).toSize(), QImage::Format_ARGB32);
            buffer.image.setDevicePixelRatio(devicePixelRatioF);

            imageRect = widgetRect;

        }

        QPainter gc(&buffer.image);

        gc.translate(-buffer.position);
        gc.setTransform(QTransform(), true);

        gc.save();
        gc.setCompositionMode(QPainter::CompositionMode_Source);
        gc.fillRect(imageRect, Qt::transparent);
        gc.restore();

        // to ensure that clipping rect is also big enough for all the pixels
        // BUG 411118
        gc.setClipRect(QRectF(imageRect.topLeft(), imageRect.size()*devicePixelRatioF));
        layer->paintReferences(gc);
    }
};

KisReferenceImagesDecoration::KisReferenceImagesDecoration(QPointer<KisView> parent, KisDocument *document, bool viewReady)
    : KisCanvasDecoration("referenceImagesDecoration", parent)
    , d(new Private(this))
{
    connect(document->image().data(), SIGNAL(sigNodeAddedAsync(KisNodeSP)), this, SLOT(slotNodeAdded(KisNodeSP)));
    connect(document, &KisDocument::sigReferenceImagesLayerChanged, this, &KisReferenceImagesDecoration::slotNodeAdded);

    auto referenceImageLayer = document->referenceImagesLayer();
    if (referenceImageLayer) {
        setReferenceImageLayer(referenceImageLayer, /* updateCanvas = */ viewReady);
    }
}

KisReferenceImagesDecoration::~KisReferenceImagesDecoration()
{}

void KisReferenceImagesDecoration::addReferenceImage(KisReferenceImage *referenceImage)
{
    KisDocument *document = view()->document();
    KUndo2Command *cmd = KisReferenceImagesLayer::addReferenceImages(document, {referenceImage});
    document->addCommand(cmd);
}

bool KisReferenceImagesDecoration::documentHasReferenceImages() const
{
    return view()->document()->referenceImagesLayer() != nullptr;
}

void KisReferenceImagesDecoration::drawDecoration(QPainter &gc, const QRectF &/*updateRect*/, const KisCoordinatesConverter *converter, KisCanvas2 */*canvas*/)
{
    // TODO: can we use partial updates here?

    KisSharedPtr<KisReferenceImagesLayer> layer = d->layer.toStrongRef();

    if (!layer.isNull()) {
        QSizeF viewSize = view()->size();

        QTransform transform = converter->imageToWidgetTransform();
        if (d->previousViewSize != viewSize || !KisAlgebra2D::fuzzyMatrixCompare(transform, d->previousTransform, 1e-4)) {
            d->previousViewSize = viewSize;
            d->previousTransform = transform;
            d->buffer.image = QImage();
            d->updateBufferByWidgetCoordinates(QRectF(QPointF(0,0), viewSize));
        }

        if (!d->buffer.image.isNull()) {
            gc.drawImage(d->buffer.position, d->buffer.image);
        }
    }
}

void KisReferenceImagesDecoration::slotNodeAdded(KisNodeSP node)
{
    auto *referenceImagesLayer = dynamic_cast<KisReferenceImagesLayer*>(node.data());

    if (referenceImagesLayer) {
        setReferenceImageLayer(referenceImagesLayer, /* updateCanvas = */ true);
    }
}

void KisReferenceImagesDecoration::slotReferenceImagesChanged(const QRectF &dirtyRect)
{
    d->updateBufferByImageCoordinates(dirtyRect);

    QRectF documentRect = view()->viewConverter()->imageToDocument(dirtyRect);
    view()->canvasBase()->updateCanvas(documentRect);
}

void KisReferenceImagesDecoration::setReferenceImageLayer(KisSharedPtr<KisReferenceImagesLayer> layer, bool updateCanvas)
{
    if (d->layer != layer.data()) {
        if (d->layer) {
            d->layer.toStrongRef()->disconnect(this);
        }

        d->layer = layer;

        if (layer) {
            connect(layer.data(), SIGNAL(sigUpdateCanvas(QRectF)),
                    this, SLOT(slotReferenceImagesChanged(QRectF)));

            const QRectF dirtyRect = layer->boundingImageRect();

            // If the view is not ready yet (because this is being constructed
            // from view.d's ctor and thus view.d is not available now),
            // do not update canvas because it will lead to a crash.
            if (updateCanvas && !dirtyRect.isEmpty()) { // in case the reference layer is just being loaded from the .kra file
                slotReferenceImagesChanged(dirtyRect);
            }
        }
    }
}
