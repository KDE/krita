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

#include <klocale.h>
#include <kdebug.h>
#include <limits.h>

KoShapeReorderCommand::KoShapeReorderCommand(const QList<KoShape*> &shapes, QList<int> &newIndexes, QUndoCommand *parent)
: QUndoCommand(parent)
, m_shapes(shapes)
, m_newIndexes(newIndexes)
{
    Q_ASSERT(m_shapes.count() == m_newIndexes.count());
    foreach(KoShape *shape, shapes)
        m_previousIndexes.append(shape->zIndex());

    setText(i18n( "Reorder shapes" ));
}

void KoShapeReorderCommand::redo() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->setZIndex( m_newIndexes.at(i) );
        m_shapes.at(i)->repaint();
    }
}

void KoShapeReorderCommand::undo() {
    for(int i=0; i < m_shapes.count(); i++) {
        m_shapes.at(i)->repaint();
        m_shapes.at(i)->setZIndex( m_previousIndexes.at(i) );
        m_shapes.at(i)->repaint();
    }
}

// static
KoShapeReorderCommand *KoShapeReorderCommand::createCommand(const KoSelectionSet &shapes, KoShapeManager *manager, MoveShapeType move, QUndoCommand *parent) {
    QList<int> newIndexes;
    QList<KoShape*> changedShapes;
    foreach(KoShape *shape, shapes) {
        // for each shape create a 'stack' and then move the shape up/down
        // since two indexes can not collide we may need to change the zIndex of a number
        // of other shapes in the stack as well.
        QList<KoShape*> sortedShapes( manager->shapesAt(shape->boundingRect()) );
        qSort(sortedShapes.begin(), sortedShapes.end(), KoShape::compareShapeZIndex);
        if(move == BringToFront) {
            KoShape *top = *(--sortedShapes.end());
            if(shape != top) {
                changedShapes.append(shape);
                newIndexes.append(top->zIndex()+1);
            }
        }
        else if(move == SendToBack) {
            KoShape *bottom = (*sortedShapes.begin());
            if(shape != bottom) {
                changedShapes.append(shape);
                newIndexes.append(bottom->zIndex()-1);
            }
        }
        else {
            QList<KoShape*>::Iterator iter = sortedShapes.begin();
            while((*iter) != shape)
                iter++;

            if(move == RaiseShape) {
                if(++iter == sortedShapes.end()) continue; // already at top
                int newIndex = (*iter)->zIndex()+1;
                changedShapes.append(shape);
                newIndexes.append(newIndex);
                ++iter; // skip the one we want to get above.
                while(iter != sortedShapes.end() && newIndex <= (*iter)->zIndex()) {
                    changedShapes.append(*iter);
                    newIndexes.append(++newIndex);
                    iter++;
                }
            }
            else if(move == LowerShape) {
                if(--iter == sortedShapes.begin()) continue; // already at bottom
                int newIndex = (*iter)->zIndex()-1;
                changedShapes.append(shape);
                newIndexes.append(newIndex);
                --iter; // skip the one we want to get below
                while(iter != sortedShapes.begin() && newIndex >= (*iter)->zIndex()) {
                    changedShapes.append(*iter);
                    newIndexes.append(--newIndex);
                    iter--;
                }
            }
        }
    }
    Q_ASSERT(changedShapes.count() == newIndexes.count());
    return new KoShapeReorderCommand(changedShapes, newIndexes, parent);
}
