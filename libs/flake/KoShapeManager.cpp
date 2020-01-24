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
#include "KoShapeManager_p.h"
#include "KoSelection.h"
#include "KoToolManager.h"
#include "KoPointerEvent.h"
#include "KoShape.h"
#include "KoShape_p.h"
#include "KoCanvasBase.h"
#include "KoShapeContainer.h"
#include "KoShapeStrokeModel.h"
#include "KoShapeGroup.h"
#include "KoToolProxy.h"
#include "KoShapeShadow.h"
#include "KoShapeLayer.h"
#include "KoFilterEffect.h"
#include "KoFilterEffectStack.h"
#include "KoFilterEffectRenderContext.h"
#include "KoShapeBackground.h"
#include <KoRTree.h>
#include "KoClipPath.h"
#include "KoClipMaskPainter.h"
#include "KoShapePaintingContext.h"
#include "KoViewConverter.h"
#include "KisQPainterStateSaver.h"
#include "KoSvgTextChunkShape.h"
#include "KoSvgTextShape.h"
#include <QApplication>

#include <QPainter>
#include <QTimer>
#include <FlakeDebug.h>

#include "kis_painting_tweaks.h"
#include "kis_debug.h"
#include "KisForest.h"
#include <unordered_set>


namespace {

/**
 * Returns whether the shape should be added to the RTree for collision and ROI
 * detection.
 */
inline bool shapeUsedInRenderingTree(KoShape *shape)
{
    // FIXME: make more general!

    return !dynamic_cast<KoShapeGroup*>(shape) &&
            !dynamic_cast<KoShapeLayer*>(shape) &&
            !(dynamic_cast<KoSvgTextChunkShape*>(shape) && !dynamic_cast<KoSvgTextShape*>(shape));
}

/**
 * Returns whether a shape should be added to the rendering tree because of
 * its clip mask/path or effects.
 */
inline bool shapeHasGroupEffects(KoShape *shape) {
    return shape->clipPath() ||
        (shape->filterEffectStack() && !shape->filterEffectStack()->isEmpty()) ||
        shape->clipMask();
}

/**
 * Returns true if the shape is not fully transparent
 */
inline bool shapeIsVisible(KoShape *shape) {
    return shape->isVisible(false) && shape->transparency() < 1.0;
}


/**
 * Populate \p tree with the subtree of shapes pointed by a shape \p parentShape.
 * All new shapes are added as children of \p parentIt. Please take it into account
 * that \c *parentIt might be not the same as \c parentShape, because \c parentShape
 * may be hidden from rendering.
 */
void populateRenderSubtree(KoShape *parentShape,
                           KisForest<KoShape*>::child_iterator parentIt,
                           KisForest<KoShape*> &tree,
                           std::function<bool(KoShape*)> shouldIncludeNode,
                           std::function<bool(KoShape*)> shouldEnterSubtree)
{
    KoShapeContainer *parentContainer = dynamic_cast<KoShapeContainer*>(parentShape);
    if (!parentContainer) return;

    QList<KoShape*> children = parentContainer->shapes();
    std::sort(children.begin(), children.end(), KoShape::compareShapeZIndex);

    for (auto it = children.constBegin(); it != children.constEnd(); ++it) {
        auto newParentIt = parentIt;

        if (shouldIncludeNode(*it)) {
            newParentIt = tree.insert(childEnd(parentIt), *it);
        }

        if (shouldEnterSubtree(*it)) {
            populateRenderSubtree(*it, newParentIt, tree, shouldIncludeNode, shouldEnterSubtree);
        }
    }

}

/**
 * Build a rendering tree for **leaf** nodes defined by \p leafNodes
 *
 * Sometimes we should render only a part of the layer (e.g. when we render
 * in patches). So we shouldn't render the whole graph. The problem is that
 * some of the shapes may have parents with clip paths/masks and/or effects.
 * In such a case, these parents should be also included into the rendering
 * process.
 *
 * \c buildRenderTree() builds a graph for such rendering. It includes the
 * leaf shapes themselves, and all parent shapes that have some effects affecting
 * these shapes.
 */
void buildRenderTree(QList<KoShape*> leafShapes,
                     KisForest<KoShape*> &tree)
{
    QList<KoShape*> sortedShapes = leafShapes;
    std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    std::unordered_set<KoShape*> includedShapes;

    Q_FOREACH (KoShape *shape, sortedShapes) {
        bool shouldSkipShape = !shapeIsVisible(shape);
        if (shouldSkipShape) continue;

        QVector<KoShape*> hierarchy = {shape};

        while ((shape = shape->parent())) {
            if (!shapeIsVisible(shape)) {
                shouldSkipShape = true;
                break;
            }

            if (shapeHasGroupEffects(shape)) {
                hierarchy << shape;
            }
        }

        if (shouldSkipShape) continue;

        if (includedShapes.find(hierarchy.last()) == end(includedShapes)) {
            tree.insert(childEnd(tree), hierarchy.last());
        }
        std::copy(hierarchy.begin(), hierarchy.end(),
                  std::inserter(includedShapes, end(includedShapes)));
    }

    auto shouldIncludeShape =
        [includedShapes] (KoShape *shape) {
            // included shapes are guaranteed to be visible
            return includedShapes.find(shape) != end(includedShapes);
        };

    for (auto it = childBegin(tree); it != childEnd(tree); ++it) {
        populateRenderSubtree(*it, it, tree, shouldIncludeShape, &shapeIsVisible);
    }
}

/**
 * Render the prebuilt rendering tree on \p painter
 */
void renderShapes(typename KisForest<KoShape*>::child_iterator beginIt,
                  typename KisForest<KoShape*>::child_iterator endIt,
                  QPainter &painter,
                  KoShapePaintingContext &paintContext)
{
    for (auto it = beginIt; it != endIt; ++it) {
        KoShape *shape = *it;

        KisQPainterStateSaver saver(&painter);

        if (!isEnd(parent(it))) {
            painter.setTransform(shape->transformation() * painter.transform());
        } else {
            painter.setTransform(shape->absoluteTransformation() * painter.transform());
        }

        KoClipPath::applyClipping(shape, painter);

        qreal transparency = shape->transparency(true);
        if (transparency > 0.0) {
            painter.setOpacity(1.0-transparency);
        }

        if (shape->shadow()) {
            KisQPainterStateSaver saver(&painter);
            shape->shadow()->paint(shape, painter);
        }

        QScopedPointer<KoClipMaskPainter> clipMaskPainter;
        QPainter *shapePainter = &painter;

        KoClipMask *clipMask = shape->clipMask();
        if (clipMask) {
            const QRectF bounds = painter.transform().mapRect(shape->outlineRect());

            clipMaskPainter.reset(new KoClipMaskPainter(&painter, bounds/*shape->boundingRect())*/));
            shapePainter = clipMaskPainter->shapePainter();
        }

        /**
         * We expect the shape to save/restore the painter's state itself. Such design was not
         * not always here, so we need a period of sanity checks to ensure all the shapes are
         * ported correctly.
         */
        const QTransform sanityCheckTransformSaved = shapePainter->transform();

        renderShapes(childBegin(it), childEnd(it), *shapePainter, paintContext);

        shape->paint(*shapePainter, paintContext);
        shape->paintStroke(*shapePainter, paintContext);

        KIS_SAFE_ASSERT_RECOVER(shapePainter->transform() == sanityCheckTransformSaved) {
            shapePainter->setTransform(sanityCheckTransformSaved);
        }

        if (clipMask) {
            clipMaskPainter->maskPainter()->save();

            shape->clipMask()->drawMask(clipMaskPainter->maskPainter(), shape);
            clipMaskPainter->renderOnGlobalPainter();

            clipMaskPainter->maskPainter()->restore();
        }
    }
}

}

