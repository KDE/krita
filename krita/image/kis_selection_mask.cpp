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
#include "kis_fill_painter.h"
#include <KoCompositeOp.h>

class KisSelectionMask::Private
{
public:
    KisSelectionSP selection;
};

KisSelectionMask::KisSelectionMask()
    : KisMask( "selection" )
    , m_d( new Private() )
{
}

KisSelectionMask::~KisSelectionMask()
{
    delete m_d;
}

bool KisSelectionMask::allowAsChild( KisNodeSP node)
{
    Q_UNUSED(node);
    return false;
}


KisSelectionMask::KisSelectionMask( const KisSelectionMask& rhs )
    : KisMask( rhs )
    , m_d( new Private() )
{
    // Deep copy.
    m_d->selection = new KisSelection( rhs.m_d->selection.data() );
}

KisSelectionSP KisSelectionMask::selection() const
{
    return m_d->selection;
}

void KisSelectionMask::setSelection(KisSelectionSP selection)
{
    m_d->selection = new KisSelection();
    KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    KisFillPainter gc(KisPaintDeviceSP(m_d->selection.data()));

    if (selection) {
        gc.bitBlt(0, 0, cs->compositeOp(COMPOSITE_COPY), KisPaintDeviceSP(selection.data()),
                  0, 0, image()->bounds().width(), image()->bounds().height());
    } else {
        gc.fillRect(image()->bounds(), KoColor(Qt::white, cs), MAX_SELECTED);
    }

    gc.end();

    m_d->selection->setInterestedInDirtyness(true);

    if (!m_d->selection->hasSelection())
        m_d->selection->setSelection(m_d->selection);

    setDirty();
}


qint32 KisSelectionMask::x() const
{
    if (m_d->selection)
        return m_d->selection->x();
    else
        return 0;
}

void KisSelectionMask::setX(qint32 x)
{
    if (m_d->selection) {
        m_d->selection->setX(x);
    }

}

qint32 KisSelectionMask::y() const
{
    if (m_d->selection)
        return m_d->selection->y();
    else
        return 0;
}

void KisSelectionMask::setY(qint32 y)
{
    if (m_d->selection) {
        m_d->selection->setY(y);
    }
}

QRect KisSelectionMask::extent() const
{
    if (m_d->selection)
        return m_d->selection->selectedRect();
    else if (image() )
        return image()->bounds();
    else
        return QRect();
}

QRect KisSelectionMask::exactBounds() const
{
    if (m_d->selection)
        return m_d->selection->selectedExactRect();
    else if (image())
        return image()->bounds();
    else
        return QRect();
}

KisImageSP KisSelectionMask::image() const
{
    if ( m_d->selection )
        return m_d->selection->image();
    else
        return 0;
}
