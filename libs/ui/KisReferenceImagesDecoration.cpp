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

#include "KisDocument.h"

struct KisReferenceImagesDecoration::Private {

};

KisReferenceImagesDecoration::KisReferenceImagesDecoration(QPointer<KisView> parent)
    : KisCanvasDecoration("referenceImagesDecoration", parent)
    , d(new Private)
{
}


KisReferenceImagesDecoration::~KisReferenceImagesDecoration()
{

}

void KisReferenceImagesDecoration::addReferenceImage(KisReferenceImageSP referenceImage)
{
    QList<KisReferenceImageSP> images = view()->document()->referenceImages();
    images.append(referenceImage);
    view()->document()->setReferenceImages(images);
    setVisible(!images.isEmpty());
    emit referenceImagesChanged();
}

QList<KisReferenceImageSP> KisReferenceImagesDecoration::referenceImages() const
{
    return view()->document()->referenceImages();
}

void KisReferenceImagesDecoration::drawDecoration(QPainter &gc, const QRectF &updateRect, const KisCoordinatesConverter *converter, KisCanvas2 *canvas)
{
    Q_FOREACH(KisReferenceImageSP image, view()->document()->referenceImages()) {
        image->draw(gc, updateRect, converter, canvas);
    }
}