void KoShapeManager::Private::updateTree()
{
    QMutexLocker l(&this->treeMutex);

    bool selectionModified = false;
    bool anyModified = false;
    Q_FOREACH (KoShape *shape, aggregate4update) {
        selectionModified = selectionModified || selection->isSelected(shape);
        anyModified = true;
    }

    foreach (KoShape *shape, aggregate4update) {
        if (!shapeUsedInRenderingTree(shape)) continue;

        tree.remove(shape);
        QRectF br(shape->boundingRect());
        tree.insert(br, shape);
    }

    aggregate4update.clear();
    shapeIndexesBeforeUpdate.clear();

    if (selectionModified) {
        emit q->selectionContentChanged();
    }
    if (anyModified) {
        emit q->contentChanged();
    }
}

void KoShapeManager::Private::forwardCompressedUdpate()
{
    bool shouldUpdateDecorations = false;
    QRectF scheduledUpdate;

    {
        QMutexLocker l(&shapesMutex);

        if (!compressedUpdate.isEmpty()) {
            scheduledUpdate = compressedUpdate;
            compressedUpdate = QRect();
        }

        Q_FOREACH (const KoShape *shape, compressedUpdatedShapes) {
            if (selection->isSelected(shape)) {
                shouldUpdateDecorations = true;
                break;
            }
        }
        compressedUpdatedShapes.clear();
    }

    if (shouldUpdateDecorations && canvas->toolProxy()) {
        canvas->toolProxy()->repaintDecorations();
    }
    canvas->updateCanvas(scheduledUpdate);

}

