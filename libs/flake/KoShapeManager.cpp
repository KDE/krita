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

bool KoShapeManager::Private::shapeUsedInRenderingTree(KoShape *shape)
{
    // FIXME: make more general!

    return !dynamic_cast<KoShapeGroup*>(shape) &&
            !dynamic_cast<KoShapeLayer*>(shape) &&
            !(dynamic_cast<KoSvgTextChunkShape*>(shape) && !dynamic_cast<KoSvgTextShape*>(shape));
}

void KoShapeManager::Private::updateTree()
{
    QMutexLocker l(&this->treeMutex);

    // for detecting collisions between shapes.
    DetectCollision detector;
    bool selectionModified = false;
    bool anyModified = false;
    Q_FOREACH (KoShape *shape, aggregate4update) {
        if (shapeIndexesBeforeUpdate.contains(shape))
            detector.detect(tree, shape, shapeIndexesBeforeUpdate[shape]);
        selectionModified = selectionModified || selection->isSelected(shape);
        anyModified = true;
    }

    foreach (KoShape *shape, aggregate4update) {
        if (!shapeUsedInRenderingTree(shape)) continue;

        tree.remove(shape);
        QRectF br(shape->boundingRect());
        tree.insert(br, shape);
    }

    // do it again to see which shapes we intersect with _after_ moving.
    foreach (KoShape *shape, aggregate4update) {
        detector.detect(tree, shape, shapeIndexesBeforeUpdate[shape]);
    }
    aggregate4update.clear();
    shapeIndexesBeforeUpdate.clear();

    detector.fireSignals();
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

void KoShapeManager::Private::paintGroup(KoShapeGroup *group, QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    QList<KoShape*> shapes = group->shapes();
    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
    Q_FOREACH (KoShape *child, shapes) {
        // we paint recursively here, so we do not have to check recursively for visibility
        if (!child->isVisible(false))
            continue;
        KoShapeGroup *childGroup = dynamic_cast<KoShapeGroup*>(child);
        if (childGroup) {
            paintGroup(childGroup, painter, converter, paintContext);
        } else {
            painter.save();
            KoShapeManager::renderSingleShape(child, painter, converter, paintContext);
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
        shape->priv()->removeShapeManager(q);

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
        shape->priv()->addShapeManager(this);
        d->shapes.append(shape);

        if (d->shapeUsedInRenderingTree(shape)) {
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

    {
        QMutexLocker l(&d->treeMutex);

        Private::DetectCollision detector;
        detector.detect(d->tree, shape, shape->zIndex());
        detector.fireSignals();
    }
}

void KoShapeManager::remove(KoShape *shape)
{
    QRectF dirtyRect;
    {
        QMutexLocker l1(&d->shapesMutex);
        QMutexLocker l2(&d->treeMutex);

        Private::DetectCollision detector;
        detector.detect(d->tree, shape, shape->zIndex());
        detector.fireSignals();

        dirtyRect = shape->absoluteOutlineRect();
        shape->priv()->removeShapeManager(this);

        d->selection->deselect(shape);
        d->aggregate4update.remove(shape);
        d->compressedUpdatedShapes.remove(shape);

        if (d->shapeUsedInRenderingTree(shape)) {
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


void KoShapeManager::paint(QPainter &painter, const KoViewConverter &converter, bool forPrint)
{
    QMutexLocker l1(&d->shapesMutex);

    d->updateTree();
    painter.setPen(Qt::NoPen);  // painters by default have a black stroke, lets turn that off.
    painter.setBrush(Qt::NoBrush);

    QList<KoShape*> unsortedShapes;
    if (painter.hasClipping()) {
        QMutexLocker l(&d->treeMutex);

        QRectF rect = converter.viewToDocument(KisPaintingTweaks::safeClipBoundingRect(painter));
        unsortedShapes = d->tree.intersects(rect);
    } else {
        unsortedShapes = d->shapes;
        warnFlake << "KoShapeManager::paint  Painting with a painter that has no clipping will lead to too much being painted!";
    }

    // filter all hidden shapes from the list
    // also filter shapes with a parent which has filter effects applied
    QList<KoShape*> sortedShapes;
    foreach (KoShape *shape, unsortedShapes) {
        if (!shape->isVisible())
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

    std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);

    KoShapePaintingContext paintContext(d->canvas, forPrint); //FIXME

    foreach (KoShape *shape, sortedShapes) {
        renderSingleShape(shape, painter, converter, paintContext);
    }

#ifdef CALLIGRA_RTREE_DEBUG
    // paint tree
    qreal zx = 0;
    qreal zy = 0;
    converter.zoom(&zx, &zy);
    painter.save();
    painter.scale(zx, zy);
    d->tree.paint(painter);
    painter.restore();
#endif

    if (! forPrint) {
        KoShapePaintingContext paintContext(d->canvas, forPrint); //FIXME
        d->selection->paint(painter, converter, paintContext);
    }
}

void KoShapeManager::renderSingleShape(KoShape *shape, QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
{
    KisQPainterStateSaver saver(&painter);

    // apply shape clipping
    KoClipPath::applyClipping(shape, painter, converter);

    // apply transformation
    painter.setTransform(shape->absoluteTransformation(&converter) * painter.transform());

    // paint the shape
    paintShape(shape, painter, converter, paintContext);
}

void KoShapeManager::paintShape(KoShape *shape, QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext)
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

        QScopedPointer<KoClipMaskPainter> clipMaskPainter;
        QPainter *shapePainter = &painter;

        KoClipMask *clipMask = shape->clipMask();
        if (clipMask) {
            clipMaskPainter.reset(new KoClipMaskPainter(&painter, shape->boundingRect()));
            shapePainter = clipMaskPainter->shapePainter();
        }

        /**
         * We expect the shape to save/restore the painter's state itself. Such design was not
         * not always here, so we need a period of sanity checks to ensure all the shapes are
         * ported correctly.
         */
        const QTransform sanityCheckTransformSaved = shapePainter->transform();

        shape->paint(*shapePainter, converter, paintContext);
        shape->paintStroke(*shapePainter, converter, paintContext);

        KIS_SAFE_ASSERT_RECOVER(shapePainter->transform() == sanityCheckTransformSaved) {
            shapePainter->setTransform(sanityCheckTransformSaved);
        }

        if (clipMask) {
            shape->clipMask()->drawMask(clipMaskPainter->maskPainter(), shape);
            clipMaskPainter->renderOnGlobalPainter();
        }

    } else {
        // TODO: clipping mask is not implemented for this case!

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
                Private::paintGroup(group, imagePainter, converter, paintContext);
            } else {
                imagePainter.save();
                shape->paint(imagePainter, converter, paintContext);
                shape->paintStroke(imagePainter, converter, paintContext);
                imagePainter.restore();
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
                shape->background()->paint(fillPainter, converter, paintContext, fillPath);
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
                Q_FOREACH (const QString &input, filterEffect->inputs()) {
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
            const QPainterPath outline = shape->absoluteTransformation(0).map(shape->outline());

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
    {
        QMutexLocker l(&d->shapesMutex);

        d->compressedUpdate |= rect;

        if (selectionHandles) {
            d->compressedUpdatedShapes.insert(shape);
        }
    }

    d->updateCompressor.start();
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
