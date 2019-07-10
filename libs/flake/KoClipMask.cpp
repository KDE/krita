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

#include "KoClipMask.h"

#include <QRectF>
#include <QTransform>
#include <QPainter>
#include <QSharedData>
#include <KoShape.h>
#include "kis_algebra_2d.h"

#include <KoViewConverter.h>
#include <KoShapePainter.h>

struct Q_DECL_HIDDEN KoClipMask::Private : public QSharedData
{
    Private()
        : QSharedData()
    {}

    Private(const Private &rhs)
        : QSharedData()
        , coordinates(rhs.coordinates)
        , contentCoordinates(rhs.contentCoordinates)
        , maskRect(rhs.maskRect)
        , extraShapeTransform(rhs.extraShapeTransform)
    {
        Q_FOREACH (KoShape *shape, rhs.shapes) {
            KoShape *clonedShape = shape->cloneShape();
            KIS_ASSERT_RECOVER(clonedShape) { continue; }

            shapes << clonedShape;
        }
    }

    ~Private() {
        qDeleteAll(shapes);
        shapes.clear();
    }


    KoFlake::CoordinateSystem coordinates = KoFlake::ObjectBoundingBox;
    KoFlake::CoordinateSystem contentCoordinates = KoFlake::UserSpaceOnUse;

    QRectF maskRect = QRectF(-0.1, -0.1, 1.2, 1.2);

    QList<KoShape*> shapes;
    QTransform extraShapeTransform; // TODO: not used anymore, use direct shape transform instead

};

KoClipMask::KoClipMask()
    : m_d(new Private)
{
}

KoClipMask::~KoClipMask()
{
}

KoClipMask *KoClipMask::clone() const
{
    return new KoClipMask(*this);
}

KoFlake::CoordinateSystem KoClipMask::coordinates() const
{
    return m_d->coordinates;
}

void KoClipMask::setCoordinates(KoFlake::CoordinateSystem value)
{
    m_d->coordinates = value;
}

KoFlake::CoordinateSystem KoClipMask::contentCoordinates() const
{
    return m_d->contentCoordinates;
}

void KoClipMask::setContentCoordinates(KoFlake::CoordinateSystem value)
{
    m_d->contentCoordinates = value;
}

QRectF KoClipMask::maskRect() const
{
    return m_d->maskRect;
}

void KoClipMask::setMaskRect(const QRectF &value)
{
    m_d->maskRect = value;
}

QList<KoShape *> KoClipMask::shapes() const
{
    return m_d->shapes;
}

void KoClipMask::setShapes(const QList<KoShape *> &value)
{
    m_d->shapes = value;
}

bool KoClipMask::isEmpty() const
{
    return m_d->shapes.isEmpty();
}

void KoClipMask::setExtraShapeOffset(const QPointF &value)
{
    /**
     * TODO: when we implement source shapes sharing, please wrap the shapes
     *       into a group and apply this transform to the group instead
     */

    if (m_d->contentCoordinates == KoFlake::UserSpaceOnUse) {
        const QTransform t = QTransform::fromTranslate(value.x(), value.y());

        Q_FOREACH (KoShape *shape, m_d->shapes) {
            shape->applyAbsoluteTransformation(t);
        }
    }

    if (m_d->coordinates == KoFlake::UserSpaceOnUse) {
        m_d->maskRect.translate(value);
    }
}

void KoClipMask::drawMask(QPainter *painter, KoShape *shape)
{
    painter->save();

    QPainterPath clipPathInShapeSpace;

    if (m_d->coordinates == KoFlake::ObjectBoundingBox) {
        QTransform relativeToShape = KisAlgebra2D::mapToRect(shape->outlineRect());
        clipPathInShapeSpace.addPolygon(relativeToShape.map(m_d->maskRect));
    } else {
        clipPathInShapeSpace.addRect(m_d->maskRect);
        clipPathInShapeSpace = m_d->extraShapeTransform.map(clipPathInShapeSpace);
    }

    painter->setClipPath(clipPathInShapeSpace, Qt::IntersectClip);

    if (m_d->contentCoordinates == KoFlake::ObjectBoundingBox) {
        QTransform relativeToShape = KisAlgebra2D::mapToRect(shape->outlineRect());

        painter->setTransform(relativeToShape, true);
    } else {
        painter->setTransform(m_d->extraShapeTransform, true);
    }

    KoViewConverter converter;
    KoShapePainter p;
    p.setShapes(m_d->shapes);
    p.paint(*painter, converter);

    painter->restore();
}
