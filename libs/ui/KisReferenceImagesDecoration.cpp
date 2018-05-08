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
    void updateBuffer(const QRectF &widgetRect, const QRectF &imageRect)
    {
        KisCoordinatesConverter *viewConverter = q->view()->viewConverter();
        QTransform transform = viewConverter->imageToWidgetTransform();

        if (buffer.image.isNull() || !buffer.bounds().contains(widgetRect)) {
            // TODO: only use enough buffer to cover the BB of the shapes
            buffer.position = QPointF();
            buffer.image = QImage(q->view()->width(), q->view()->height(), QImage::Format_ARGB32);
            buffer.image.fill(Qt::transparent);
        }

        QPainter gc(&buffer.image);
        gc.setTransform(transform);

        gc.save();
        gc.setCompositionMode(QPainter::CompositionMode_Source);
        gc.fillRect(imageRect, Qt::transparent);
        gc.restore();

        gc.setClipRect(imageRect);
        layer->paintReferences(gc);
    }
};

KisReferenceImagesDecoration::KisReferenceImagesDecoration(QPointer<KisView> parent)
    : KisCanvasDecoration("referenceImagesDecoration", parent)
    , d(new Private(this))
{}

KisReferenceImagesDecoration::~KisReferenceImagesDecoration()
{}

void KisReferenceImagesDecoration::addReferenceImage(KisReferenceImage *referenceImage)
{
    KisSharedPtr<KisReferenceImagesLayer> layer = view()->document()->createReferenceImagesLayer();
    KIS_SAFE_ASSERT_RECOVER_RETURN(layer);

    KUndo2Command *cmd = layer->addReferenceImage(referenceImage);
    view()->document()->addCommand(cmd);
}

bool KisReferenceImagesDecoration::documentHasReferenceImages() const
{
    return view()->document()->referenceImagesLayer() != nullptr;
}

void KisReferenceImagesDecoration::drawDecoration(QPainter &gc, const QRectF &updateRect, const KisCoordinatesConverter */*converter*/, KisCanvas2 */*canvas*/)
{
    KisSharedPtr<KisReferenceImagesLayer> layer = d->layer.toStrongRef();
    if (layer.isNull()) {
        layer = d->layer = view()->document()->referenceImagesLayer();
        if (layer.isNull()) return;

        connect(layer.data(), SIGNAL(sigUpdateCanvas(const QRectF&)), this, SLOT(slotReferenceImagesChanged(const QRectF&)));

        d->updateBufferByWidgetCoordinates(updateRect);
    } else {
        QTransform transform = view()->viewConverter()->imageToWidgetTransform();
        if (!KisAlgebra2D::fuzzyMatrixCompare(transform, d->previousTransform, 1e-4)) {
            d->previousTransform = transform;
            d->updateBufferByWidgetCoordinates(QRectF(0, 0, view()->width(), view()->height()));
        }
    }

    gc.drawImage(d->buffer.position, d->buffer.image);
}

void KisReferenceImagesDecoration::slotReferenceImagesChanged(const QRectF &dirtyRect)
{
    d->updateBufferByImageCoordinates(dirtyRect);

    QRectF documentRect = view()->viewConverter()->imageToDocument(dirtyRect);
    view()->canvasBase()->updateCanvas(documentRect);
}
