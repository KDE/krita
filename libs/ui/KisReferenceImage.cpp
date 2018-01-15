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
#include <kis_coordinates_converter.h>

struct KisReferenceImage::Private {
    QImage image;
    QPointF pos;
    bool grayscale;
};

KisReferenceImage::KisReferenceImage()
    : d(new Private)
{

}

KisReferenceImage::~KisReferenceImage()
{

}

void KisReferenceImage::setImage(QImage image)
{
    d->image = image;
}

void KisReferenceImage::setPosition(QPointF pos)
{
    d->pos = pos;
}

void KisReferenceImage::setGrayscale(bool grayscale)
{
    d->grayscale = grayscale;
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
