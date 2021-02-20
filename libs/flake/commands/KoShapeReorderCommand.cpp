/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeReorderCommand.h"
#include "KoShape.h"
#include "KoShape_p.h"
#include "KoShapeManager.h"
#include "KoShapeContainer.h"

#include <klocalizedstring.h>
#include <FlakeDebug.h>
#include <limits.h>

KoShapeReorderCommand::IndexedShape::IndexedShape()
{
}

KoShapeReorderCommand::IndexedShape::IndexedShape(KoShape *_shape)
    : zIndex(_shape->zIndex()), shape(_shape)
{
}

bool KoShapeReorderCommand::IndexedShape::operator<(const KoShapeReorderCommand::IndexedShape &rhs) const
{
    return zIndex < rhs.zIndex;
}

class KoShapeReorderCommandPrivate
{
public:
    KoShapeReorderCommandPrivate() {}
    KoShapeReorderCommandPrivate(const QList<KoShape*> &s, QList<int> &ni)
        : shapes(s), newIndexes(ni)
    {
    }

    QList<KoShape*> shapes;
    QList<int> previousIndexes;
    QList<int> newIndexes;
};

KoShapeReorderCommand::KoShapeReorderCommand(const QList<KoShape*> &shapes, QList<int> &newIndexes, KUndo2Command *parent)
    : KUndo2Command(parent),
      d(new KoShapeReorderCommandPrivate(shapes, newIndexes))
{
    Q_ASSERT(shapes.count() == newIndexes.count());
    foreach (KoShape *shape, shapes)
        d->previousIndexes.append(shape->zIndex());

    setText(kundo2_i18n("Reorder shapes"));
}

KoShapeReorderCommand::KoShapeReorderCommand(const QList<IndexedShape> &shapes, KUndo2Command *parent)
    : KUndo2Command(parent),
      d(new KoShapeReorderCommandPrivate())
{
    Q_FOREACH (const IndexedShape &index, shapes) {
        d->shapes.append(index.shape);
        d->newIndexes.append(index.zIndex);
        d->previousIndexes.append(index.shape->zIndex());
    }

    setText(kundo2_i18n("Reorder shapes"));
}

KoShapeReorderCommand::~KoShapeReorderCommand()
{
    delete d;
}

void KoShapeReorderCommand::redo()
{
    KUndo2Command::redo();
    for (int i = 0; i < d->shapes.count(); i++) {
        // z-index cannot change the bounding rect of the shape, so
        // no united updates needed
        d->shapes.at(i)->setZIndex(d->newIndexes.at(i));
        d->shapes.at(i)->update();
    }
}

void KoShapeReorderCommand::undo()
{
    KUndo2Command::undo();
    for (int i = 0; i < d->shapes.count(); i++) {
        // z-index cannot change the bounding rect of the shape, so
        // no united updates needed
        d->shapes.at(i)->setZIndex(d->previousIndexes.at(i));
        d->shapes.at(i)->update();
    }
}

static void prepare(KoShape *s, QMap<KoShape*, QList<KoShape*> > &newOrder, KoShapeManager *manager, KoShapeReorderCommand::MoveShapeType move)
{
    KoShapeContainer *parent = s->parent();
    QMap<KoShape*, QList<KoShape*> >::iterator it(newOrder.find(parent));
    if (it == newOrder.end()) {
        QList<KoShape*> children;
        if (parent != 0) {
            children = parent->shapes();
        }
        else {
            // get all toplevel shapes
            children = manager->topLevelShapes();
        }
        std::sort(children.begin(), children.end(), KoShape::compareShapeZIndex);
        // the append and prepend are needed so that the raise/lower of all shapes works as expected.
        children.append(0);
        children.prepend(0);
        it = newOrder.insert(parent, children);
    }
    QList<KoShape *> & shapes(newOrder[parent]);
    int index = shapes.indexOf(s);
    if (index != -1) {
        shapes.removeAt(index);
        switch (move) {
        case KoShapeReorderCommand::BringToFront:
            index = shapes.size();
            break;
        case KoShapeReorderCommand::RaiseShape:
            if (index < shapes.size()) {
                ++index;
            }
            break;
        case KoShapeReorderCommand::LowerShape:
            if (index > 0) {
                --index;
            }
            break;
        case KoShapeReorderCommand::SendToBack:
            index = 0;
            break;
        }
        shapes.insert(index,s);
    }
}

