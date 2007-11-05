/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_mask.h"

#include <kdebug.h>
#include <klocale.h>

#include "KoColorSpaceRegistry.h"

#include "kis_types.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_selection.h"
#include "kis_pixel_selection.h"

struct KisMask::Private
{
    KisSelectionSP selection;
};


KisMask::KisMask( const QString & name )
    : m_d( new Private() )
{
    setName( name );
    m_d->selection = new KisSelection();
}

KisMask::KisMask(const KisMask& rhs)
    : KisNode( rhs )
    , m_d( new Private() )
{
    setName( rhs.name() );
    m_d->selection = new KisSelection( *rhs.m_d->selection.data() );
}

KisMask::~KisMask()
{
    delete m_d;
}

KoDocumentSectionModel::PropertyList KisMask::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l = KisBaseNode::sectionModelProperties();
    l << KoDocumentSectionModel::Property(i18n("Active"), KIcon("active"), KIcon("inactive"), active());
    return l;
}

void KisMask::setSectionModelProperties( const KoDocumentSectionModel::PropertyList &properties )
{
    setActive( properties.at( 2 ).state.toBool() );
}

bool KisMask::active() const
{
    return nodeProperties().boolProperty( "active", true );
}

void KisMask::setActive( bool active )
{
    nodeProperties().setProperty( "active", active );
}

KisSelectionSP KisMask::selection() const
{
    return m_d->selection;
}

void KisMask::setSelection( KisSelectionSP selection )
{
    m_d->selection = selection;
}

void KisMask::select( const QRect & rc, quint8 selectedness )
{
    Q_ASSERT( m_d->selection );
    KisPixelSelectionSP psel = m_d->selection->getOrCreatePixelSelection();
    psel->select( rc, selectedness );
    m_d->selection->updateProjection();
}

#include "kis_mask.moc"