KoShapeManager::KoShapeManager(KoCanvasBase *canvas, const QList<KoShape *> &shapes)
    : d(new Private(this, canvas))
{
    Q_ASSERT(d->canvas); // not optional.
    connect(d->selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));
    setShapes(shapes);

    /**
     * Shape manager uses signal compressors with timers, therefore
     * it might handle queued signals, therefore it should belong
     * to the GUI thread.
     */
    this->moveToThread(qApp->thread());
    connect(&d->updateCompressor, SIGNAL(timeout()), this, SLOT(forwardCompressedUdpate()));
}

KoShapeManager::KoShapeManager(KoCanvasBase *canvas)
    : d(new Private(this, canvas))
{
    Q_ASSERT(d->canvas); // not optional.
    connect(d->selection, SIGNAL(selectionChanged()), this, SIGNAL(selectionChanged()));

    // see a comment in another constructor
    this->moveToThread(qApp->thread());
    connect(&d->updateCompressor, SIGNAL(timeout()), this, SLOT(forwardCompressedUdpate()));
}

void KoShapeManager::Private::unlinkFromShapesRecursively(const QList<KoShape*> &shapes)
{
    Q_FOREACH (KoShape *shape, shapes) {
        shape->removeShapeManager(q);

        KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
        if (container) {
            unlinkFromShapesRecursively(container->shapes());
        }
    }
}

KoShapeManager::~KoShapeManager()
{
    d->unlinkFromShapesRecursively(d->shapes);
    d->shapes.clear();

    delete d;
}

void KoShapeManager::setShapes(const QList<KoShape *> &shapes, Repaint repaint)
{
    {
        QMutexLocker l1(&d->shapesMutex);
        QMutexLocker l2(&d->treeMutex);

        //clear selection
        d->selection->deselectAll();
        d->unlinkFromShapesRecursively(d->shapes);
        d->compressedUpdate = QRect();
        d->compressedUpdatedShapes.clear();
        d->aggregate4update.clear();
        d->shapeIndexesBeforeUpdate.clear();
        d->tree.clear();
        d->shapes.clear();
    }

    Q_FOREACH (KoShape *shape, shapes) {
        addShape(shape, repaint);
    }
}

