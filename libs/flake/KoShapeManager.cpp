/* This file is part of the KDE project

   Copyright (C) 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
   Copyright (C) 2009-2010 Jan Hambrecht <jaham@gmx.net>

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
#include "KoShape_p.h"
#include "KoCanvasBase.h"
#include "KoShapeContainer.h"
#include "KoShapeBorderModel.h"
#include "KoShapeGroup.h"
#include "KoToolProxy.h"
#include "KoShapeManagerPaintingStrategy.h"
#include "KoShapeShadow.h"
#include "KoShapeLayer.h"
#include "KoFilterEffect.h"
#include "KoFilterEffectStack.h"
#include "KoFilterEffectRenderContext.h"
#include "KoShapeBackground.h"
#include <KoRTree.h>

#include <QPainter>
#include <QTimer>
#include <kdebug.h>

class KoShapeManager::Private
{
public:
    Private(KoShapeManager *shapeManager, KoCanvasBase *c)
        : selection(new KoSelection()),
          canvas(c),
          tree(4, 2),
          strategy(new KoShapeManagerPaintingStrategy(shapeManager)),
          q(shapeManager)
    {
    }

    ~Private() {
        delete selection;
        delete strategy;
    }

    /**
     * Update the tree when there are shapes in m_aggregate4update. This is done so not all
     * updates to the tree are done when they are asked for but when they are needed.
     */
    void updateTree();

    /**
     * Recursively paints the given group shape to the specified painter
     * This is needed for filter effects on group shapes where the filter effect
     * applies to all the children of the group shape at once
     */
    void paintGroup(KoShapeGroup *group, QPainter &painter, const KoViewConverter &converter, bool forPrint);

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
                shape->priv()->shapeChanged(KoShape::CollisionDetected);
        }

    private:
        QList<KoShape*> shapesWithCollisionDetection;
    };

    QList<KoShape *> shapes;
    QList<KoShape *> additionalShapes; // these are shapes that are only handled for updates
    KoSelection *selection;
    KoCanvasBase *canvas;
    KoRTree<KoShape *> tree;
    QSet<KoShape *> aggregate4update;
    QHash<KoShape*, int> shapeIndexesBeforeUpdate;
    KoShapeManagerPaintingStrategy *strategy;
    KoShapeManager *q;
};

void KoShapeManager::Private::updateTree()
{
    // for detecting collisions between shapes.
    DetectCollision detector;
    bool selectionModified = false;
    foreach(KoShape *shape, aggregate4update) {
        if (shapeIndexesBeforeUpdate.contains(shape))
            detector.detect(tree, shape, shapeIndexesBeforeUpdate[shape]);
        selectionModified = selectionModified || selection->isSelected(shape);
    }

    foreach (KoShape *shape, aggregate4update) {
        tree.remove(shape);
        QRectF br(shape->boundingRect());
        strategy->adapt(shape, br);
        tree.insert(br, shape);
    }

    // do it again to see which shapes we intersect with _after_ moving.
    foreach (KoShape *shape, aggregate4update)
        detector.detect(tree, shape, shapeIndexesBeforeUpdate[shape]);
    aggregate4update.clear();
    shapeIndexesBeforeUpdate.clear();

    detector.fireSignals();
    if (selectionModified) {
        selection->updateSizeAndPosition();
        emit q->selectionContentChanged();
    }
}

void KoShapeManager::Private::paintGroup(KoShapeGroup *group, QPainter &painter, const KoViewConverter &converter, bool forPrint)
{
    QList<KoShape*> shapes = group->shapes();
    qSort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    foreach(KoShape *child, shapes) {
        // we paint recursively here, so we do not have to check recursively for visibility
        if (!child->isVisible())
            continue;
        KoShapeGroup *childGroup = dynamic_cast<KoShapeGroup*>(child);
        if (childGroup) {
            paintGroup(childGroup, painter, converter, forPrint);
        } else {
            painter.save();
            strategy->paint(child, painter, converter, forPrint);
            painter.restore();
        }
    }
}

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
        shape->priv()->removeShapeManager(this);
    }
    foreach(KoShape *shape, d->additionalShapes) {
        shape->priv()->removeShapeManager(this);
    }
    delete d;
}


