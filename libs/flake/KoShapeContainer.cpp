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

#include "KoShapeContainer.h"

#include <QPointF>
#include <QPainter>

KoShapeContainer::KoShapeContainer() : KoShape() {
    m_children = 0;
}

KoShapeContainer::KoShapeContainer(KoGraphicsContainerModel *model)
: KoShape()
, m_children(model) {
}

KoShapeContainer::~KoShapeContainer() {
    delete m_children;
}

void KoShapeContainer::addChild(KoShape *object) {
    Q_ASSERT(object);
    if(m_children == 0)
        m_children = new ChildrenData();
    m_children->add(object);
    object->setParent(this);
}

void KoShapeContainer::removeChild(KoShape *object) {
    Q_ASSERT(object);
    if(m_children == 0)
        return;
    m_children->remove(object);
    object->setParent(0);
}

int  KoShapeContainer::childCount() const {
    if(m_children == 0)
        return 0;
    return m_children->count();
}

void KoShapeContainer::setClipping(const KoShape *child, bool clipping) {
    if(m_children == 0)
        return;
    m_children->setClipping(child, clipping);
}

void KoShapeContainer::paint(QPainter &painter, KoViewConverter &converter) {
    painter.save();
    applyConversion(painter, converter);
    paintComponent(painter, converter);
    painter.restore();
    if(m_children == 0 || m_children->count() == 0)
        return;

    QList<KoShape*> sorterdObjects = m_children->iterator();
    qSort(sorterdObjects.begin(), sorterdObjects.end(), KoShape::compareShapeZIndex);
    painter.setMatrix( m_invMatrix * painter.matrix() );
    QMatrix myMatrix = transformationMatrix(&converter);
    foreach (KoShape *shape, sorterdObjects) {
        if(! shape->isVisible())
            continue;
        painter.save();
        // TODO this is not perfect yet..
//           QRectF clipRect(QPoint(0,0), size()); // old
//           QPolygon clip = (myMatrix * shapeMatrix.inverted()).mapToPolygon(clipRect.toRect());
//           painter.setClipRegion(QRegion(clip));

        if( childClipped(shape) ) {

            QRectF clipRect(QPointF(0, 0), size());
            clipRect = converter.documentToView(clipRect);

            QPolygon clip = myMatrix.mapToPolygon(clipRect.toRect());
            clip.translate( (position() - converter.documentToView(position())).toPoint() );
            painter.setClipRegion(QRegion(clip));
        }
//kDebug() << "rect: " << position() << endl;
//kDebug() << "polygon: " << clip.boundingRect() << endl;
        //painter.drawPolygon(clip);
        painter.setMatrix( shape->transformationMatrix(&converter) * painter.matrix() );
        shape->paint(painter, converter);
        painter.restore();
    }
}

void KoShapeContainer::recalcMatrix() {
    KoShape::recalcMatrix();
    if(m_children == 0)
        return;
    m_children->containerChanged(this);
    foreach (KoShape *shape, m_children->iterator())
        shape->recalcMatrix();
}

bool KoShapeContainer::childClipped(const KoShape *child) const {
    if(m_children == 0) // throw exception??
        return false;
    return m_children->childClipped(child);
}


// ##  inner class ChildrenData
KoShapeContainer::ChildrenData::ChildrenData() {
}

KoShapeContainer::ChildrenData::~ChildrenData() {
    // TODO will the relation instances in m_relations be deleted?
}

void KoShapeContainer::ChildrenData::add(KoShape *child) {
    Relation *r = new Relation(child);
    m_relations.append(r);
}

KoShapeContainer::ChildrenData::Relation* KoShapeContainer::ChildrenData::findRelation(const KoShape *child) const {
    foreach(Relation *relation, m_relations) {
        if(relation->child() == child)
            return relation;
    }
    return 0;
}

void KoShapeContainer::ChildrenData::setClipping(const KoShape *child, bool clipping) {
    Relation *relation = findRelation(child);
    if(relation == 0) // throw exception?
        return;
    if(relation->m_inside == clipping)
        return;
    relation->m_inside = clipping;
    relation->child()->repaint();
    relation->child()->recalcMatrix();
    relation->child()->repaint();
}

void KoShapeContainer::ChildrenData::remove(KoShape *child) {
    Relation *relation = findRelation(child);
    if(relation == 0)
        return;
    m_relations.removeAll(relation);
}

int KoShapeContainer::ChildrenData::count() const {
    return m_relations.count();
}

bool KoShapeContainer::ChildrenData::childClipped(const KoShape *child) const {
    Relation *relation = findRelation(child);
    if(relation == 0) // throw exception?
        return false;
    return relation->m_inside;
}

QList<KoShape*> KoShapeContainer::ChildrenData::iterator() const {
    QList<KoShape*> answer;
    foreach (Relation *relation, m_relations)
        answer.append(relation->child());
    return answer;
}

void KoShapeContainer::ChildrenData::containerChanged(KoShapeContainer *container) {
    Q_UNUSED(container);
}

void KoShapeContainer::repaint() const {
    KoShape::repaint();
    if(m_children)
        foreach ( KoShape *shape, m_children->iterator())
            shape->repaint();
}

QList<KoShape*> KoShapeContainer::iterator() const {
    return m_children->iterator();
}

// ## inner class KoGraphicsContainerModel::Relation
KoShapeContainer::ChildrenData::Relation::Relation(KoShape *child)
:m_inside(false)
, m_child(child) {
}
