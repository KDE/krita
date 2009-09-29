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

#include "kis_selection_mask.h"
#include "kis_selection.h"
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include "kis_fill_painter.h"
#include "kis_image.h"
#include <KoCompositeOp.h>
#include "kis_node_visitor.h"
#include "kis_pixel_selection.h"

class KisSelectionMask::Private
{
public:
    KisImageWSP image;
    KisSelectionSP deselectedSelection;
};

KisSelectionMask::KisSelectionMask(KisImageWSP image)
        : KisMask("selection")
        , m_d(new Private())
{
    m_d->image = image;
    m_d->deselectedSelection = 0;
}

KisSelectionMask::~KisSelectionMask()
{
    delete m_d;
}

bool KisSelectionMask::allowAsChild(KisNodeSP node) const
{
    Q_UNUSED(node);
    return false;
}


KisSelectionMask::KisSelectionMask(const KisSelectionMask& rhs)
        : KisMask(rhs)
        , m_d(new Private())
{
}

void KisSelectionMask::setSelection(KisSelectionSP selection)
{
    if (selection) {
        KisMask::setSelection(selection);
    } else {
        KisMask::setSelection(new KisSelection());

        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->alpha8();
        KisFillPainter gc(KisPaintDeviceSP(this->selection()->getOrCreatePixelSelection().data()));
        gc.fillRect(image()->bounds(), KoColor(Qt::white, cs), MAX_SELECTED);
        gc.end();
    }

    this->selection()->setInterestedInDirtyness(true);

    setDirty();
}

KisImageWSP KisSelectionMask::image() const
{
    return m_d->image;
}

bool KisSelectionMask::accept(KisNodeVisitor &v)
{
    return v.visit(this);
}

KisSelectionSP KisSelectionMask::deleselectedSelection()
{
    return m_d->deselectedSelection;
}

void KisSelectionMask::setDeleselectedSelection(KisSelectionSP selection)
{
    m_d->deselectedSelection = selection;
}

