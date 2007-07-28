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

#include "kdebug.h"

#include "kis_node_graph_listener.h"

class KisNode::Private
{
public:

    KisNodeSP parent;
    KisNodeGraphListener * graphListener;
    vKisNodeSP nodes;
};

KisNode::KisNode()
    : m_d( new Private() )
{
    m_d->parent = 0;
    m_d->graphListener = 0;

}


KisNode::KisNode( const KisNode & rhs )
    : KisShared( rhs )
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
    return at(0);
}

KisNodeSP KisNode::lastChild() const
{
    return at(childCount() - 1);
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
    if (    childCount()
         && qBound( uint( 0 ), uint( index ), childCount() - 1 ) == uint( index ) ) {

        return m_d->nodes.at( reverseIndex( index ) );
    }

    return 0;
}

int KisNode::index( const KisNodeSP node ) const
{
    return  m_d->nodes.indexOf( node );
}

bool KisNode::add( KisNodeSP newNode, quint32 index )
{
    Q_ASSERT( newNode );

    if ( !newNode ) return false;
    if ( index > childCount() ) return false;
    if ( newNode->parent() ) return false;
    if ( m_d->nodes.contains(newNode) ) return false;

    if ( m_d->graphListener )
        m_d->graphListener->aboutToAddANode( this, index );

    if (index == 0)
        m_d->nodes.append(newNode);
    else
        m_d->nodes.insert(m_d->nodes.begin() + reverseIndex(index) + 1, newNode);

    newNode->setParent( this );
    newNode->setGraphListener( m_d->graphListener );

    if ( m_d->graphListener )
        m_d->graphListener->nodeHasBeenAdded(this, index);

    return true;

}

bool KisNode::add( KisNodeSP newNode, KisNodeSP aboveThis )
{
    Q_ASSERT( newNode );

    if (aboveThis && aboveThis->parent().data() != this)
    {
        kWarning() << "invalid input to KisGroupNode::addNode(KisNodeSP newNode, KisNodeSP aboveThis)!" << endl;
        return false;
    }

    quint32 i = childCount();
    if ( aboveThis && parent() )
        i = index( aboveThis );

    return add(newNode, i);
}

bool KisNode::remove( quint32 index )
{
    if ( index < childCount() )
    {
        KisNodeSP removedNode = at(index);

        removedNode->setParent( 0 );

        if ( m_d->graphListener )
            m_d->graphListener->aboutToRemoveANode( this, index );

        m_d->nodes.erase(m_d->nodes.begin() + reverseIndex(index));

        if ( m_d->graphListener ) m_d->graphListener->nodeHasBeenRemoved(this, index);

        return true;
    }
    kWarning() << "invalid input to KisGroupNode::removeNode()!" << endl;
    return false;
}

bool KisNode::remove( KisNodeSP node )
{
    if ( node->parent().data() != this)
    {
        kWarning() << "invalid input to KisGroupNode::removeNode()!" << endl;
        return false;
    }

    return remove( index( node ) );

}


