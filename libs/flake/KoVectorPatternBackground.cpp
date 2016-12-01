/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoVectorPatternBackground.h"

#include <QTransform>
#include <KoShape.h>
#include <KoShapePainter.h>
#include "KoShapeBackground_p.h"
#include <KoBakedShapeRenderer.h>
#include <KoViewConverter.h>

class KoVectorPatternBackgroundPrivate : public KoShapeBackgroundPrivate
{
public:
    KoVectorPatternBackgroundPrivate()
    {
    }

    ~KoVectorPatternBackgroundPrivate()
    {
        qDeleteAll(shapes);
        shapes.clear();
    }

    QList<KoShape*> shapes;
    KoVectorPatternBackground::CoordinateSystem referenceCoordinates =
            KoVectorPatternBackground::ObjectBoundingBox;
    KoVectorPatternBackground::CoordinateSystem contentCoordinates =
            KoVectorPatternBackground::UserSpaceOnUse;
    QRectF referenceRect;
    QTransform patternTransform;
};

KoVectorPatternBackground::KoVectorPatternBackground()
    : KoShapeBackground(*(new KoVectorPatternBackgroundPrivate()))
{
}

KoVectorPatternBackground::~KoVectorPatternBackground()
{

}

void KoVectorPatternBackground::setReferenceCoordinates(CoordinateSystem value)
{
    Q_D(KoVectorPatternBackground);
    d->referenceCoordinates = value;
}

KoVectorPatternBackground::CoordinateSystem KoVectorPatternBackground::referenceCoordinates() const
{
    Q_D(const KoVectorPatternBackground);
    return d->referenceCoordinates;
}

void KoVectorPatternBackground::setContentCoordinates(CoordinateSystem value)
{
    Q_D(KoVectorPatternBackground);
    d->contentCoordinates = value;
}

KoVectorPatternBackground::CoordinateSystem KoVectorPatternBackground::contentCoordinates() const
{
    Q_D(const KoVectorPatternBackground);
    return d->contentCoordinates;
}

void KoVectorPatternBackground::setReferenceRect(const QRectF &value)
{
    Q_D(KoVectorPatternBackground);
    d->referenceRect = value;
}

QRectF KoVectorPatternBackground::referenceRect() const
{
    Q_D(const KoVectorPatternBackground);
    return d->referenceRect;
}

void KoVectorPatternBackground::setPatternTransform(const QTransform &value)
{
    Q_D(KoVectorPatternBackground);
    d->patternTransform = value;
}

QTransform KoVectorPatternBackground::patternTransform() const
{
    Q_D(const KoVectorPatternBackground);
    return d->patternTransform;
}

void KoVectorPatternBackground::setShapes(const QList<KoShape*> value)
{
    Q_D(KoVectorPatternBackground);
    qDeleteAll(d->shapes);
    d->shapes.clear();

    d->shapes = value;
}

QList<KoShape *> KoVectorPatternBackground::shapes() const
{
    Q_D(const KoVectorPatternBackground);
    return d->shapes;
}

void KoVectorPatternBackground::paint(QPainter &painter, const KoViewConverter &converter_Unused, KoShapePaintingContext &context_Unused, const QPainterPath &fillPath) const
{
    Q_UNUSED(context_Unused);
    Q_UNUSED(converter_Unused);

    Q_D(const KoVectorPatternBackground);

    const QPainterPath dstShapeOutline = fillPath;
    const QRectF dstShapeBoundingBox = dstShapeOutline.boundingRect();

    KoBakedShapeRenderer renderer(dstShapeOutline, QTransform(),
                                  QTransform(),
                                  d->referenceRect,
                                  d->contentCoordinates != UserSpaceOnUse,
                                  dstShapeBoundingBox,
                                  d->referenceCoordinates != UserSpaceOnUse,
                                  d->patternTransform);

    QPainter *patchPainter = renderer.bakeShapePainter();

    KoViewConverter converter;
    KoShapePainter p;
    p.setShapes(d->shapes);
    p.paint(*patchPainter, converter);

    // uncomment for debug
    // renderer.patchImage().save("dd_patch_image.png");

    painter.setPen(Qt::NoPen);
    renderer.renderShape(painter);
}

bool KoVectorPatternBackground::hasTransparency() const
{
    return true;
}

void KoVectorPatternBackground::fillStyle(KoGenStyle &, KoShapeSavingContext &)
{
    // noop
}

bool KoVectorPatternBackground::loadStyle(KoOdfLoadingContext &, const QSizeF &Size)
{
    Q_UNUSED(Size);
    return true;
}

