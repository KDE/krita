/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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
#include "KisLayerStyleKnockoutBlower.h"

#include "kis_painter.h"
#include "KoCompositeOpRegistry.h"

KisSelectionSP KisLayerStyleKnockoutBlower::knockoutSelectionLazy()
{
    {
        QReadLocker l(&m_lock);
        if (m_knockoutSelection) {
            return m_knockoutSelection;
        }
    }

    {
        QWriteLocker l(&m_lock);
        if (m_knockoutSelection) {
            return m_knockoutSelection;
        } else {
            m_knockoutSelection = new KisSelection(new KisSelectionEmptyBounds(0));
            return m_knockoutSelection;
        }
    }
}

void KisLayerStyleKnockoutBlower::setKnockoutSelection(KisSelectionSP selection)
{
    QWriteLocker l(&m_lock);
    m_knockoutSelection = selection;
}

void KisLayerStyleKnockoutBlower::resetKnockoutSelection()
{
    QWriteLocker l(&m_lock);
    m_knockoutSelection = 0;
}

void KisLayerStyleKnockoutBlower::apply(KisPainter *painter, KisPaintDeviceSP mergedStyle, const QRect &rect) const
{
    QReadLocker l(&m_lock);

    KIS_SAFE_ASSERT_RECOVER_NOOP(m_knockoutSelection);

    painter->setOpacity(OPACITY_OPAQUE_U8);
    painter->setChannelFlags(QBitArray());
    painter->setCompositeOp(COMPOSITE_COPY);
    painter->setSelection(m_knockoutSelection);
    painter->bitBlt(rect.topLeft(), mergedStyle, rect);
}

bool KisLayerStyleKnockoutBlower::isEmpty() const
{
    QReadLocker l(&m_lock);
    return !m_knockoutSelection;
}
