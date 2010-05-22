/*
 * Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
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
#include <KDebug>

FolderShapeModel::FolderShapeModel(FolderShape *parent)
    : m_parent(parent)
{
}

void FolderShapeModel::remove(KoShape *child)
{
    m_icons.removeAll(child);
}

void FolderShapeModel::setClipped(const KoShape *, bool )
{
}

bool FolderShapeModel::isClipped(const KoShape *) const
{
    return true;
}

bool FolderShapeModel::inheritsTransform(const KoShape *shape) const
{
    return true;
}

void FolderShapeModel::setInheritsTransform(const KoShape *, bool)
{
}

bool FolderShapeModel::isChildLocked(const KoShape *) const
{
    return false;
}

int FolderShapeModel::count() const
{
    return m_icons.count();
}

QList<KoShape *> FolderShapeModel::shapes() const
{
    return m_icons;
}

void FolderShapeModel::containerChanged(KoShapeContainer *, KoShape::ChangeType)
{
}

void FolderShapeModel::folderResized()
{
    int x = 5, y = 5;
    const qreal width = m_parent->size().width();
    int rowHeight=0;
    foreach (KoShape *shape, m_icons) {
        const QSizeF size = shape->size();
        if (x + size.width() > width) { // next row
            y += rowHeight + 5; // 5 = gap
            x = 5;
        }
        shape->update();
        shape->setPosition(QPointF(x, y));
        shape->update();

        rowHeight = qMax(rowHeight, qRound(size.height()));
        x += (int)size.width() + 5;
    }
}

void FolderShapeModel::childChanged(KoShape *child, KoShape::ChangeType type)
{
    Q_UNUSED(child);
    Q_UNUSED(type);
}

void FolderShapeModel::add(KoShape *shape)
{
    int x = 5, y = 5;
    const int w = (int) shape->size().width();
    const qreal width = m_parent->size().width();
    bool ok;
    do {
        int rowHeight=0;
        ok = true;
        foreach (const KoShape *shape, m_icons) {
            if (shape->position().y() > y || shape->position().y() + shape->size().height() < y)
                continue; // other row.
            rowHeight = qMax(rowHeight, qRound(shape->size().height()));
            x = qMax(x, qRound(shape->position().x() + shape->size().width()) + 5); // 5=gap
            if (x + w > width) { // next row
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
