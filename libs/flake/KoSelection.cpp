/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>

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

#include "KoSelection.h"
#include "KoShapeContainer.h"
#include "KoShapeGroup.h"
#include "KoPointerEvent.h"

#include <QPainter>
#include <QTimer>

KoSelection::KoSelection()
    : m_eventTriggered( false ), m_activeLayer( 0 )
{
}

KoSelection::~KoSelection()
{
}

void KoSelection::paint( QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);
}

void KoSelection::selectGroupChilds( KoShapeGroup *group )
{
    if( ! group )
        return;

    foreach(KoShape *shape, group->iterator()) {
        if( m_selectedShapes.contains(shape))
            continue;
        m_selectedShapes << shape;

        KoShapeGroup* childGroup = dynamic_cast<KoShapeGroup*>( shape );
        if( childGroup )
            selectGroupChilds( childGroup );
    }
}

void KoSelection::select(KoShape * object)
{
    Q_ASSERT(object != this);
    Q_ASSERT(object);
    if(! object->isSelectable())
        return;
    if(!m_selectedShapes.contains(object))
        m_selectedShapes << object;

    KoShapeGroup* group = dynamic_cast<KoShapeGroup*>(object);
    if( group )
        selectGroupChilds( group );

    KoShapeContainer *parent = object->parent();
    while( parent ) {
        KoShapeGroup *parentGroup = dynamic_cast<KoShapeGroup*>(parent);
        if( ! parentGroup ) break;
        if( ! m_selectedShapes.contains(parentGroup) ) {
            m_selectedShapes << parentGroup;
            selectGroupChilds( parentGroup );
        }
        parent = parentGroup->parent();
    }

    if(m_selectedShapes.count() == 1)
    {
        rotate(object->rotation());
        shear(object->shearX(), object->shearY());
    }
    else
    {
        rotate(0);
        shear(0, 0);
    }
    requestSelectionChangedEvent();
}

void KoSelection::deselect(KoShape * object)
{
    if(! m_selectedShapes.contains(object))
        return;
    KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(object->parent());
    if(group) {
        m_selectedShapes.removeAll(group);
        foreach(KoShape *shape, group->iterator())
            m_selectedShapes.removeAll(shape);
    }
    else
        m_selectedShapes.removeAll( object );

    if(m_selectedShapes.count() == 1)
    {
        rotate(firstSelectedShape()->rotation());
        shear(firstSelectedShape()->shearX(), firstSelectedShape()->shearY());
    }
    requestSelectionChangedEvent();
}

void KoSelection::deselectAll()
{
    if(m_selectedShapes.isEmpty())
        return;
    m_selectedShapes.clear();
    requestSelectionChangedEvent();
}

void KoSelection::requestSelectionChangedEvent() {
    if(m_eventTriggered)
        return;
    m_eventTriggered = true;
    QTimer::singleShot(0, this, SLOT(selectionChangedEvent()));
}

void KoSelection::selectionChangedEvent() {
    m_eventTriggered = false;
    scale(1,1);
    boundingRect(); //has the side effect of updating the size and position
    emit selectionChanged();
}

int KoSelection::count() const
{
    return m_selectedShapes.count();
}

bool KoSelection::hitTest( const QPointF &position ) const
{
    if ( count() > 1 )
    {
        QRectF bb( boundingRect() );
        return bb.contains( position );
    }
    else if ( count() == 1 )
        return ( *m_selectedShapes.begin() )->hitTest( position );
    else // count == 0
        return false;
}

QRectF KoSelection::boundingRect() const
{
    bool first=true;
    QRectF bb;

    QMatrix tmat = transformationMatrix(0);
    QMatrix itmat = tmat.inverted();

    if ( !m_selectedShapes.isEmpty() )
    {
        QList<KoShape*>::const_iterator it = m_selectedShapes.begin();
        for ( ; it != m_selectedShapes.end(); ++it ) {
            if( dynamic_cast<KoShapeGroup*>( *it ))
                continue;
            if(first) {
                bb = ((*it)->transformationMatrix(0) * itmat).mapRect(QRectF(QPointF(),(*it)->size()));
                first = false;
            }
            else
                bb = bb.unite(
                 ((*it)->transformationMatrix(0) * itmat).mapRect(QRectF(QPointF(),(*it)->size())) );
        }
    }

    //just as well use the oppertunity to update the size and position
    (const_cast <KoSelection *>(this))->resize( bb.size() );
    QPointF p = tmat.map(bb.topLeft() + itmat.map(position()));
    (const_cast <KoSelection *>(this))->setPosition( p );

    return tmat.mapRect(bb);
}

const QList<KoShape*> KoSelection::selectedShapes(KoFlake::SelectionType strip) const {
    QList<KoShape*> answer;
    // strip the child objects when there is also a parent included.
    bool doStripping = strip == KoFlake::StrippedSelection;
    foreach (KoShape *shape, m_selectedShapes) {
        KoShapeContainer *container = shape->parent();
        if(strip != KoFlake::TopLevelSelection && dynamic_cast<KoShapeGroup*>(shape))
            // since a KoShapeGroup
            // guarentees all its children are selected at the same time as itself
            // is selected we will only return its children.
            continue;
        bool add = true;
        while(doStripping && add && container) {
            if(dynamic_cast<KoShapeGroup*>(container) == 0 && m_selectedShapes.contains(container))
                add = false;
            container = container->parent();
        }
        if(strip == KoFlake::TopLevelSelection && container && m_selectedShapes.contains(container))
            add = false;
        if(add)
            answer << shape;
    }
    return answer;
}

bool KoSelection::isSelected(const KoShape *object) const {
    if(object == this)
        return true;

    foreach(KoShape *shape, m_selectedShapes)
        if(shape == object)
            return true;

    return false;
}

KoShape *KoSelection::firstSelectedShape(KoFlake::SelectionType strip) const {
    QList<KoShape*> set = selectedShapes(strip);
    if(set.isEmpty())
        return 0;
    return *(set.begin());
}

void KoSelection::setActiveLayer( KoShapeLayer* layer ) {
    m_activeLayer = layer;
}

KoShapeLayer* KoSelection::activeLayer() const {
    return m_activeLayer;
}

#include "KoSelection.moc"