void KoShapeManager::addShape(KoShape *shape, Repaint repaint)
{
    {
        QMutexLocker l1(&d->shapesMutex);

        if (d->shapes.contains(shape))
            return;
        shape->addShapeManager(this);
        d->shapes.append(shape);

        if (shapeUsedInRenderingTree(shape)) {
            QMutexLocker l2(&d->treeMutex);

            QRectF br(shape->boundingRect());
            d->tree.insert(br, shape);
        }
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
}

void KoShapeManager::remove(KoShape *shape)
{
    QRectF dirtyRect;
    {
        QMutexLocker l1(&d->shapesMutex);
        QMutexLocker l2(&d->treeMutex);

        dirtyRect = shape->absoluteOutlineRect();

        shape->removeShapeManager(this);
        d->selection->deselect(shape);
        d->aggregate4update.remove(shape);
        d->compressedUpdatedShapes.remove(shape);

        if (shapeUsedInRenderingTree(shape)) {
            d->tree.remove(shape);
        }
        d->shapes.removeAll(shape);
    }

    if (!dirtyRect.isEmpty()) {
        d->canvas->updateCanvas(dirtyRect);
    }

    // remove the children of a KoShapeContainer
    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
    if (container) {
        foreach (KoShape *containerShape, container->shapes()) {
            remove(containerShape);
        }
    }
}

KoShapeManager::ShapeInterface::ShapeInterface(KoShapeManager *_q)
    : q(_q)
{
}

void KoShapeManager::ShapeInterface::notifyShapeDestructed(KoShape *shape)
{
    QMutexLocker l1(&q->d->shapesMutex);
    QMutexLocker l2(&q->d->treeMutex);

    q->d->selection->deselect(shape);
    q->d->aggregate4update.remove(shape);
    q->d->compressedUpdatedShapes.remove(shape);

    // we cannot access RTTI of the semi-destructed shape, so just
    // unlink it lazily
    if (q->d->tree.contains(shape)) {
        q->d->tree.remove(shape);
    }

    q->d->shapes.removeAll(shape);
}


KoShapeManager::ShapeInterface *KoShapeManager::shapeInterface()
{
    return &d->shapeInterface;
}

void KoShapeManager::preparePaintJobs(PaintJobsList &jobs,
                                      KoShape *excludeRoot)
{
    QMutexLocker l1(&d->shapesMutex);
    d->updateTree();

    QSet<KoShape*> rootShapesSet;
    Q_FOREACH (KoShape *shape, d->shapes) {
        while (shape->parent() && shape->parent() != excludeRoot) {
            shape = shape->parent();
        }

        if (!rootShapesSet.contains(shape) && shape != excludeRoot) {
            rootShapesSet.insert(shape);
        }
    }

    const QList<KoShape*> rootShapes = rootShapesSet.toList();

    QList<KoShape*> newRootShapes;

    Q_FOREACH (KoShape *srcShape, rootShapes) {
        newRootShapes << srcShape->cloneShape();
    }

    PaintJobsList result;

    PaintJob::SharedSafeStorage shapesStorage = std::make_shared<PaintJob::ShapesStorage>();
    Q_FOREACH (KoShape *shape, newRootShapes) {
        shapesStorage->emplace_back(std::unique_ptr<KoShape>(shape));
    }

    const QList<KoShape*> originalShapes = KoShape::linearizeSubtreeSorted(rootShapes);
    const QList<KoShape*> clonedShapes = KoShape::linearizeSubtreeSorted(newRootShapes);
    KIS_SAFE_ASSERT_RECOVER_RETURN(clonedShapes.size() == originalShapes.size());

    QHash<KoShape*, KoShape*> clonedFromOriginal;
    for (int i = 0; i < originalShapes.size(); i++) {
        clonedFromOriginal[originalShapes[i]] = clonedShapes[i];
    }


    for (auto it = std::begin(jobs); it != std::end(jobs); ++it) {
        QMutexLocker l(&d->treeMutex);
        QList<KoShape*> unsortedOriginalShapes = d->tree.intersects(it->docUpdateRect);

        it->allClonedShapes = shapesStorage;

        Q_FOREACH (KoShape *shape, unsortedOriginalShapes) {
            KIS_SAFE_ASSERT_RECOVER(shapeUsedInRenderingTree(shape)) { continue; }
            it->shapes << clonedFromOriginal[shape];
        }
    }
}

void KoShapeManager::paintJob(QPainter &painter, const KoShapeManager::PaintJob &job, bool forPrint)
{
    painter.setPen(Qt::NoPen);  // painters by default have a black stroke, lets turn that off.
    painter.setBrush(Qt::NoBrush);

    KisForest<KoShape*> renderTree;
    buildRenderTree(job.shapes, renderTree);

    KoShapePaintingContext paintContext(d->canvas, forPrint); //FIXME
    renderShapes(childBegin(renderTree), childEnd(renderTree), painter, paintContext);
}

void KoShapeManager::paint(QPainter &painter, bool forPrint)
{
    QMutexLocker l1(&d->shapesMutex);

    d->updateTree();
    painter.setPen(Qt::NoPen);  // painters by default have a black stroke, lets turn that off.
    painter.setBrush(Qt::NoBrush);

    QList<KoShape*> unsortedShapes;
    if (painter.hasClipping()) {
        QMutexLocker l(&d->treeMutex);

        QRectF rect = KisPaintingTweaks::safeClipBoundingRect(painter);
        unsortedShapes = d->tree.intersects(rect);
    } else {
        unsortedShapes = d->shapes;
        warnFlake << "KoShapeManager::paint  Painting with a painter that has no clipping will lead to too much being painted!";
    }

    KoShapePaintingContext paintContext(d->canvas, forPrint); //FIXME

    KisForest<KoShape*> renderTree;
    buildRenderTree(unsortedShapes, renderTree);
    renderShapes(childBegin(renderTree), childEnd(renderTree), painter, paintContext);
}

void KoShapeManager::renderSingleShape(KoShape *shape, QPainter &painter, KoShapePaintingContext &paintContext)
{
    KisForest<KoShape*> renderTree;

    KoViewConverter converter;

    auto root = renderTree.insert(childBegin(renderTree), shape);
    populateRenderSubtree(shape, root, renderTree, &shapeIsVisible, &shapeIsVisible);
    renderShapes(childBegin(renderTree), childEnd(renderTree), painter, paintContext);
}

KoShape *KoShapeManager::shapeAt(const QPointF &position, KoFlake::ShapeSelection selection, bool omitHiddenShapes)
{
    QMutexLocker l(&d->shapesMutex);

    d->updateTree();
    QList<KoShape*> sortedShapes;

    {
        QMutexLocker l(&d->treeMutex);
        sortedShapes = d->tree.contains(position);
    }

    std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);
    KoShape *firstUnselectedShape = 0;
    for (int count = sortedShapes.count() - 1; count >= 0; count--) {
        KoShape *shape = sortedShapes.at(count);
        if (omitHiddenShapes && ! shape->isVisible())
            continue;
        if (! shape->hitTest(position))
            continue;

        switch (selection) {
        case KoFlake::ShapeOnTop:
            if (shape->isSelectable())
                return shape;
            break;
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

QList<KoShape *> KoShapeManager::shapesAt(const QRectF &rect, bool omitHiddenShapes, bool containedMode)
{
    QMutexLocker l(&d->shapesMutex);

    d->updateTree();
    QList<KoShape*> shapes;

    {
        QMutexLocker l(&d->treeMutex);
        shapes = containedMode ? d->tree.contained(rect) : d->tree.intersects(rect);
    }

    for (int count = shapes.count() - 1; count >= 0; count--) {

        KoShape *shape = shapes.at(count);

        if (omitHiddenShapes && !shape->isVisible()) {
            shapes.removeAt(count);
        } else {
            const QPainterPath outline = shape->absoluteTransformation().map(shape->outline());

            if (!containedMode && !outline.intersects(rect) && !outline.contains(rect)) {
                shapes.removeAt(count);

            } else if (containedMode) {

                QPainterPath containingPath;
                containingPath.addRect(rect);

                if (!containingPath.contains(outline)) {
                    shapes.removeAt(count);
                }
            }
        }
    }

    return shapes;
}

void KoShapeManager::update(const QRectF &rect, const KoShape *shape, bool selectionHandles)
{
    if (d->updatesBlocked) return;

    {
        QMutexLocker l(&d->shapesMutex);

        d->compressedUpdate |= rect;

        if (selectionHandles) {
            d->compressedUpdatedShapes.insert(shape);
        }
    }

    d->updateCompressor.start();
}

void KoShapeManager::setUpdatesBlocked(bool value)
{
    d->updatesBlocked = value;
}

bool KoShapeManager::updatesBlocked() const
{
    return d->updatesBlocked;
}
void KoShapeManager::notifyShapeChanged(KoShape *shape)
{
    {
        QMutexLocker l(&d->treeMutex);

        Q_ASSERT(shape);
        if (d->aggregate4update.contains(shape)) {
            return;
        }

        d->aggregate4update.insert(shape);
        d->shapeIndexesBeforeUpdate.insert(shape, shape->zIndex());
    }

    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
    if (container) {
        Q_FOREACH (KoShape *child, container->shapes())
            notifyShapeChanged(child);
    }
}

QList<KoShape*> KoShapeManager::shapes() const
{
    QMutexLocker l(&d->shapesMutex);

    return d->shapes;
}

QList<KoShape*> KoShapeManager::topLevelShapes() const
{
    QMutexLocker l(&d->shapesMutex);

    QList<KoShape*> shapes;
    // get all toplevel shapes
    Q_FOREACH (KoShape *shape, d->shapes) {
        if (!shape->parent() || dynamic_cast<KoShapeLayer*>(shape->parent())) {
            shapes.append(shape);
        }
    }
    return shapes;
}

KoSelection *KoShapeManager::selection() const
{
    return d->selection;
}

KoCanvasBase *KoShapeManager::canvas()
{
    return d->canvas;
}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoShapeManager.cpp"