void KoShapeManager::setShapes(const QList<KoShape *> &shapes, Repaint repaint)
{
    //clear selection
    d->selection->deselectAll();
    foreach(KoShape *shape, d->shapes) {
        shape->priv()->removeShapeManager(this);
    }
    d->aggregate4update.clear();
    d->tree.clear();
    d->shapes.clear();
    foreach(KoShape *shape, shapes) {
        addShape(shape, repaint);
    }
}

void KoShapeManager::addShape(KoShape *shape, Repaint repaint)
{
    if (d->shapes.contains(shape))
        return;
    shape->priv()->addShapeManager(this);
    d->shapes.append(shape);
    if (! dynamic_cast<KoShapeGroup*>(shape) && ! dynamic_cast<KoShapeLayer*>(shape)) {
        QRectF br(shape->boundingRect());
        d->tree.insert(br, shape);
    }
    if (repaint == PaintShapeOnAdd) {
        shape->update();
    }

    // add the children of a KoShapeContainer
    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);

    if (container) {
        foreach (KoShape *containerShape, container->shapes()) {
            addShape(containerShape, repaint);
        }
    }

    Private::DetectCollision detector;
    detector.detect(d->tree, shape, shape->zIndex());
    detector.fireSignals();
}

void KoShapeManager::addAdditional(KoShape *shape)
{
    if (shape) {
        if (d->additionalShapes.contains(shape)) {
            return;
        }
        shape->priv()->addShapeManager(this);
        d->additionalShapes.append(shape);
    }
}

void KoShapeManager::remove(KoShape *shape)
{
    Private::DetectCollision detector;
    detector.detect(d->tree, shape, shape->zIndex());
    detector.fireSignals();

    shape->update();
    shape->priv()->removeShapeManager(this);
    d->selection->deselect(shape);
    d->aggregate4update.remove(shape);
    d->tree.remove(shape);
    d->shapes.removeAll(shape);

    // remove the children of a KoShapeContainer
    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
    if (container) {
        foreach (KoShape *containerShape, container->shapes()) {
            remove(containerShape);
        }
    }
}

void KoShapeManager::removeAdditional(KoShape *shape)
{
    if (shape) {
        shape->priv()->removeShapeManager(this);
        d->additionalShapes.removeAll(shape);
    }
}

