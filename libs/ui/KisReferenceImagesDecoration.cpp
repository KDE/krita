/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
        KisCoordinatesConverter *viewConverter = q->view()->viewConverter();
        QTransform transform = viewConverter->imageToWidgetTransform();

        if (buffer.image.isNull() || !buffer.bounds().contains(widgetRect)) {
            const QRectF boundingImageRect = layer->boundingImageRect();
            const QRectF boundingWidgetRect = q->view()->viewConverter()->imageToWidget(boundingImageRect);
            widgetRect = boundingWidgetRect.intersected(q->view()->rect());

            if (widgetRect.isNull()) return;

            buffer.position = widgetRect.topLeft();
            buffer.image = QImage(widgetRect.size().toSize(), QImage::Format_ARGB32);
            buffer.image.fill(Qt::transparent);

            imageRect = q->view()->viewConverter()->widgetToImage(widgetRect);
        }

        QPainter gc(&buffer.image);

        gc.translate(-buffer.position);
        gc.setTransform(transform, true);

        gc.save();
        gc.setCompositionMode(QPainter::CompositionMode_Source);
        gc.fillRect(imageRect, Qt::transparent);
        gc.restore();

        gc.setClipRect(imageRect);
        layer->paintReferences(gc);
    }
};

KisReferenceImagesDecoration::KisReferenceImagesDecoration(QPointer<KisView> parent, KisDocument *document)
    : KisCanvasDecoration("referenceImagesDecoration", parent)
    , d(new Private(this))
{
    connect(document->image().data(), SIGNAL(sigNodeAddedAsync(KisNodeSP)), this, SLOT(slotNodeAdded(KisNodeSP)));

    auto referenceImageLayer = document->referenceImagesLayer();
    if (referenceImageLayer) {
        setReferenceImageLayer(referenceImageLayer);
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
    Q_UNUSED(updateRect);

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
        setReferenceImageLayer(referenceImagesLayer);
    }
}

void KisReferenceImagesDecoration::slotReferenceImagesChanged(const QRectF &dirtyRect)
{
    d->updateBufferByImageCoordinates(dirtyRect);

    QRectF documentRect = view()->viewConverter()->imageToDocument(dirtyRect);
    view()->canvasBase()->updateCanvas(documentRect);
}

void KisReferenceImagesDecoration::setReferenceImageLayer(KisSharedPtr<KisReferenceImagesLayer> layer)
{
    d->layer = layer;
    connect(
            layer.data(), SIGNAL(sigUpdateCanvas(const QRectF&)),
            this, SLOT(slotReferenceImagesChanged(const QRectF&))
    );
}
