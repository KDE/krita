/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoVectorPatternBackground.h"

#include <QTransform>
#include <KoShape.h>
#include <KoShapePainter.h>
#include <KoBakedShapeRenderer.h>

class KoVectorPatternBackground::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
    {
    }

    ~Private()
    {
        qDeleteAll(shapes);
        shapes.clear();
    }

    QList<KoShape*> shapes;
    KoFlake::CoordinateSystem referenceCoordinates =
            KoFlake::ObjectBoundingBox;
    KoFlake::CoordinateSystem contentCoordinates =
            KoFlake::UserSpaceOnUse;
    QRectF referenceRect;
    QTransform patternTransform;
};

KoVectorPatternBackground::KoVectorPatternBackground()
    : KoShapeBackground()
    , d(new Private)
{
}

KoVectorPatternBackground::~KoVectorPatternBackground()
{

}

bool KoVectorPatternBackground::compareTo(const KoShapeBackground *other) const
{
    Q_UNUSED(other);
    return false;
}

void KoVectorPatternBackground::setReferenceCoordinates(KoFlake::CoordinateSystem value)
{
    d->referenceCoordinates = value;
}

KoFlake::CoordinateSystem KoVectorPatternBackground::referenceCoordinates() const
{
    return d->referenceCoordinates;
}

void KoVectorPatternBackground::setContentCoordinates(KoFlake::CoordinateSystem value)
{
    d->contentCoordinates = value;
}

KoFlake::CoordinateSystem KoVectorPatternBackground::contentCoordinates() const
{
    return d->contentCoordinates;
}

void KoVectorPatternBackground::setReferenceRect(const QRectF &value)
{
    d->referenceRect = value;
}

QRectF KoVectorPatternBackground::referenceRect() const
{
    return d->referenceRect;
}

void KoVectorPatternBackground::setPatternTransform(const QTransform &value)
{
    d->patternTransform = value;
}

QTransform KoVectorPatternBackground::patternTransform() const
{
    return d->patternTransform;
}

void KoVectorPatternBackground::setShapes(const QList<KoShape*> value)
{
    qDeleteAll(d->shapes);
    d->shapes.clear();

    d->shapes = value;
}

QList<KoShape *> KoVectorPatternBackground::shapes() const
{
    return d->shapes;
}

void KoVectorPatternBackground::paint(QPainter &painter, KoShapePaintingContext &context_Unused, const QPainterPath &fillPath) const
{
    Q_UNUSED(context_Unused);

    const QPainterPath dstShapeOutline = fillPath;
    const QRectF dstShapeBoundingBox = dstShapeOutline.boundingRect();

    KoBakedShapeRenderer renderer(dstShapeOutline, QTransform(),
                                  QTransform(),
                                  d->referenceRect,
                                  d->contentCoordinates != KoFlake::UserSpaceOnUse,
                                  dstShapeBoundingBox,
                                  d->referenceCoordinates != KoFlake::UserSpaceOnUse,
                                  d->patternTransform);

    QPainter *patchPainter = renderer.bakeShapePainter();

    KoShapePainter p;
    p.setShapes(d->shapes);
    p.paint(*patchPainter);

    // uncomment for debug
    // renderer.patchImage().save("dd_patch_image.png");

    painter.setPen(Qt::NoPen);
    renderer.renderShape(painter);
}

bool KoVectorPatternBackground::hasTransparency() const
{
    return true;
}