void KoShapeManager::paint(QPainter &painter, const KoViewConverter &converter, bool forPrint)
{
    d->updateTree();
    painter.setPen(Qt::NoPen);  // painters by default have a black stroke, lets turn that off.
    painter.setBrush(Qt::NoBrush);

    QList<KoShape*> unsortedShapes;
    if (painter.hasClipping()) {
        QRectF rect = converter.viewToDocument(painter.clipRegion().boundingRect());
        unsortedShapes = d->tree.intersects(rect);
    } else {
        unsortedShapes = shapes();
        kWarning() << "KoShapeManager::paint  Painting with a painter that has no clipping will lead to too much being painted!";
    }

    // filter all hidden shapes from the list
    // also filter shapes with a parent which has filter effects applied
    QList<KoShape*> sortedShapes;
    foreach (KoShape *shape, unsortedShapes) {
        if (!shape->isVisible(true))
            continue;
        bool addShapeToList = true;
        // check if one of the shapes ancestors have filter effects
        KoShapeContainer *parent = shape->parent();
        while (parent) {
            // parent must be part of the shape manager to be taken into account
            if (!d->shapes.contains(parent))
                break;
            if (parent->filterEffectStack() && !parent->filterEffectStack()->isEmpty()) {
                    addShapeToList = false;
                    break;
            }
            parent = parent->parent();
        }
        if (addShapeToList) {
            sortedShapes.append(shape);
        } else if (parent) {
            sortedShapes.append(parent);
        }
    }

    qSort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    foreach (KoShape *shape, sortedShapes) {
        if (shape->parent() != 0 && shape->parent()->isClipped(shape))
            continue;

        painter.save();
        d->strategy->paint(shape, painter, converter, forPrint);
        painter.restore();
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

void KoShapeManager::paintShape(KoShape *shape, QPainter &painter, const KoViewConverter &converter, bool forPrint)
{
    qreal transparency = shape->transparency(true);
    if (transparency > 0.0) {
        painter.setOpacity(1.0-transparency);
    }

    if (shape->shadow()) {
        painter.save();
        shape->shadow()->paint(shape, painter, converter);
        painter.restore();
    }
    if (!shape->filterEffectStack() || shape->filterEffectStack()->isEmpty()) {
        painter.save();
        shape->paint(painter, converter);
        painter.restore();
        if (shape->border()) {
            painter.save();
            shape->border()->paint(shape, painter, converter);
            painter.restore();
        }
    } else {
        // There are filter effects, then we need to prerender the shape on an image, to filter it
        QRectF shapeBound(QPointF(), shape->size());
        // First step, compute the rectangle used for the image
        QRectF clipRegion = shape->filterEffectStack()->clipRectForBoundingRect(shapeBound);
        // convert clip region to view coordinates
        QRectF zoomedClipRegion = converter.documentToView(clipRegion);
        // determine the offset of the clipping rect from the shapes origin
        QPointF clippingOffset = zoomedClipRegion.topLeft();

        // Initialize the buffer image
        QImage sourceGraphic(zoomedClipRegion.size().toSize(), QImage::Format_ARGB32_Premultiplied);
        sourceGraphic.fill(qRgba(0,0,0,0));

        QHash<QString, QImage> imageBuffers;

        QSet<QString> requiredStdInputs = shape->filterEffectStack()->requiredStandarsInputs();

        if (requiredStdInputs.contains("SourceGraphic") || requiredStdInputs.contains("SourceAlpha")) {
            // Init the buffer painter
            QPainter imagePainter(&sourceGraphic);
            imagePainter.translate(-1.0f*clippingOffset);
            imagePainter.setPen(Qt::NoPen);
            imagePainter.setBrush(Qt::NoBrush);
            imagePainter.setRenderHint(QPainter::Antialiasing, painter.testRenderHint(QPainter::Antialiasing));

            // Paint the shape on the image
            KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(shape);
            if (group) {
                // the childrens matrix contains the groups matrix as well
                // so we have to compensate for that before painting the children
                imagePainter.setTransform(group->absoluteTransformation(&converter).inverted(), true);
                d->paintGroup(group, imagePainter, converter, forPrint);
            } else {
                imagePainter.save();
                shape->paint(imagePainter, converter);
                imagePainter.restore();
                if (shape->border()) {
                    imagePainter.save();
                    shape->border()->paint(shape, imagePainter, converter);
                    imagePainter.restore();
                }
                imagePainter.end();
            }
        }
        if (requiredStdInputs.contains("SourceAlpha")) {
            QImage sourceAlpha = sourceGraphic;
            sourceAlpha.fill(qRgba(0,0,0,255));
            sourceAlpha.setAlphaChannel(sourceGraphic.alphaChannel());
            imageBuffers.insert("SourceAlpha", sourceAlpha);
        }
        if (requiredStdInputs.contains("FillPaint")) {
            QImage fillPaint = sourceGraphic;
            if (shape->background()) {
                QPainter fillPainter(&fillPaint);
                QPainterPath fillPath;
                fillPath.addRect(fillPaint.rect().adjusted(-1,-1,1,1));
                shape->background()->paint(fillPainter, fillPath);
            } else {
                fillPaint.fill(qRgba(0,0,0,0));
            }
            imageBuffers.insert("FillPaint", fillPaint);
        }

        imageBuffers.insert("SourceGraphic", sourceGraphic);
        imageBuffers.insert(QString(), sourceGraphic);

        KoFilterEffectRenderContext renderContext(converter);
        renderContext.setShapeBoundingBox(shapeBound);

        QImage result;
        QList<KoFilterEffect*> filterEffects = shape->filterEffectStack()->filterEffects();
        // Filter
        foreach (KoFilterEffect *filterEffect, filterEffects) {
            QRectF filterRegion = filterEffect->filterRectForBoundingRect(shapeBound);
            filterRegion = converter.documentToView(filterRegion);
            QRect subRegion = filterRegion.translated(-clippingOffset).toRect();
            // set current filter region
            renderContext.setFilterRegion(subRegion & sourceGraphic.rect());

            if (filterEffect->maximalInputCount() <= 1) {
                QList<QString> inputs = filterEffect->inputs();
                QString input = inputs.count() ? inputs.first() : QString();
                // get input image from image buffers and apply the filter effect
                QImage image = imageBuffers.value(input);
                if (!image.isNull()) {
                    result = filterEffect->processImage(imageBuffers.value(input), renderContext);
                }
            } else {
                QList<QImage> inputImages;
                foreach(const QString &input, filterEffect->inputs()) {
                    QImage image = imageBuffers.value(input);
                    if (!image.isNull())
                        inputImages.append(imageBuffers.value(input));
                }
                // apply the filter effect
                if (filterEffect->inputs().count() == inputImages.count())
                    result = filterEffect->processImages(inputImages, renderContext);
            }
            // store result of effect
            imageBuffers.insert(filterEffect->output(), result);
        }

        KoFilterEffect *lastEffect = filterEffects.last();

        // Paint the result
        painter.save();
        painter.drawImage(clippingOffset, imageBuffers.value(lastEffect->output()));
        painter.restore();
    }
    if (! forPrint) {
        painter.setRenderHint(QPainter::Antialiasing, false);
        shape->paintDecorations(painter, converter, d->canvas);
    }
}

KoShape *KoShapeManager::shapeAt(const QPointF &position, KoFlake::ShapeSelection selection, bool omitHiddenShapes)
{
    d->updateTree();
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
            if (shape->isSelectable())
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
    d->updateTree();

    QList<KoShape*> intersectedShapes(d->tree.intersects(rect));
    for (int count = intersectedShapes.count() - 1; count >= 0; count--) {
        KoShape *shape = intersectedShapes.at(count);
        if (omitHiddenShapes && ! shape->isVisible(true)) {
            intersectedShapes.removeAt(count);
        } else {
            const QPainterPath outline = shape->absoluteTransformation(0).map(shape->outline());
            if (! outline.intersects(rect) && ! outline.contains(rect)) {
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

void KoShapeManager::notifyShapeChanged(KoShape *shape)
{
    Q_ASSERT(shape);
    if (d->aggregate4update.contains(shape) || d->additionalShapes.contains(shape)) {
        return;
    }
    const bool wasEmpty = d->aggregate4update.isEmpty();
    d->aggregate4update.insert(shape);
    d->shapeIndexesBeforeUpdate.insert(shape, shape->zIndex());

    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
    if (container) {
        foreach(KoShape *child, container->shapes())
            notifyShapeChanged(child);
    }
    if (wasEmpty && !d->aggregate4update.isEmpty())
        QTimer::singleShot(100, this, SLOT(updateTree()));
}

QList<KoShape*> KoShapeManager::shapes() const
{
    return d->shapes;
}

QList<KoShape*> KoShapeManager::topLevelShapes() const
{
    QList<KoShape*> shapes;
    // get all toplevel shapes
    foreach(KoShape *shape, d->shapes) {
        if (shape->parent() == 0) {
            shapes.append(shape);
        }
    }
    return shapes;
}

KoSelection *KoShapeManager::selection() const
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

    QList<KoShape*> shapes2;
    foreach (KoShape *shape, shapes) {
        QSet<KoShape*> delegates = shape->toolDelegates();
        if (delegates.isEmpty()) {
            shapes2.append(shape);
        } else {
            foreach (KoShape *delegatedShape, delegates) {
                shapes2.append(delegatedShape);
            }
        }
    }
    KoToolManager::instance()->switchToolRequested(
        KoToolManager::instance()->preferredToolForSelection(shapes2));
}

void KoShapeManager::setPaintingStrategy(KoShapeManagerPaintingStrategy *strategy)
{
    delete d->strategy;
    d->strategy = strategy;
}

#include <KoShapeManager.moc>
