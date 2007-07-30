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
#include "kis_node_facade.h"
#include "kis_node.h"

class KisNodeFacade::Private
{
public:

    Private( KisNodeSP node ) : root( node ) {}

    KisNodeSP const root;
};

KisNodeFacade::KisNodeFacade( KisNodeSP root )
    : m_d( new Private( root.data ) )
{
}

KisNodeFacade::~KisNodeFacade()
{
    delete m_d;
}

const KisNodeSP KisNodeFacade::root() const
{
    return m_d->root;
}

bool KisNodeFacade::moveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
}

bool KisNodeFacade::addNode(KisNodeSP node, KisNodeSP parent)
{
}

bool KisNodeFacade::addNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
}

bool KisNodeFacade::addNode( KisNodeSP node,  KisNodeSP parent, int index )
{
}

bool KisNodeFacade::removeNode(KisNodeSP node)
{
}

bool KisNodeFacade::raiseNode(KisNodeSP node)
{
}

bool KisNodeFacade::lowerNode(KisNodeSP node)
{
}

bool KisNodeFacade::toTop( KisNodeSP node )
{
}

bool KisNodeFacade::toBottom( KisNodeSP node )
{
}

quint32 KisNodeFacade::numNodes() const
{
}

quint32 KisNodeFacade::numVisibleNodes() const
{
}

quint32 KisNodeFacade::numHiddenNodes() const
{
}

