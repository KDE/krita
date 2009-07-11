/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>

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

#include "KoShapeManager.h"
#include "KoSelection.h"
#include "KoToolManager.h"
#include "KoPointerEvent.h"
#include "KoShape.h"
#include "KoCanvasBase.h"
#include "KoShapeContainer.h"
#include "KoShapeBorderModel.h"
#include "KoShapeGroup.h"
#include "KoToolProxy.h"
#include "KoShapeManagerPaintingStrategy.h"
#include "KoShapeShadow.h"
#include "KoShapeLayer.h"
#include "KoFilterEffect.h"

#include <KoRTree.h>

#include <QPainter>
#include <kdebug.h>

class KoShapeManager::Private
{
public:
    Private(KoShapeManager * shapeManager, KoCanvasBase *c)
            : selection(new KoSelection())
            , canvas(c)
            , tree(4, 2)
            , strategy(new KoShapeManagerPaintingStrategy(shapeManager)) {
    }

    ~Private() {
        delete selection;
        delete strategy;
    }

    QList<KoShape *> shapes;
    QList<KoShape *> additionalShapes; // these are shapes that are only handled for updates
    KoSelection * selection;
    KoCanvasBase * canvas;
    KoRTree<KoShape *> tree;
    QSet<KoShape *> aggregate4update;
    QHash<KoShape*, int> shapeIndexesBeforeUpdate;
    KoShapeManagerPaintingStrategy * strategy;
};

