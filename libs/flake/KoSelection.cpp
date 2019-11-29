/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2006-2007,2009 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoSelection.h"
#include "KoSelection_p.h"
#include "KoShapeContainer.h"
#include "KoShapeGroup.h"
#include "KoPointerEvent.h"
#include "KoShapePaintingContext.h"
#include "kis_algebra_2d.h"
#include "krita_container_utils.h"

#include <QPainter>

#include "kis_debug.h"
KoSelection::KoSelection(QObject *parent)
    : QObject(parent)
    , KoShape()
    , d(new Private)
{
    connect(&d->selectionChangedCompressor, SIGNAL(timeout()), SIGNAL(selectionChanged()));
}

KoSelection::KoSelection(const KoSelection &rhs)
    : QObject()
    , KoShape(rhs)
    , d(rhs.d)
{
}

KoSelection::~KoSelection()
{
}

void KoSelection::paint(QPainter &painter, KoShapePaintingContext &paintcontext)
{
    Q_UNUSED(painter);
    Q_UNUSED(paintcontext);
}

void KoSelection::setSize(const QSizeF &size)
{
    Q_UNUSED(size);
    qWarning() << "WARNING: KoSelection::setSize() should never be used!";
}

QSizeF KoSelection::size() const
{
    return outlineRect().size();
}

QRectF KoSelection::outlineRect() const
{
    QPolygonF globalPolygon;
    Q_FOREACH (KoShape *shape, selectedVisibleShapes()) {
        globalPolygon = globalPolygon.united(
            shape->absoluteTransformation().map(QPolygonF(shape->outlineRect())));
    }
    const QPolygonF localPolygon = transformation().inverted().map(globalPolygon);

    return localPolygon.boundingRect();
}

QRectF KoSelection::boundingRect() const
{
    return KoShape::boundingRect(selectedVisibleShapes());
}

void KoSelection::select(KoShape *shape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape != this);
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape);

    if (!shape->isSelectable() || !shape->isVisible()) {
        return;
    }

    // check recursively
    if (isSelected(shape)) {
        return;
    }

    // find the topmost parent to select
    while (KoShapeGroup *parentGroup = dynamic_cast<KoShapeGroup*>(shape->parent())) {
        shape = parentGroup;
    }

    d->selectedShapes << shape;
    shape->addShapeChangeListener(this);

    if (d->selectedShapes.size() == 1) {
        setTransformation(shape->absoluteTransformation());
    } else {
        setTransformation(QTransform());
    }

    d->selectionChangedCompressor.start();
}

void KoSelection::deselect(KoShape *shape)
{
    if (!d->selectedShapes.contains(shape))
        return;

    d->selectedShapes.removeAll(shape);
    shape->removeShapeChangeListener(this);

    if (d->selectedShapes.size() == 1) {
        setTransformation(d->selectedShapes.first()->absoluteTransformation());
    }

    d->selectionChangedCompressor.start();
}

void KoSelection::deselectAll()
{

    if (d->selectedShapes.isEmpty())
        return;

    Q_FOREACH (KoShape *shape, d->selectedShapes) {
        shape->removeShapeChangeListener(this);
    }

    // reset the transformation matrix of the selection
    setTransformation(QTransform());

    d->selectedShapes.clear();
    d->selectionChangedCompressor.start();
}

int KoSelection::count() const
{
    return d->selectedShapes.size();
}

bool KoSelection::hitTest(const QPointF &position) const
{

    Q_FOREACH (KoShape *shape, d->selectedShapes) {
        if (shape->isVisible()) continue;
        if (shape->hitTest(position)) return true;
    }

    return false;
}

const QList<KoShape*> KoSelection::selectedShapes() const
{
    return d->selectedShapes;
}

const QList<KoShape *> KoSelection::selectedVisibleShapes() const
{
    QList<KoShape*> shapes = selectedShapes();

    KritaUtils::filterContainer (shapes, [](KoShape *shape) {
        return shape->isVisible();
    });

    return shapes;
}

const QList<KoShape *> KoSelection::selectedEditableShapes() const
{
    QList<KoShape*> shapes = selectedShapes();

    KritaUtils::filterContainer (shapes, [](KoShape *shape) {
        return shape->isShapeEditable();
    });

    return shapes;
}

const QList<KoShape *> KoSelection::selectedEditableShapesAndDelegates() const
{
    QList<KoShape*> shapes;
    Q_FOREACH (KoShape *shape, selectedShapes()) {
        QSet<KoShape *> delegates = shape->toolDelegates();
        if (delegates.isEmpty()) {
            shapes.append(shape);
        } else {
            Q_FOREACH (KoShape *delegatedShape, delegates) {
                shapes.append(delegatedShape);
            }
        }
    }
    return shapes;
}

bool KoSelection::isSelected(const KoShape *shape) const
{
    if (shape == this)
        return true;

    const KoShape *tmpShape = shape;
    while (tmpShape && std::find(d->selectedShapes.begin(), d->selectedShapes.end(), tmpShape) == d->selectedShapes.end()) {
        tmpShape = tmpShape->parent();
    }

    return tmpShape;
}

KoShape *KoSelection::firstSelectedShape() const
{
    return !d->selectedShapes.isEmpty() ? d->selectedShapes.first() : 0;
}

void KoSelection::setActiveLayer(KoShapeLayer *layer)
{
    d->activeLayer = layer;
    emit currentLayerChanged(layer);
}

KoShapeLayer* KoSelection::activeLayer() const
{
    return d->activeLayer;
}

void KoSelection::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    Q_UNUSED(shape);
    if (type == KoShape::Deleted) {
        deselect(shape);

        // HACK ALERT: the caller will also remove the listener, which was
        // removed in deselect(), so re-add it here
        shape->addShapeChangeListener(this);
    }
}

void KoSelection::saveOdf(KoShapeSavingContext &) const
{
}

bool KoSelection::loadOdf(const KoXmlElement &, KoShapeLoadingContext &)
{
    return true;
}
