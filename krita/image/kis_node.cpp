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
#include <klocale.h>
#include <kicon.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include "kis_image.h"

class KisNode::Private
{
public:

    bool useProjections;
    bool locked;
    bool visible;
    QString name;
    KisNodeSP parent;
    KisImageWSP image;
    int index;

};

KisNode::KisNode()
{
    init();
}

KisNode::KisNode( KisImageWSP image, const QString & name )
{
    init();
    m_d->image = image;
    m_d->name = name;
}

void KisNode::init()
{

    m_d = new KisNode::Private();
    updateSettings();
    m_d->visible = true;
    m_d->locked = false;
    m_d->parent = 0;
    m_d->image = 0;
    m_d->index = -1;
}

KisNode::KisNode( const KisNode & rhs )
    : KisShared( rhs )
{
    m_d->index = rhs.m_d->index;
    m_d->locked = rhs.m_d->locked;
    m_d->visible = rhs.m_d->visible;
    m_d->name = rhs.m_d->name;
    m_d->image = rhs.m_d->image;
    m_d->parent = 0;
}

KisNode::~KisNode()
{
    delete m_d;
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

KisNodeSP KisNode::parentNode()
{
}

const KisNodeSP KisNode::parentNode() const
{
}

KisNodeSP KisNode::prevSiblingNode() const
{
}

KisNodeSP KisNode::nextSiblingNode() const
{
}

uint KisNode::childCount() const
{
    return 0;
}

KisNodeSP KisNode::atNode(int index) const
{
    return 0;
}

KisNodeSP KisNode::firstChildNode() const
{
    return 0;
}

KisNodeSP KisNode::lastChildNode() const
{
    return 0;
}



KisNodeSP KisNode::findNode(const QString& name) const
{
}

KisNodeSP KisNode::findNode(int id) const
{
}

int KisNode::numNodes(int type) const
{
}

void KisNode::updateSettings()
{
    KisNodeSP child = firstChildNode();
    while ( child ) {
        child->updateSettings();
        child = child->nextSiblingNode();
    }

    KConfigGroup cfg = KGlobal::config()->group("");
    m_d->useProjections = cfg.readEntry("useProjections",  true );
    kDebug(41001) << "Node stack uses projections for caching: " << m_d->useProjections << endl;
}


bool KisNode::useProjections()
{
    return m_d->useProjections;
}

// void KisNode::setIndex( int index )
// {
//     m_d->index = index;
// }

