/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#include "KoShapeContainerModel.h"
#include "KoShapeBorderModel.h"
#include "KoChildrenData.h"

#include <QPointF>
#include <QPainter>
#include <QPainterPath>

class KoShapeContainer::Private
{
public:
    Private() : children(0) {}
    ~Private() {
        if (children) {
            foreach(KoShape *shape, children->childShapes())
                shape->setParent(0);
            delete children;
        }
    }
    KoShapeContainerModel *children;
};

KoShapeContainer::KoShapeContainer() : KoShape(), d(new Private())
{
}

KoShapeContainer::KoShapeContainer(KoShapeContainerModel *model)
        : KoShape(),
        d(new Private())
{
    d->children = model;
}

KoShapeContainer::~KoShapeContainer()
{
    delete d;
}

void KoShapeContainer::addChild(KoShape *shape)
{
    Q_ASSERT(shape);
    if (shape->parent() == this && childShapes().contains(shape))
        return;
    if (d->children == 0)
        d->children = new KoChildrenData();
    if (shape->parent() && shape->parent() != this)
        shape->parent()->removeChild(shape);
    d->children->add(shape);
    shape->setParent(this);
    childCountChanged();
}

void KoShapeContainer::removeChild(KoShape *shape)
{
    Q_ASSERT(shape);
    if (d->children == 0)
        return;
    d->children->remove(shape);
    shape->setParent(0);
    childCountChanged();
}

int  KoShapeContainer::childCount() const
{
    if (d->children == 0)
        return 0;
    return d->children->count();
}

bool KoShapeContainer::isChildLocked(const KoShape *child) const
{
    if (d->children == 0)
        return false;
    return d->children->isChildLocked(child);
}

void KoShapeContainer::setClipping(const KoShape *child, bool clipping)
{
    if (d->children == 0)
        return;
    d->children->setClipping(child, clipping);
}

void KoShapeContainer::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.save();
    paintComponent(painter, converter);
    painter.restore();
    if (d->children == 0 || d->children->count() == 0)
        return;

    QList<KoShape*> sortedObjects = d->children->childShapes();
    qSort(sortedObjects.begin(), sortedObjects.end(), KoShape::compareShapeZIndex);

    QMatrix baseMatrix = absoluteTransformation(0).inverted() * painter.matrix();

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
            shape->border()->paintBorder(shape, painter, converter);
            painter.restore();
        }
    }
}

void KoShapeContainer::shapeChanged(ChangeType type, KoShape *shape)
{
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
    if (d->children == 0) // throw exception??
        return false;
    return d->children->childClipped(child);
}

void KoShapeContainer::update() const
{
    KoShape::update();
    if (d->children)
        foreach(KoShape *shape, d->children->childShapes())
            shape->update();
}

QList<KoShape*> KoShapeContainer::childShapes() const
{
    if (d->children == 0)
        return QList<KoShape*>();

    return d->children->childShapes();
}

KoShapeContainerModel *KoShapeContainer::model() const
{
    return d->children;
}

