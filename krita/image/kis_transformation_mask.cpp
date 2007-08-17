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


#include "kis_transformation_mask.h"

#include "kis_selection.h"
#include "kis_paint_device.h"
#include "kis_transform_worker.h"

#include "QRect"

KisTransformationMask::KisTransformationMask()
    : KisEffectMask()
    , m_xscale( 0 )
    , m_yscale( 0 )
    , m_xshear( 0 )
    , m_yshear( 0 )
    , m_rotation( 0 )
    , m_xtranslate( 0 )
    , m_ytranslate( 0 )
    , m_filter( 0 )
{}

KisTransformationMask::~KisTransformationMask()
{
}

KisTransformationMask::KisTransformationMask( const KisTransformationMask& rhs )
    : KisEffectMask( rhs )
{
    m_xscale = rhs.m_xscale;
    m_yscale = rhs.m_yscale;
    m_xshear = rhs.m_xshear;
    m_yshear = rhs.m_yshear;
    m_rotation = rhs.m_rotation;
    m_xtranslate = rhs.m_xtranslate;
    m_ytranslate = rhs.m_ytranslate;
    m_filter = rhs.m_filter;
}

void KisTransformationMask::apply( KisPaintDeviceSP projection, const QRect & rc ) const
{
    // Create a selection
    KisSelectionSP selection = new KisSelection( 0, const_cast<KisTransformationMask*>( this ) );

    // Make the selection as small as the required rect
    selection->crop( rc );

    // Save the old selection to restore it later
    KisSelectionSP oldSelection = projection->setSelection( selection );

    // Transform
    KisTransformWorker worker( projection, m_xscale, m_yscale, m_xshear, m_yshear, m_rotation, m_xtranslate, m_ytranslate, 0, m_filter );
    worker.run();

    // Restore the old selection
    projection->setSelection( oldSelection );
}

#include "kis_transformation_mask.moc"
