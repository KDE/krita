/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_node.h"
#include <QList>

#include "kdebug.h"

#include "kis_node_graph_listener.h"

class KisNode::Private
{
public:

    KisNodeSP parent;
    KisNodeGraphListener * graphListener;
    QList<KisNodeSP> nodes;
};

KisNode::KisNode()
    : m_d( new Private() )
{
    m_d->parent = 0;
    m_d->graphListener = 0;

}


KisNode::KisNode( const KisNode & rhs )
    : KisBaseNode( rhs )
    , m_d( new Private() )
{
    m_d->parent = 0;
    m_d->graphListener = rhs.m_d->graphListener;
}

KisNode::~KisNode()
{
    delete m_d;
}

KisNodeGraphListener * KisNode::graphListener() const
{
    return m_d->graphListener;
}

void KisNode::setGraphListener( KisNodeGraphListener * graphListener )
{
    m_d->graphListener = graphListener;
}

KisNodeSP KisNode::parent() const
{
    return m_d->parent;
}

void KisNode::setParent( KisNodeSP parent )
{
    m_d->parent = parent;
}

KisNodeSP KisNode::firstChild() const
{
    if ( !m_d->nodes.isEmpty() )
        return m_d->nodes.first();
    else
        return 0;
}

KisNodeSP KisNode::lastChild() const
{
    if ( !m_d->nodes.isEmpty() )
        return m_d->nodes.last();
    else
        return 0;
}

KisNodeSP KisNode::prevSibling() const
{
    if ( !parent() ) return 0;
    int i = parent()->index( const_cast<KisNode*>( this ) );
    return parent()->at( i - 1 );

}

KisNodeSP KisNode::nextSibling() const
{
    if ( !parent() ) return 0;

    return parent()->at( parent()->index( const_cast<KisNode*>( this ) ) + 1 );
}

quint32 KisNode::childCount() const
{
    return m_d->nodes.count();
}


KisNodeSP KisNode::at( quint32 index ) const
{
    if ( !m_d->nodes.isEmpty() && index < ( quint32 )m_d->nodes.size() ) {
        return m_d->nodes.at( index );
    }

    return 0;
}

int KisNode::index( const KisNodeSP node ) const
{
    if ( m_d->nodes.contains( node ) ) {
         return m_d->nodes.indexOf( node );
    }

    return -1;
}

bool KisNode::add( KisNodeSP newNode, KisNodeSP aboveThis )
{
    Q_ASSERT( newNode );

    if ( !newNode ) return false;
    if (aboveThis && aboveThis->parent().data() != this) return false;
    if ( !allowAsChild( newNode ) ) return false;
    if ( newNode->parent() ) return false;
    if ( m_d->nodes.contains(newNode) ) return false;


    int idx = 0;

    if ( aboveThis != 0 ) {

        idx = this->index( aboveThis );

        if ( m_d->graphListener )
            m_d->graphListener->aboutToAddANode( this, idx );

        if ( idx >= ( int )childCount() )
            m_d->nodes.append(newNode);
        else {
            m_d->nodes.insert( idx + 1, newNode );
        }
    }
    else
    {
        if ( m_d->graphListener )
            m_d->graphListener->aboutToAddANode( this, idx );

        m_d->nodes.prepend( newNode );
    }

    newNode->setParent( this );
    newNode->setGraphListener( m_d->graphListener );

    if ( m_d->graphListener )
        m_d->graphListener->nodeHasBeenAdded(this, idx);


    return true;
}

bool KisNode::remove( quint32 index )
{
    if ( index < childCount() )
    {
        KisNodeSP removedNode = at(index);

        removedNode->setParent( 0 );
        removedNode->setGraphListener( 0 );

        if ( m_d->graphListener )
            m_d->graphListener->aboutToRemoveANode( this, index );

        m_d->nodes.removeAt( index );

        if ( m_d->graphListener ) m_d->graphListener->nodeHasBeenRemoved(this, index);

        return true;
    }
    kWarning() << "invalid input to KisGroupNode::removeNode()!";
    return false;
}

bool KisNode::remove( KisNodeSP node )
{
    if ( node->parent().data() != this)
    {
        kWarning() << "invalid input to KisGroupNode::removeNode()!";
        return false;
    }

    return remove( index( node ) );

}


