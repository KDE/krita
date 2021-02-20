/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoClipPath.h"
#include "KoPathShape.h"
#include "KoShapeGroup.h"

#include <QTransform>
#include <QPainterPath>
#include <QPainter>
#include <QVarLengthArray>
#include <QSharedData>

#include <kis_algebra_2d.h>


QTransform scaleToPercent(const QSizeF &size)
{
    const qreal w = qMax(static_cast<qreal>(1e-5), size.width());
    const qreal h = qMax(static_cast<qreal>(1e-5), size.height());
    return QTransform().scale(1.0/w, 1.0/h);
}

QTransform scaleFromPercent(const QSizeF &size)
{
    const qreal w = qMax(static_cast<qreal>(1e-5), size.width());
    const qreal h = qMax(static_cast<qreal>(1e-5), size.height());
    return QTransform().scale(w/1.0, h/1.0);
}

class Q_DECL_HIDDEN KoClipPath::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
    {}

    Private(const Private &rhs)
        : QSharedData()
        , clipPath(rhs.clipPath)
        , clipRule(rhs.clipRule)
        , coordinates(rhs.coordinates)
        , initialTransformToShape(rhs.initialTransformToShape)
        , initialShapeSize(rhs.initialShapeSize)
    {
        Q_FOREACH (KoShape *shape, rhs.shapes) {
            KoShape *clonedShape = shape->cloneShape();
            KIS_ASSERT_RECOVER(clonedShape) { continue; }

            shapes.append(clonedShape);
        }
    }

    ~Private()
    {
        qDeleteAll(shapes);
        shapes.clear();
    }

    void collectShapePath(QPainterPath *result, const KoShape *shape) {
        if (const KoPathShape *pathShape = dynamic_cast<const KoPathShape*>(shape)) {
            // different shapes add up to the final path using Windind Fill rule (acc. to SVG 1.1)
            QTransform t = pathShape->absoluteTransformation();
            result->addPath(t.map(pathShape->outline()));
        } else if (const KoShapeGroup *groupShape = dynamic_cast<const KoShapeGroup*>(shape)) {
            QList<KoShape*> shapes = groupShape->shapes();
            std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

            Q_FOREACH (const KoShape *child, shapes) {
                collectShapePath(result, child);
            }
        }
    }


    void compileClipPath()
    {
        QList<KoShape*> clipShapes = this->shapes;
        if (clipShapes.isEmpty())
            return;

        clipPath = QPainterPath();
        clipPath.setFillRule(Qt::WindingFill);

        std::sort(clipShapes.begin(), clipShapes.end(), KoShape::compareShapeZIndex);

        Q_FOREACH (KoShape *path, clipShapes) {
            if (!path) continue;

            collectShapePath(&clipPath, path);
        }
    }

    QList<KoShape*> shapes;
    QPainterPath clipPath; ///< the compiled clip path in shape coordinates of the clipped shape
    Qt::FillRule clipRule = Qt::WindingFill;
    KoFlake::CoordinateSystem coordinates = KoFlake::ObjectBoundingBox;
    QTransform initialTransformToShape; ///< initial transformation to shape coordinates of the clipped shape
    QSizeF initialShapeSize; ///< initial size of clipped shape
};

KoClipPath::KoClipPath(QList<KoShape*> clipShapes, KoFlake::CoordinateSystem coordinates)
   : d(new Private())
{
    d->shapes = clipShapes;
    d->coordinates = coordinates;
    d->compileClipPath();
}

KoClipPath::~KoClipPath()
{
}

KoClipPath *KoClipPath::clone() const
{
    return new KoClipPath(*this);
}

void KoClipPath::setClipRule(Qt::FillRule clipRule)
{
    d->clipRule = clipRule;
}

Qt::FillRule KoClipPath::clipRule() const
{
    return d->clipRule;
}

KoFlake::CoordinateSystem KoClipPath::coordinates() const
{
    return d->coordinates;
}

void KoClipPath::applyClipping(KoShape *shape, QPainter &painter)
{
    if (shape->clipPath()) {
        QPainterPath path = shape->clipPath()->path();

        if (shape->clipPath()->coordinates() == KoFlake::ObjectBoundingBox) {
            const QRectF shapeLocalBoundingRect = shape->outline().boundingRect();
            path = KisAlgebra2D::mapToRect(shapeLocalBoundingRect).map(path);
        }

        if (!path.isEmpty()) {
            painter.setClipPath(path, Qt::IntersectClip);
        }
    }
}

QPainterPath KoClipPath::path() const
{
    return d->clipPath;
}

QPainterPath KoClipPath::pathForSize(const QSizeF &size) const
{
    return scaleFromPercent(size).map(d->clipPath);
}

QList<KoPathShape*> KoClipPath::clipPathShapes() const
{
    // TODO: deprecate this method!

    QList<KoPathShape*> shapes;

    Q_FOREACH (KoShape *shape, d->shapes) {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape);
        if (pathShape) {
            shapes << pathShape;
        }
    }

    return shapes;
}

QList<KoShape *> KoClipPath::clipShapes() const
{
    return d->shapes;
}

QTransform KoClipPath::clipDataTransformation(KoShape *clippedShape) const
{
    if (!clippedShape)
        return d->initialTransformToShape;

    // the current transformation of the clipped shape
    QTransform currentShapeTransform = clippedShape->absoluteTransformation();

    // calculate the transformation which represents any resizing of the clipped shape
    const QSizeF currentShapeSize = clippedShape->outline().boundingRect().size();
    const qreal sx = currentShapeSize.width() / d->initialShapeSize.width();
    const qreal sy = currentShapeSize.height() / d->initialShapeSize.height();
    QTransform scaleTransform = QTransform().scale(sx, sy);

    // 1. transform to initial clipped shape coordinates
    // 2. apply resizing transformation
    // 3. convert to current clipped shape document coordinates
    return d->initialTransformToShape * scaleTransform * currentShapeTransform;
}
