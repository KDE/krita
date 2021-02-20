/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007-2008 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2009 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoDrag.h"

#include <QApplication>
#include <QBuffer>
#include <QByteArray>
#include <QClipboard>
#include <QMimeData>
#include <QString>
#include <QTransform>

#include <FlakeDebug.h>

#include <KoStore.h>
#include <KoXmlWriter.h>
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
            clonedShape->applyAbsoluteTransformation(oldParentShape->absoluteTransformation());
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
