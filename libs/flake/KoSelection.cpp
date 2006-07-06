/* This file is part of the KDE project

   Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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
{
    m_eventTriggered = false;
}

KoSelection::~KoSelection()
{
}

void KoSelection::paint( QPainter &painter, KoViewConverter &converter)
{
/*    if ( count() == 0 )
        return;
    painter.setRenderHint( QPainter::Antialiasing, false );
    QRectF bb = converter.documentToView( boundingRect() );
    QPen pen( Qt::blue ); //TODO make it configurable
    painter.setPen( pen );
    painter.drawRect( bb );
*/
}

void KoSelection::select(KoShape * object)
{
    Q_ASSERT(object != this);
    if(m_selectedObjects.contains(object))
        return;
    m_selectedObjects << object;
    KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(object->parent());
    if(group && !m_selectedObjects.contains(group)) {
        m_selectedObjects << group;
        foreach(KoShape *shape, group->iterator()) {
            if(! m_selectedObjects.contains(shape))
                m_selectedObjects << shape;
        }
    }
    m_unmodifiedSize = boundingRect().size();
    rotate(0);
    shear(0,0);
    scale(1,1);
    requestSelectionChangedEvent();
}

void KoSelection::deselect(KoShape * object)
{
    if(! m_selectedObjects.contains(object))
        return;
    KoShapeGroup *group = dynamic_cast<KoShapeGroup*>(object->parent());
    if(group) {
        m_selectedObjects.remove(group);
        foreach(KoShape *shape, group->iterator())
            m_selectedObjects.remove(shape);
    }
    else
        m_selectedObjects.remove( object );
    m_unmodifiedSize = boundingRect().size();
    rotate(0);
    shear(0,0);
    scale(1,1);
    requestSelectionChangedEvent();
}

void KoSelection::deselectAll()
{
    if(m_selectedObjects.count() == 0)
        return;
    m_selectedObjects.clear();
    m_unmodifiedSize = boundingRect().size();
    rotate(0);
    shear(0,0);
    scale(1,1);
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
    QRectF bb( boundingRect() );
    resize( bb.size() );
    setPosition( bb.topLeft() );
    emit selectionChanged();
}

int KoSelection::count() const
{
    return m_selectedObjects.count();
}

bool KoSelection::hitTest( const QPointF &position ) const
{
    if ( count() > 1 )
    {
        QRectF bb( boundingRect() );
        return bb.contains( position );
    }
    else if ( count() == 1 )
        return ( *m_selectedObjects.begin() )->hitTest( position );
    else // count == 0
        return false;
}

QSizeF KoSelection::unmodifiedSize() const
{
    return m_unmodifiedSize;
}

QRectF KoSelection::boundingRect() const
{
    bool first=true;
    QRectF bb;
    if ( count() > 0 )
    {
        KoSelectionSet::const_iterator it = m_selectedObjects.begin();
        for ( ; it != m_selectedObjects.end(); ++it ) {
            if( dynamic_cast<KoShapeGroup*>( *it ))
                continue;
            if(first) {
                bb = (*it)->boundingRect();
                first = false;
            }
            else
                bb = bb.unite( ( *it )->boundingRect() );
        }
    }
    return bb;
}

const KoSelectionSet KoSelection::selectedShapes(KoFlake::SelectionType strip) const {
    KoSelectionSet answer;
    // strip the child objects when there is also a parent included.
    bool doStripping = strip == KoFlake::StrippedSelection;
    foreach (KoShape *shape, m_selectedObjects) {
        KoShapeContainer *container = shape->parent();
        if(dynamic_cast<KoShapeGroup*>(shape))
            continue;
        bool add = true;
        while(doStripping && add && container) {
            if(dynamic_cast<KoShapeGroup*>(container) == 0 && m_selectedObjects.contains(container))
                add = false;
            container = container->parent();
        }
        if(add)
            answer << shape;
    }
    return answer;
}

bool KoSelection::isSelected(const KoShape *object) const {
    if(object == this)
        return true;

    QSetIterator<KoShape*> iter (m_selectedObjects);
    while(iter.hasNext()) {
        if(iter.next() == object)
            return true;
    }

    return false;
}

KoShape *KoSelection::firstSelectedShape(KoFlake::SelectionType strip) const {
    KoSelectionSet set = selectedShapes(strip);
    if(set.isEmpty())
        return 0;
    return *(set.begin());
}

#include "KoSelection.moc"
