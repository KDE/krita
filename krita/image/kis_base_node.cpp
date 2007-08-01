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
#include <klocale.h>
#include "kis_base_node.h"


class KisBaseNode::Private
{
public:

    bool visible;
    bool locked;
    KisBaseNodeSP linkedTo;
};

KisBaseNode::KisBaseNode()
    : m_d( new Private() )
{
    m_d->visible = true;
    m_d->locked = false;
    m_d->linkedTo = 0;
}


KisBaseNode::KisBaseNode( const KisBaseNode & rhs )
    : QObject()
    , KisShared( rhs )
    ,  m_d( new Private() )
{
    m_d->visible = rhs.m_d->visible;
    m_d->locked = rhs.m_d->locked;
    m_d->linkedTo = rhs.m_d->linkedTo;
}

KisBaseNode::~KisBaseNode()
{
    delete m_d;
}

KoDocumentSectionModel::PropertyList KisBaseNode::properties() const
{
    KoDocumentSectionModel::PropertyList l;
    l << KoDocumentSectionModel::Property(i18n("Visible"), KIcon("visible"), KIcon("novisible"), visible());
    l << KoDocumentSectionModel::Property(i18n("Locked"), KIcon("locked"), KIcon("unlocked"), locked());
    // XXX: Add linked!
    return l;
}

void KisBaseNode::setProperties( const KoDocumentSectionModel::PropertyList &properties )
{
    setVisible( properties.at( 0 ).state.toBool() );
    setLocked( properties.at( 1 ).state.toBool() );
}

QImage KisBaseNode::createThumbnail(qint32 w, qint32 h )
{
    try {
        QImage img( w, h, QImage::Format_ARGB32 );
        img.fill( 0 );
        return img;
    }
    catch ( std::bad_alloc )
    {
        return QImage();
    }

}

const bool KisBaseNode::visible() const
{
    return m_d->visible;
}

void KisBaseNode::setVisible(bool visible)
{
    m_d->visible = visible;
}

bool KisBaseNode::locked() const
{
    return m_d->locked;
}

void KisBaseNode::setLocked(bool locked)
{
    m_d->locked = locked;
}

#include "kis_base_node.moc"
