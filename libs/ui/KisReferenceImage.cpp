/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#include "KisReferenceImage.h"
#include <QImage>
#include <QPainter>
#include <kundo2command.h>
#include <kis_coordinates_converter.h>

struct KisReferenceImage::Private {
    QImage image;
    QPointF pos;
    qreal saturation {1.0};
};


KisReferenceImage::SetSaturationCommand::SetSaturationCommand(const QList<KoShape *> &shapes, qreal newSaturation, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Set saturation"), parent)
    , newSaturation(newSaturation)
{
    images.reserve(shapes.count());

    Q_FOREACH(auto *shape, shapes) {
        auto *reference = dynamic_cast<KisReferenceImage*>(shape);
        KIS_SAFE_ASSERT_RECOVER_BREAK(reference);
        images.append(reference);
    }

    Q_FOREACH(auto *image, images) {
        oldSaturations.append(image->saturation());
    }
}

void KisReferenceImage::SetSaturationCommand::undo()
{
    auto saturationIterator = oldSaturations.begin();
    Q_FOREACH(auto *image, images) {
        image->setSaturation(*saturationIterator);
        image->update();
        saturationIterator++;
    }
}

void KisReferenceImage::SetSaturationCommand::redo()
{
    Q_FOREACH(auto *image, images) {
        image->setSaturation(newSaturation);
        image->update();
    }
}

KisReferenceImage::KisReferenceImage()
    : d(new Private)
{
    setKeepAspectRatio(true);
}

KisReferenceImage::~KisReferenceImage()
{

}

KisReferenceImage * KisReferenceImage::fromFile(const QString &filename)
{
    QImage img;
    img.load(filename);

    KisReferenceImage *reference = new KisReferenceImage();
    reference->setImage(img);
    reference->setSize(img.size());

    return reference;
}


void KisReferenceImage::setImage(QImage image)
{
    d->image = image;
}

void KisReferenceImage::setPosition(QPointF pos)
{
    d->pos = pos;
}

void KisReferenceImage::paint(QPainter &gc, const KoViewConverter &converter, KoShapePaintingContext &paintcontext)
{
    gc.save();

    applyConversion(gc, converter);

    QSizeF shapeSize = size();
    QTransform transform = QTransform::fromScale(shapeSize.width() / d->image.width(), shapeSize.height() / d->image.height());

    gc.setRenderHint(QPainter::SmoothPixmapTransform);
    gc.setClipRect(QRectF(QPointF(), shapeSize), Qt::IntersectClip);
    gc.setTransform(transform, true);
    gc.drawImage(QPoint(), d->image);

    gc.restore();
}

void KisReferenceImage::setSaturation(qreal saturation)
{
    d->saturation = saturation;
}

qreal KisReferenceImage::saturation() const
{
    return d->saturation;
}

QColor KisReferenceImage::getPixel(QPointF position)
{
    const QSizeF shapeSize = size();
    const QTransform scale = QTransform::fromScale(d->image.width() / shapeSize.width(), d->image.height() / shapeSize.height());

    const QTransform transform = absoluteTransformation(nullptr).inverted() * scale;
    const QPointF localPosition = position * transform;

    return d->image.pixelColor(localPosition.toPoint());
}
