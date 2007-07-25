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
#include "klocale.h"
#include "kicon.h"

KisNode::KisNode()
{
}

KisNode::KisNode( KisImageWSP image, const QString & name )
{
}

KisNode::KisNode( const KisNode & rhs )
{
}

KisNode::~KisNode()
{
}

KoDocumentSectionModel::PropertyList KisNode::properties() const
{
    KoDocumentSectionModel::PropertyList l;
    l << KoDocumentSectionModel::Property(i18n("Visible"), KIcon("visible"), KIcon("novisible"), visible());
    l << KoDocumentSectionModel::Property(i18n("Locked"), KIcon("locked"), KIcon("unlocked"), locked());
    return l;
}

void KisNode::setProperties( const KoDocumentSectionModel::PropertyList &properties )
{
//     setVisible( properties.at( 0 ).state.toBool() );
//     setLocked( properties.at( 1 ).state.toBool() );
}



int KisNode::index() const
{
}

KisNode * KisNode::parentNode()
{
}

const KisNode * KisNode::parentNode() const
{
}

KisNode * KisNode::prevSiblingNode() const
{
}

KisNode * KisNode::nextSiblingNode() const
{
}

KisNode * KisNode::findNode(const QString& name) const
{
}

KisNode * KisNode::findNode(int id) const
{
}

int KisNode::numNodes(int type) const
{
}

void KisNode::updateSettings()
{
    KisNode * child = firstChildNode();
    while ( child ) {
        child->updateSettings();
        child = child->nextSiblingNode();
    }
}
