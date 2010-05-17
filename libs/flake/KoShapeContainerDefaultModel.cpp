/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoShapeContainerDefaultModel.h"

#include "KoShapeContainer.h"

class KoShapeContainerDefaultModel::Private
{
public:
    class Relation
    {
    public:
        explicit Relation(KoShape *child)
        : m_inside(false)
        , m_child(child)
        {}

        KoShape* child()
        {
            return m_child;
        }

        bool m_inside; ///< if true, the child will be clipped by the parent.

    private:
        KoShape *m_child;
    };

    ~Private()
    {
        qDeleteAll(relations);
    }

    Relation* findRelation(const KoShape *child) const
    {
        foreach (Relation *relation, relations) {
            if (relation->child() == child) {
                return relation;
            }
        }
        return 0;
    }


    // TODO use a QMap<KoShape*, bool> instead this should speed things up a bit
    QList<Relation *> relations;
};

KoShapeContainerDefaultModel::KoShapeContainerDefaultModel()
: d( new Private() )
{
}

KoShapeContainerDefaultModel::~KoShapeContainerDefaultModel()
{
    delete d;
}

void KoShapeContainerDefaultModel::add(KoShape *child)
{
    Private::Relation *r = new Private::Relation(child);
    d->relations.append(r);
}

void KoShapeContainerDefaultModel::proposeMove(KoShape *shape, QPointF &move)
{
    KoShapeContainer *parent = shape->parent();
    bool allowedToMove = true;
    while (allowedToMove && parent) {
        allowedToMove = parent->isEditable();
        parent = parent->parent();
    }
    if (! allowedToMove) {
        move.setX( 0 );
        move.setY( 0 );
    }
}


void KoShapeContainerDefaultModel::setClipped(const KoShape *child, bool clipping)
{
    Private::Relation *relation = d->findRelation(child);
    if (relation == 0)
        return;
    if (relation->m_inside == clipping)
        return;
    relation->m_inside = clipping;
    relation->child()->update();
    relation->child()->notifyChanged();
    relation->child()->update();
}

bool KoShapeContainerDefaultModel::isClipped(const KoShape *child) const
{
    Private::Relation *relation = d->findRelation(child);
    return relation ? relation->m_inside: false;
}

void KoShapeContainerDefaultModel::remove(KoShape *child)
{
    Private::Relation *relation = d->findRelation(child);
    if (relation == 0)
        return;
    d->relations.removeAll(relation);
}

int KoShapeContainerDefaultModel::count() const
{
    return d->relations.count();
}

QList<KoShape*> KoShapeContainerDefaultModel::shapes() const
{
    QList<KoShape*> answer;
    foreach(Private::Relation *relation, d->relations) {
        answer.append(relation->child());
    }
    return answer;
}

bool KoShapeContainerDefaultModel::isChildLocked(const KoShape *child) const
{
    return child->isGeometryProtected();
}

void KoShapeContainerDefaultModel::containerChanged(KoShapeContainer *, KoShape::ChangeType)
{
}
