/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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
#include "KoShapeContainer.h"
#include "KoShapeContainer_p.h"
#include "KoShapeContainerModel.h"
#include "KoShapeBorderModel.h"
#include "KoShapeContainerDefaultModel.h"

#include <QPointF>
#include <QPainter>
#include <QPainterPath>

KoShapeContainerPrivate::KoShapeContainerPrivate(KoShapeContainer *q)
    : KoShapePrivate(q),
    children(0)
{
}

KoShapeContainerPrivate::~KoShapeContainerPrivate()
{
    delete children;
}

KoShapeContainer::KoShapeContainer()
    : KoShape(*(new KoShapeContainerPrivate(this)))
{
}

KoShapeContainer::KoShapeContainer(KoShapeContainerModel *model)
        : KoShape(*(new KoShapeContainerPrivate(this)))
{
    Q_D(KoShapeContainer);
    d->children = model;
}

KoShapeContainer::KoShapeContainer(KoShapeContainerPrivate &dd)
    : KoShape(dd)
{
}

KoShapeContainer::~KoShapeContainer()
{
    Q_D(KoShapeContainer);
    if (d->children) {
        foreach(KoShape *shape, d->children->childShapes())
            shape->setParent(0);
    }
}

void KoShapeContainer::addChild(KoShape *shape)
{
    Q_D(KoShapeContainer);
    Q_ASSERT(shape);
    if (shape->parent() == this && childShapes().contains(shape))
        return;
    if (d->children == 0)
        d->children = new KoShapeContainerDefaultModel();
    if (shape->parent() && shape->parent() != this)
        shape->parent()->removeChild(shape);
    d->children->add(shape);
    shape->setParent(this);
    childCountChanged();
}

void KoShapeContainer::removeChild(KoShape *shape)
{
    Q_D(KoShapeContainer);
    Q_ASSERT(shape);
    if (d->children == 0)
        return;
    d->children->remove(shape);
    shape->setParent(0);
    childCountChanged();

    KoShapeContainer * grandparent = parent();
    if (grandparent) {
        grandparent->model()->childChanged(this, KoShape::ChildChanged);
    }
}

int  KoShapeContainer::childCount() const
{
    Q_D(const KoShapeContainer);
    if (d->children == 0)
        return 0;
    return d->children->count();
}

bool KoShapeContainer::isChildLocked(const KoShape *child) const
{
    Q_D(const KoShapeContainer);
    if (d->children == 0)
        return false;
    return d->children->isChildLocked(child);
}

void KoShapeContainer::setClipping(const KoShape *child, bool clipping)
{
    Q_D(KoShapeContainer);
    if (d->children == 0)
        return;
    d->children->setClipping(child, clipping);
}

void KoShapeContainer::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_D(KoShapeContainer);
    painter.save();
    paintComponent(painter, converter);
    painter.restore();
    if (d->children == 0 || d->children->count() == 0)
        return;

    QList<KoShape*> sortedObjects = d->children->childShapes();
    qSort(sortedObjects.begin(), sortedObjects.end(), KoShape::compareShapeZIndex);

    // Do the following to revert the absolute transformation of the container
    // that is re-applied in shape->absoluteTransformation() later on. The transformation matrix
    // of the container has already been applied once before this function is called.
    QMatrix baseMatrix = absoluteTransformation(&converter).inverted() * painter.matrix();

    // clip the children to the parent outline.
    QMatrix m;
    qreal zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    m.scale(zoomX, zoomY);
    painter.setClipPath(m.map(outline()));

    foreach(KoShape *shape, sortedObjects) {
        //kDebug(30006) <<"KoShapeContainer::painting shape:" << shape->shapeId() <<"," << shape->boundingRect();
        if (! shape->isVisible())
            continue;
        if (! childClipped(shape))  // the shapeManager will have to draw those, or else we can't do clipRects
            continue;
        if (painter.hasClipping()) {
            QRectF rect = converter.viewToDocument(painter.clipRegion().boundingRect());
            rect = matrix().mapRect(rect);
            // don't try to draw a child shape that is not in the clipping rect of the painter.
            if (! rect.intersects(shape->boundingRect()))
                continue;
        }

        painter.save();
        painter.setMatrix(shape->absoluteTransformation(&converter) * baseMatrix);
        shape->paint(painter, converter);
        painter.restore();
        if (shape->border()) {
            painter.save();
            painter.setMatrix(shape->absoluteTransformation(&converter) * baseMatrix);
            shape->border()->paint(shape, painter, converter);
            painter.restore();
        }
    }
}

void KoShapeContainer::shapeChanged(ChangeType type, KoShape *shape)
{
    Q_D(KoShapeContainer);
    Q_UNUSED(shape);
    if (d->children == 0)
        return;
    if (!(type == RotationChanged || type == ScaleChanged || type == ShearChanged
            || type == SizeChanged || type == PositionChanged))
        return;
    d->children->containerChanged(this);
    foreach(KoShape *shape, d->children->childShapes())
        shape->notifyChanged();
}

bool KoShapeContainer::childClipped(const KoShape *child) const
{
    Q_D(const KoShapeContainer);
    if (d->children == 0) // throw exception??
        return false;
    return d->children->childClipped(child);
}

void KoShapeContainer::update() const
{
    Q_D(const KoShapeContainer);
    KoShape::update();
    if (d->children)
        foreach(KoShape *shape, d->children->childShapes())
            shape->update();
}

QList<KoShape*> KoShapeContainer::childShapes() const
{
    Q_D(const KoShapeContainer);
    if (d->children == 0)
        return QList<KoShape*>();

    return d->children->childShapes();
}

KoShapeContainerModel *KoShapeContainer::model() const
{
    Q_D(const KoShapeContainer);
    return d->children;
}

