/*
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "FolderShapeModel.h"
#include "FolderShape.h"

#include <KoShapeContainer.h>

FolderShapeModel::FolderShapeModel(FolderShape *parent)
    : m_parent(parent)
{
}

void FolderShapeModel::remove(KoShape *child) {
    m_icons.removeAll(child);
}

void FolderShapeModel::setClipping(const KoShape *, bool ) {
}

bool FolderShapeModel::childClipped(const KoShape *) const {
    return true;
}

bool FolderShapeModel::isChildLocked(const KoShape *) const {
    return false;
}

int FolderShapeModel::count() const {
    return m_icons.count();
}

QList<KoShape *> FolderShapeModel::iterator() const {
    return m_icons;
}

void FolderShapeModel::containerChanged(KoShapeContainer *container) {
}

void FolderShapeModel::childChanged(KoShape *child, KoShape::ChangeType type) {
}

void FolderShapeModel::add(KoShape *shape) {
    int x=5, y=5; // 5 = gap
    int w = (int) shape->size().width();
    const double width = m_parent->size().width();
    bool ok=true; // lets be optimistic ;)
    do {
        int rowHeight=0;
        ok=true;
        foreach(const KoShape *shape, iterator()) {
            if(shape->position().y() > y || shape->position().y() + shape->size().height() < y)
                continue; // other row.
            rowHeight = qMax(rowHeight, qRound(shape->size().height()));
            x = qMax(x, qRound(shape->position().x() + shape->size().width()) + 5); // 5=gap
            if(x + w > width) { // next row
                y += rowHeight + 5; // 5 = gap
                x = 5;
                ok=false;
                break;
            }
        }
    } while(! ok);
    shape->setPosition(QPointF(x, y));
    m_icons.append(shape);
}