KoShapeManager::KoShapeManager(KoCanvasBase *canvas, const QList<KoShape *> &shapes)
        : d(new Private(this, canvas))
{
    Q_ASSERT(d->canvas); // not optional.
    connect(d->selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
    setShapes(shapes);
}

KoShapeManager::KoShapeManager(KoCanvasBase *canvas)
        : d(new Private(this, canvas))
{
    Q_ASSERT(d->canvas); // not optional.
    connect(d->selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
}

KoShapeManager::~KoShapeManager()
{
    foreach(KoShape *shape, d->shapes) {
        shape->removeShapeManager(this);
    }
    foreach(KoShape *shape, d->additionalShapes) {
        shape->removeShapeManager(this);
    }
    delete d;
}


void KoShapeManager::setShapes(const QList<KoShape *> &shapes, Repaint repaint)
{
    //clear selection
    d->selection->deselectAll();
    foreach(KoShape *shape, d->shapes) {
        shape->removeShapeManager(this);
    }
    d->aggregate4update.clear();
    d->tree.clear();
    d->shapes.clear();
    foreach(KoShape *shape, shapes) {
        add(shape, repaint);
    }
}

void KoShapeManager::add(KoShape *shape,  Repaint repaint)
{
    if (d->shapes.contains(shape))
        return;
    shape->addShapeManager(this);
    d->shapes.append(shape);
    if (! dynamic_cast<KoShapeGroup*>(shape) && ! dynamic_cast<KoShapeLayer*>(shape)) {
        QRectF br(shape->boundingRect());
        d->tree.insert(br, shape);
    }
    if (repaint == PaintShapeOnAdd) {
        shape->update();
    }

    // add the children of a KoShapeContainer
    KoShapeContainer* container = dynamic_cast<KoShapeContainer*>(shape);

    if (container) {
        foreach(KoShape* containerShape, container->childShapes()) {
            add(containerShape, repaint);
        }
    }
}

void KoShapeManager::addAdditional(KoShape *shape)
{
    if ( shape ) {
        if (d->additionalShapes.contains(shape)) {
            return;
        }
        shape->addShapeManager(this);
        d->additionalShapes.append(shape);
    }
}

void KoShapeManager::remove(KoShape *shape)
{
    shape->update();
    shape->removeShapeManager(this);
    d->selection->deselect(shape);
    d->aggregate4update.remove(shape);
    d->tree.remove(shape);
    d->shapes.removeAll(shape);

    // remove the children of a KoShapeContainer
    KoShapeContainer* container = dynamic_cast<KoShapeContainer*>(shape);

    if (container) {
        foreach(KoShape* containerShape, container->childShapes()) {
            remove(containerShape);
        }
    }
}

void KoShapeManager::removeAdditional(KoShape *shape)
{
    if ( shape ) {
        shape->removeShapeManager(this);
        d->additionalShapes.removeAll(shape);
    }
}

void KoShapeManager::paint(QPainter &painter, const KoViewConverter &converter, bool forPrint)
{
    updateTree();
    painter.setPen(Qt::NoPen);  // painters by default have a black stroke, lets turn that off.
    painter.setBrush(Qt::NoBrush);

    QList<KoShape*> unsortedShapes;
    if (painter.hasClipping()) {
        QRectF rect = converter.viewToDocument(painter.clipRegion().boundingRect());
        unsortedShapes = d->tree.intersects(rect);
    } else {
        unsortedShapes = shapes();
        kWarning() << "KoShapeManager::paint  Painting with a painter that has no clipping will lead to too much being painted!\n";
    }

    // filter all hidden shapes from the list
    QList<KoShape*> sortedShapes;
    foreach(KoShape * shape, unsortedShapes) {
        if (shape->isVisible(true))
            sortedShapes.append(shape);
    }

    qSort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    foreach(KoShape * shape, sortedShapes) {
        if (shape->parent() != 0 && shape->parent()->childClipped(shape))
            continue;

        d->strategy->paint(shape, painter, converter, forPrint);
    }

#ifdef KOFFICE_RTREE_DEBUG
    // paint tree
    qreal zx = 0;
    qreal zy = 0;
    converter.zoom(&zx, &zy);
    painter.save();
    painter.scale(zx, zy);
    d->tree.paint(painter);
    painter.restore();
#endif

    if (! forPrint)
        d->selection->paint(painter, converter);
}

void KoShapeManager::paintShape(KoShape * shape, QPainter &painter, const KoViewConverter &converter, bool forPrint)
{
    if (shape->shadow()) {
        painter.save();
        shape->shadow()->paint(shape, painter, converter);
        painter.restore();
    }
    if(shape->filterEffectStack().empty()) {
        painter.save();
        shape->paint(painter, converter);
        painter.restore();
        if (shape->border()) {
            painter.save();
            shape->border()->paintBorder(shape, painter, converter);
            painter.restore();
        }
    } else {
        // There are filter effets, then we need to prerender the shape on an image, to filter it

        // First step, compute the rectangle used for the image
        QRectF clipRegion;
        foreach(KoFilterEffect* filterEffect, shape->filterEffectStack()) {
            clipRegion |= filterEffect->clipRect();
        }
        clipRegion = converter.documentToView(clipRegion);
        // determine the offset of the clipping rect from the shapes origin
        QPointF clippingOffset = clipRegion.topLeft();
        
        // Init the buffer image
        QImage image(clipRegion.size().toSize(), QImage::Format_ARGB32_Premultiplied);
        image.fill(qRgba(0,0,0,0));

        // Init the buffer painter
        QPainter imagePainter(&image);
        imagePainter.translate(-1.0f*clippingOffset);
        imagePainter.setPen(Qt::NoPen);
        imagePainter.setBrush(Qt::NoBrush);
        
        // Paint the shape on the image
        imagePainter.save();
        shape->paint(imagePainter, converter);
        imagePainter.restore();
        if (shape->border()) {
            imagePainter.save();
            shape->border()->paintBorder(shape, imagePainter, converter);
            imagePainter.restore();
        }
        imagePainter.end();

        // Filter
        foreach(KoFilterEffect* filterEffect, shape->filterEffectStack()) {
            QRectF filterRegion = converter.documentToView(filterEffect->filterRect());
            QPointF filterOffset = filterRegion.topLeft()-2*clippingOffset;
            QRect subRegion = filterRegion.translated(filterOffset).toRect();
            filterEffect->processImage(image, subRegion, converter);
        }

        // Paint the result
        painter.save();
        painter.drawImage(clippingOffset, image);
        painter.restore();
    }
    if (! forPrint) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, false);
        shape->paintDecorations(painter, converter, d->canvas);
        painter.restore();
    }
}


KoShape * KoShapeManager::shapeAt(const QPointF &position, KoFlake::ShapeSelection selection, bool omitHiddenShapes)
{
    updateTree();
    QList<KoShape*> sortedShapes(d->tree.contains(position));
    qSort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);
    KoShape *firstUnselectedShape = 0;
    for (int count = sortedShapes.count() - 1; count >= 0; count--) {
        KoShape *shape = sortedShapes.at(count);
        if (omitHiddenShapes && ! shape->isVisible(true))
            continue;
        if (! shape->hitTest(position))
            continue;

        switch (selection) {
        case KoFlake::ShapeOnTop:
            return shape;
        case KoFlake::Selected:
            if (d->selection->isSelected(shape))
                return shape;
            break;
        case KoFlake::Unselected:
            if (! d->selection->isSelected(shape))
                return shape;
            break;
        case KoFlake::NextUnselected:
            // we want an unselected shape
            if (d->selection->isSelected(shape))
                continue;
            // memorize the first unselected shape
            if (! firstUnselectedShape)
                firstUnselectedShape = shape;
            // check if the shape above is selected
            if (count + 1 < sortedShapes.count() && d->selection->isSelected(sortedShapes.at(count + 1)))
                return shape;
            break;
        }
    }
    // if we want the next unselected below a selected but there was none selected,
    // return the first found unselected shape
    if (selection == KoFlake::NextUnselected && firstUnselectedShape)
        return firstUnselectedShape;

    if (d->selection->hitTest(position))
        return d->selection;

    return 0; // missed everything
}

