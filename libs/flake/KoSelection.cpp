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

KoSelection::KoSelection()
    : KoShape(new KoSelectionPrivate(this))
{
    Q_D(KoSelection);
    connect(&d->selectionChangedCompressor, SIGNAL(timeout()), SIGNAL(selectionChanged()));
}

KoSelection::~KoSelection()
{
}

void KoSelection::paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintcontext)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
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
    Q_D(const KoSelection);

    QPolygonF globalPolygon;
    Q_FOREACH (KoShape *shape, d->selectedShapes) {
        globalPolygon = globalPolygon.united(
            shape->absoluteTransformation(0).map(QPolygonF(shape->outlineRect())));
    }
    const QPolygonF localPolygon = transformation().inverted().map(globalPolygon);

    return localPolygon.boundingRect();
}

QRectF KoSelection::boundingRect() const
{
    Q_D(const KoSelection);
    return KoShape::boundingRect(d->selectedShapes);
}

void KoSelection::select(KoShape *shape)
{
    Q_D(KoSelection);
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape != this);
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape);

    if (!shape->isSelectable() || !shape->isVisible(true)) {
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

    d->savedMatrices = d->fetchShapesMatrices();

    if (d->selectedShapes.size() == 1) {
        setTransformation(shape->absoluteTransformation(0));
    } else {
        setTransformation(QTransform());
    }

    d->selectionChangedCompressor.start();
}

void KoSelection::deselect(KoShape *shape)
{
    Q_D(KoSelection);
    if (!d->selectedShapes.contains(shape))
        return;

    d->selectedShapes.removeAll(shape);
    shape->removeShapeChangeListener(this);
    d->savedMatrices = d->fetchShapesMatrices();

    if (d->selectedShapes.size() == 1) {
        setTransformation(d->selectedShapes.first()->absoluteTransformation(0));
    }

    d->selectionChangedCompressor.start();
}

void KoSelection::deselectAll()
{
    Q_D(KoSelection);

    if (d->selectedShapes.isEmpty())
        return;

    Q_FOREACH (KoShape *shape, d->selectedShapes) {
        shape->removeShapeChangeListener(this);
    }
    d->savedMatrices = d->fetchShapesMatrices();

    // reset the transformation matrix of the selection
    setTransformation(QTransform());

    d->selectedShapes.clear();
    d->selectionChangedCompressor.start();
}

int KoSelection::count() const
{
    Q_D(const KoSelection);
    return d->selectedShapes.size();
}

bool KoSelection::hitTest(const QPointF &position) const
{
    Q_D(const KoSelection);

    Q_FOREACH (KoShape *shape, d->selectedShapes) {
        if (shape->hitTest(position)) return true;
    }

    return false;
}

const QList<KoShape*> KoSelection::selectedShapes() const
{
    Q_D(const KoSelection);
    return d->selectedShapes;
}

const QList<KoShape *> KoSelection::selectedEditableShapes() const
{
    Q_D(const KoSelection);

    QList<KoShape*> shapes = selectedShapes();

    KritaUtils::filterContainer (shapes, [](KoShape *shape) {
        return shape->isEditable();
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
    Q_D(const KoSelection);
    if (shape == this)
        return true;

    const KoShape *tmpShape = shape;
    while (tmpShape && std::find(d->selectedShapes.begin(), d->selectedShapes.end(), tmpShape) == d->selectedShapes.end()/*d->selectedShapes.contains(tmpShape)*/) {
        tmpShape = tmpShape->parent();
    }

    return tmpShape;
}

KoShape *KoSelection::firstSelectedShape() const
{
    Q_D(const KoSelection);
    return !d->selectedShapes.isEmpty() ? d->selectedShapes.first() : 0;
}

void KoSelection::setActiveLayer(KoShapeLayer *layer)
{
    Q_D(KoSelection);
    d->activeLayer = layer;
    emit currentLayerChanged(layer);
}

KoShapeLayer* KoSelection::activeLayer() const
{
    Q_D(const KoSelection);
    return d->activeLayer;
}

void KoSelection::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    Q_UNUSED(shape);
    Q_D(KoSelection);

    if (type == KoShape::Deleted) {
        deselect(shape);
        // HACK ALERT: the caller will also remove the listener, so re-add it here
        shape->addShapeChangeListener(this);

    } else if (type >= KoShape::PositionChanged && type <= KoShape::GenericMatrixChange) {
        QList<QTransform> matrices = d->fetchShapesMatrices();

        QTransform newTransform;
        if (d->checkMatricesConsistent(matrices, &newTransform)) {
            d->savedMatrices = matrices;
            setTransformation(newTransform);
        } else {
            d->savedMatrices = matrices;
            setTransformation(QTransform());
        }
    }
}

void KoSelection::saveOdf(KoShapeSavingContext &) const
{
}

bool KoSelection::loadOdf(const KoXmlElement &, KoShapeLoadingContext &)
{
    return true;
}


QList<QTransform> KoSelectionPrivate::fetchShapesMatrices() const
{
    QList<QTransform> result;
    Q_FOREACH (KoShape *shape, selectedShapes) {
        result << shape->absoluteTransformation(0);
    }
    return result;
}

bool KoSelectionPrivate::checkMatricesConsistent(const QList<QTransform> &matrices, QTransform *newTransform)
{
    Q_Q(KoSelection);

    QTransform commonDiff;
    bool haveCommonDiff = false;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(matrices.size() == selectedShapes.size(), false);

    for (int i = 0; i < matrices.size(); i++) {
        QTransform t = savedMatrices[i];
        QTransform diff = t.inverted() * matrices[i];


        if (haveCommonDiff) {
            if (!KisAlgebra2D::fuzzyMatrixCompare(commonDiff, diff, 1e-5)) {
                return false;
            }
        } else {
            commonDiff = diff;
        }
    }

    *newTransform = q->transformation() * commonDiff;
    return true;
}
