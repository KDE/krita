/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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

#include "KoDrag.h"
#include "KoDragOdfSaveHelper.h"

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QClipboard>
#include <QMimeData>
#include <QString>
#include <QTransform>

#include <FlakeDebug.h>

#include <KoStore.h>
#include <KoGenStyles.h>
#include <KoOdfWriteStore.h>
#include <KoXmlWriter.h>
#include <KoDocumentBase.h>
#include <KoEmbeddedDocumentSaver.h>
#include "KoShapeSavingContext.h"

#include <KoShapeContainer.h>
#include <KoShape.h>

#include <QRect>
#include <SvgWriter.h>


class KoDragPrivate {
public:
    KoDragPrivate() : mimeData(0) { }
    ~KoDragPrivate() { delete mimeData; }
    QMimeData *mimeData;
};

KoDrag::KoDrag()
    : d(new KoDragPrivate())
{
}

KoDrag::~KoDrag()
{
    delete d;
}

bool KoDrag::setSvg(const QList<KoShape *> originalShapes)
{
    QRectF boundingRect;
    QList<KoShape*> shapes;

    Q_FOREACH (KoShape *shape, originalShapes) {
        boundingRect |= shape->boundingRect();

        KoShape *clonedShape = shape->cloneShape();

        /**
         * The shape is cloned without its parent's transformation, so we should
         * adjust it manually.
         */
        KoShape *oldParentShape = shape->parent();
        if (oldParentShape) {
            clonedShape->applyTransformation(oldParentShape->absoluteTransformation());
        }

        shapes.append(clonedShape);
    }

    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    QBuffer buffer;
    QLatin1String mimeType("image/svg+xml");

    buffer.open(QIODevice::WriteOnly);

    const QSizeF pageSize(boundingRect.right(), boundingRect.bottom());
    SvgWriter writer(shapes);
    writer.save(buffer, pageSize);

    buffer.close();

    qDeleteAll(shapes);

    setData(mimeType, buffer.data());
    return true;
}

void KoDrag::setData(const QString &mimeType, const QByteArray &data)
{
    if (d->mimeData == 0) {
        d->mimeData = new QMimeData();
    }
    d->mimeData->setData(mimeType, data);
}

void KoDrag::addToClipboard()
{
    if (d->mimeData) {
        QApplication::clipboard()->setMimeData(d->mimeData);
        d->mimeData = 0;
    }
}

QMimeData * KoDrag::mimeData()
{
    QMimeData *mimeData = d->mimeData;
    d->mimeData = 0;
    return mimeData;
}
