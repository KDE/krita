/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
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
#include "kis_transparency_mask.h"
#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"
#include "KoColorSpace.h"

KisTransparencyMask::KisTransparencyMask()
    : KisEffectMask()
{
}

KisTransparencyMask::~KisTransparencyMask()
{
}

KisTransparencyMask::KisTransparencyMask( const KisTransparencyMask& rhs )
    : KisEffectMask( rhs )
{
}

void KisTransparencyMask::apply( KisPaintDeviceSP projection, const QRect & rc )
{
    KoColorSpace * cs = projection->colorSpace();

    KisHLineIteratorPixel projectionIt = projection->createHLineIterator( rc.x(), rc.y(), rc.width() );
    KisHLineConstIteratorPixel maskIt = createHLineConstIterator( rc.x(), rc.y(), rc.width() );

    for ( int row = rc.y(); row < rc.height(); ++row ) {
        while ( !projectionIt.isDone() ) {

            int pixels = qMin( projectionIt.nConseqHPixels(), maskIt.nConseqHPixels() );
            cs->applyAlphaU8Mask(projectionIt.rawData(), maskIt.rawData(), pixels);

            projectionIt += pixels;
            maskIt += pixels;
        }
        projectionIt.nextRow();
        maskIt.nextRow();
    }


}
