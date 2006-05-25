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

#include "KoShapeGroup.h"

KoShapeGroup::KoShapeGroup()
: KoShapeContainer(new GroupMembers())
{
}

void KoShapeGroup::paintComponent(QPainter &painter, KoViewConverter &converter) {
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

bool KoShapeGroup::hitTest( const QPointF &position ) const {
    Q_UNUSED(position);
    return false;
}

//  ############# GroupMembers #############
void KoShapeGroup::GroupMembers::add(KoShape *child) {
    if(m_groupMembers.contains(child))
        return;
    m_groupMembers.append(child);
}

void KoShapeGroup::GroupMembers::remove(KoShape *child) {
    m_groupMembers.removeAll(child);
}

int KoShapeGroup::GroupMembers::count() const {
    return m_groupMembers.count();
}

QList<KoShape*> KoShapeGroup::GroupMembers::iterator() const {
    return QList<KoShape*>(m_groupMembers);
}

void KoShapeGroup::GroupMembers::containerChanged(KoShapeContainer *container) {
    Q_UNUSED(container);
}

void KoShapeGroup::GroupMembers::setClipping(const KoShape *child, bool clipping) {
    Q_UNUSED(child);
    Q_UNUSED(clipping);
}

bool KoShapeGroup::GroupMembers::childClipped(const KoShape *child) const {
    Q_UNUSED(child);
    return false;
}