// static
KoShapeReorderCommand *KoShapeReorderCommand::createCommand(const QList<KoShape*> &shapes, KoShapeManager *manager, MoveShapeType move, KUndo2Command *parent)
{
    /**
     * TODO: this method doesn't handle the case when one of the shapes
     *       has maximum or minimum zIndex value (which is 16-bit in our case)!
     */

    QList<int> newIndexes;
    QList<KoShape*> changedShapes;
    QMap<KoShape*, QList<KoShape*> > newOrder;
    QList<KoShape*> sortedShapes(shapes);
    std::sort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);
    if (move == BringToFront || move == LowerShape) {
        for (int i = 0; i < sortedShapes.size(); ++i) {
            prepare(sortedShapes.at(i), newOrder, manager, move);
        }
    }
    else {
        for (int i = sortedShapes.size() - 1; i >= 0; --i) {
            prepare(sortedShapes.at(i), newOrder, manager, move);
        }
    }

    QMap<KoShape*, QList<KoShape*> >::iterator newIt(newOrder.begin());
    for (; newIt!= newOrder.end(); ++newIt) {
        QList<KoShape*> order(newIt.value());
        order.removeAll(0);
        int index = -KoShape::maxZIndex - 1; // set minimum zIndex
        int pos = 0;
        for (; pos < order.size(); ++pos) {
            if (order[pos]->zIndex() > index) {
                index = order[pos]->zIndex();
            }
            else {
                break;
            }
        }

        if (pos == order.size()) {
            //nothing needs to be done
            continue;
        }
        else if (pos <= order.size() / 2) {
            // new index for the front
            int startIndex = order[pos]->zIndex() - pos;
            for (int i = 0; i < pos; ++i) {
                changedShapes.append(order[i]);
                newIndexes.append(startIndex++);
            }
        }
        else {
            //new index for the end
            for (int i = pos; i < order.size(); ++i) {
                changedShapes.append(order[i]);
                newIndexes.append(++index);
            }
        }
    }
    Q_ASSERT(changedShapes.count() == newIndexes.count());
    return changedShapes.isEmpty() ? 0: new KoShapeReorderCommand(changedShapes, newIndexes, parent);
}

KoShapeReorderCommand *KoShapeReorderCommand::mergeInShape(QList<KoShape *> shapes, KoShape *newShape, KUndo2Command *parent)
{
    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    QList<KoShape*> reindexedShapes;
    QList<int> reindexedIndexes;

    const int originalShapeZIndex = newShape->zIndex();
    int newShapeZIndex = originalShapeZIndex;
    int lastOccupiedShapeZIndex = originalShapeZIndex + 1;

    Q_FOREACH (KoShape *shape, shapes) {
        if (shape == newShape) continue;

        const int zIndex = shape->zIndex();

        if (newShapeZIndex == originalShapeZIndex) {
            if (zIndex == originalShapeZIndex) {
                newShapeZIndex = originalShapeZIndex + 1;
                lastOccupiedShapeZIndex = newShapeZIndex;

                reindexedShapes << newShape;
                reindexedIndexes << newShapeZIndex;
            }
        } else {
            if (zIndex >= newShapeZIndex &&
                zIndex <= lastOccupiedShapeZIndex) {

                lastOccupiedShapeZIndex = zIndex + 1;
                reindexedShapes << shape;
                reindexedIndexes << lastOccupiedShapeZIndex;
            }
        }
    }

    return !reindexedShapes.isEmpty() ? new KoShapeReorderCommand(reindexedShapes, reindexedIndexes, parent) : 0;
}

QList<KoShapeReorderCommand::IndexedShape>
KoShapeReorderCommand::homogenizeZIndexes(QList<KoShapeReorderCommand::IndexedShape> shapes)
{
    if (shapes.isEmpty()) return shapes;

    // the shapes are expected to be sorted, we just need to adjust the indexes

    int lastIndex = shapes.begin()->zIndex;

    auto it = shapes.begin() + 1;
    while (it != shapes.end()) {
        if (it->zIndex <= lastIndex) {
            it->zIndex = lastIndex + 1;
        }
        lastIndex = it->zIndex;
        ++it;
    }

    const int overflowSize = shapes.last().zIndex - int(std::numeric_limits<qint16>::max());

    if (overflowSize > 0) {
        if (shapes.first().zIndex - overflowSize > int(std::numeric_limits<qint16>::min())) {
            for (auto it = shapes.begin(); it != shapes.end(); ++it) {
                it->zIndex -= overflowSize;
            }
        } else {
            int index = shapes.size() < int(std::numeric_limits<qint16>::max()) ?
                        0 :
                        int(std::numeric_limits<qint16>::max()) - shapes.size();

            for (auto it = shapes.begin(); it != shapes.end(); ++it) {
                it->zIndex = index;
                index++;
            }
        }
    }

    return shapes;
}

QList<KoShapeReorderCommand::IndexedShape>
KoShapeReorderCommand::homogenizeZIndexesLazy(QList<KoShapeReorderCommand::IndexedShape> shapes)
{
    shapes = homogenizeZIndexes(shapes);
    // remove shapes that didn't change
    for (auto it = shapes.begin(); it != shapes.end();) {
        if (it->zIndex == it->shape->zIndex()) {
            it = shapes.erase(it);
        } else {
            ++it;
        }
    }

    return shapes;
}

QList<KoShapeReorderCommand::IndexedShape>
KoShapeReorderCommand::mergeDownShapes(QList<KoShape *> shapesBelow, QList<KoShape *> shapesAbove)
{
    std::sort(shapesBelow.begin(), shapesBelow.end(), KoShape::compareShapeZIndex);
    std::sort(shapesAbove.begin(), shapesAbove.end(), KoShape::compareShapeZIndex);

    QList<IndexedShape> shapes;
    Q_FOREACH (KoShape *shape, shapesBelow) {
        shapes.append(IndexedShape(shape));
    }

    Q_FOREACH (KoShape *shape, shapesAbove) {
        shapes.append(IndexedShape(shape));
    }

    return homogenizeZIndexesLazy(shapes);
}

QDebug operator<<(QDebug dbg, const KoShapeReorderCommand::IndexedShape &indexedShape)
{
    dbg.nospace() << "IndexedShape (" << indexedShape.shape << ", " << indexedShape.zIndex << ")";
    return dbg.space();
}
