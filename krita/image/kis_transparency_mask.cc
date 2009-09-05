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

#include "kis_transparency_mask.h"

#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"
#include "KoColorSpace.h"
#include "kis_selection.h"
#include "kis_node_visitor.h"

#include "kis_debug.h"

KisTransparencyMask::KisTransparencyMask()
        : KisEffectMask()
{
    dbgImage << "Creating a transparency mask";
}

KisTransparencyMask::~KisTransparencyMask()
{
}

KisTransparencyMask::KisTransparencyMask(const KisTransparencyMask& rhs)
        : KisEffectMask(rhs)
{
}

bool KisTransparencyMask::allowAsChild(KisNodeSP node) const
{
    Q_UNUSED(node);
    return false;
}

QIcon KisTransparencyMask::icon() const
{
    return KIcon("view-filter");
}

void KisTransparencyMask::apply(KisPaintDeviceSP projection, const QRect & rc) const
{
    selection()->updateProjection(rc);

    const KoColorSpace * cs = projection->colorSpace();

    KisHLineIteratorPixel projectionIt = projection->createHLineIterator(rc.x(), rc.y(), rc.width());
    KisHLineConstIteratorPixel maskIt = selection()->createHLineConstIterator(rc.x(), rc.y(), rc.width());

    for (int row = rc.y(); row < rc.bottom() + 1; ++row) {
        while (!projectionIt.isDone()) {

            int pixels = qMin(projectionIt.nConseqHPixels(), maskIt.nConseqHPixels());
            cs->applyInverseAlphaU8Mask(projectionIt.rawData(), maskIt.rawData(), pixels);

            projectionIt += pixels;
            maskIt += pixels;
        }
        projectionIt.nextRow();
        maskIt.nextRow();
    }

}

bool KisTransparencyMask::accept(KisNodeVisitor &v)
{
    dbgKrita << name() << " accepts visitor ";
    bool b = v.visit(this);
    dbgKrita << "result: " << b;
    return b;
}


#include "kis_transparency_mask.moc"
