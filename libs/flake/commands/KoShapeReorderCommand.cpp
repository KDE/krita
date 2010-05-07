/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoShapeReorderCommand.h"
#include "KoShape.h"
#include "KoShapeManager.h"
#include "KoShapeContainer.h"

#include <klocale.h>
#include <kdebug.h>
#include <limits.h>


class KoShapeReorderCommandPrivate
{
public:
    KoShapeReorderCommandPrivate(const QList<KoShape*> &s, QList<int> &ni)
        : shapes(s), newIndexes(ni)
    {
    }

    QList<KoShape*> shapes;
    QList<int> previousIndexes;
    QList<int> newIndexes;
};

KoShapeReorderCommand::KoShapeReorderCommand(const QList<KoShape*> &shapes, QList<int> &newIndexes, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KoShapeReorderCommandPrivate(shapes, newIndexes))
{
    Q_ASSERT(shapes.count() == newIndexes.count());
    foreach (KoShape *shape, shapes)
        d->previousIndexes.append(shape->zIndex());

    setText(i18n("Reorder shapes"));
}

KoShapeReorderCommand::~KoShapeReorderCommand()
{
    delete d;
}

void KoShapeReorderCommand::redo()
{
    QUndoCommand::redo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->setZIndex(d->newIndexes.at(i));
        d->shapes.at(i)->update();
    }
}

void KoShapeReorderCommand::undo()
{
    QUndoCommand::undo();
    for (int i = 0; i < d->shapes.count(); i++) {
        d->shapes.at(i)->update();
        d->shapes.at(i)->setZIndex(d->previousIndexes.at(i));
        d->shapes.at(i)->update();
    }
}

static void prepare(KoShape *s, QMap<KoShape*, QList<KoShape*> > &newOrder, KoShapeManager *manager, KoShapeReorderCommand::MoveShapeType move)
{
    KoShapeContainer *parent = s->parent();
    QMap<KoShape*, QList<KoShape*> >::iterator it( newOrder.find( parent ) );
    if ( it == newOrder.end() ) {
        QList<KoShape*> children;
        if ( parent != 0 ) {
            children = parent->shapes();
        }
        else {
            // get all toplevel shapes
            children = manager->topLevelShapes();
        }
        qSort(children.begin(), children.end(), KoShape::compareShapeZIndex);
        // the append and prepend are needed so that the raise/lower of all shapes works as expected.
        children.append(0);
        children.prepend(0);
        it = newOrder.insert(parent, children);
    }
    QList<KoShape *> & shapes(newOrder[parent]);
    int index = shapes.indexOf(s);
    if (index != -1) {
        shapes.removeAt(index);
        switch ( move ) {
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
KoShapeReorderCommand *KoShapeReorderCommand::createCommand(const QList<KoShape*> &shapes, KoShapeManager *manager, MoveShapeType move, QUndoCommand *parent)
{
    QList<int> newIndexes;
    QList<KoShape*> changedShapes;
    QMap<KoShape*, QList<KoShape*> > newOrder;
    QList<KoShape*> sortedShapes(shapes);
    qSort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);
    if ( move == BringToFront || move == LowerShape ) {
        for ( int i = 0; i < sortedShapes.size(); ++i ) {
            prepare(sortedShapes.at(i), newOrder, manager, move);
        }
    }
    else {
        for ( int i = sortedShapes.size() - 1; i >= 0; --i ) {
            prepare(sortedShapes.at(i), newOrder, manager, move);
        }
    }


    QMap<KoShape*, QList<KoShape*> >::iterator newIt(newOrder.begin());
    for (; newIt!= newOrder.end(); ++newIt) {
        QList<KoShape*> order( newIt.value() );
        order.removeAll(0);
        int index = -2^13;
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