QList<KoShape *> KoShapeManager::shapesAt(const QRectF &rect, bool omitHiddenShapes)
{
    updateTree();

    QList<KoShape*> intersectedShapes(d->tree.intersects(rect));
    for (int count = intersectedShapes.count() - 1; count >= 0; count--) {
        KoShape *shape = intersectedShapes.at(count);
        if ( omitHiddenShapes && ! shape->isVisible(true)) {
            intersectedShapes.removeAt(count);
        }
        else {
            const QPainterPath outline = shape->absoluteTransformation(0).map(shape->outline());
            if( ! outline.intersects( rect ) && ! outline.contains( rect ) ) {
                intersectedShapes.removeAt(count);
            }
        }
    }
    return intersectedShapes;
}

void KoShapeManager::update(QRectF &rect, const KoShape *shape, bool selectionHandles)
{
    d->canvas->updateCanvas(rect);
    if (selectionHandles && d->selection->isSelected(shape)) {
        if (d->canvas->toolProxy())
            d->canvas->toolProxy()->repaintDecorations();
    }
}

void KoShapeManager::notifyShapeChanged(KoShape * shape)
{
    Q_ASSERT(shape);
    if (d->aggregate4update.contains(shape) || d->additionalShapes.contains(shape)) {
        return;
    }
    d->aggregate4update.insert(shape);
    d->shapeIndexesBeforeUpdate.insert(shape, shape->zIndex());

    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
    if (container) {
        foreach(KoShape *child, container->childShapes())
            d->aggregate4update.insert(child);
    }
}

void KoShapeManager::updateTree()
{
    // for detecting collisions between shapes.
    class DetectCollision
    {
    public:
        DetectCollision() {}
        void detect(KoRTree<KoShape *> &tree, KoShape *s, int prevZIndex) {
            foreach(KoShape *shape, tree.intersects(s->boundingRect())) {
                bool isChild = false;
                KoShapeContainer *parent = s->parent();
                while (parent && !isChild) {
                    if (parent == shape)
                        isChild = true;
                    parent = parent->parent();
                }
                if (isChild)
                    continue;
                if (s->zIndex() <= shape->zIndex() && prevZIndex <= shape->zIndex())
                    // Moving a shape will only make it collide with shapes below it.
                    continue;
                if (shape->collisionDetection() && !shapesWithCollisionDetection.contains(shape))
                    shapesWithCollisionDetection.append(shape);
            }
        }

        void fireSignals() {
            foreach(KoShape *shape, shapesWithCollisionDetection)
                shape->shapeChanged(KoShape::CollisionDetected);
        }

    private:
        QList<KoShape*> shapesWithCollisionDetection;
    };
    DetectCollision detector;
    bool selectionModified = false;
    foreach(KoShape *shape, d->aggregate4update) {
        if (d->shapeIndexesBeforeUpdate.contains(shape))
            detector.detect(d->tree, shape, d->shapeIndexesBeforeUpdate[shape]);
        selectionModified = selectionModified || d->selection->isSelected(shape);
    }

    foreach(KoShape * shape, d->aggregate4update) {
        d->tree.remove(shape);
        QRectF br(shape->boundingRect());
        d->strategy->adapt(shape, br);
        d->tree.insert(br, shape);
    }

    // do it again to see which shapes we intersect with _after_ moving.
    foreach(KoShape *shape, d->aggregate4update)
        detector.detect(d->tree, shape, d->shapeIndexesBeforeUpdate[shape]);
    d->aggregate4update.clear();
    d->shapeIndexesBeforeUpdate.clear();

    detector.fireSignals();
    if (selectionModified) {
        d->selection->updateSizeAndPosition();
        emit selectionContentChanged();
    }
}

QList<KoShape*> KoShapeManager::shapes() const
{
    return d->shapes;
}

KoSelection * KoShapeManager::selection() const
{
    return d->selection;
}

void KoShapeManager::suggestChangeTool(KoPointerEvent *event)
{
    QList<KoShape*> shapes;

    KoShape *clicked = shapeAt(event->point);
    if (clicked) {
        if (! selection()->isSelected(clicked)) {
            selection()->deselectAll();
            selection()->select(clicked);
        }
        shapes.append(clicked);
    }
    KoToolManager::instance()->switchToolRequested(
        KoToolManager::instance()->preferredToolForSelection(shapes));
}

void KoShapeManager::setPaintingStrategy(KoShapeManagerPaintingStrategy * strategy)
{
    delete d->strategy;
    d->strategy = strategy;
}

#include "KoShapeManager.moc"
