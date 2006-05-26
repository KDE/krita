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

#ifndef KOSHAPEGROUP_H
#define KOSHAPEGROUP_H

#include "KoShapeContainer.h"

#include <QList>

#include <koffice_export.h>

/**
 * Provide grouping for shapes.
 * The group shape allows you to add children which will then be grouped in selections
 * and actions.
 * <p>If you have a set of shapes, that together make up a bigger shape it is often
 * usefull to group them together so the user will precieve the different shapes as
 * actually being one.  This means that if the user clicks on one object, all objects
 * in the group will be selected at ones, making the tools that alter them alter all
 * of them at the same time.
 * <p>Note that while this object is also a shape, it is not actually visible.
 */
class FLAKE_EXPORT KoShapeGroup : public KoShapeContainer {
public:
    /// Constructor
    KoShapeGroup();
    /// destructor
    ~KoShapeGroup() {};
    /// This implementation is empty since a group is itself not visible.
    void paintComponent(QPainter &painter, KoViewConverter &converter);
    /// always returns false since the group itself can't be selected or hit
    bool hitTest( const QPointF &position ) const;

private:
    class GroupMembers: public KoGraphicsContainerModel {
        public:
            GroupMembers() {};
            ~GroupMembers() {};
            void add(KoShape *child);
            void setClipping(const KoShape *child, bool clipping);
            bool childClipped(const KoShape *child) const;
            void remove(KoShape *child);
            int count() const;
            QList<KoShape*> iterator() const;
            void containerChanged(KoShapeContainer *container);

        private: // members
            QList <KoShape *> m_groupMembers;
    };
};

#endif
