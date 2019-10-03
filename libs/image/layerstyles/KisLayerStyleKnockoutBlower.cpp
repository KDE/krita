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

KisSelectionSP KisLayerStyleKnockoutBlower::knockoutSelection() const
{
    return m_knockoutSelection;
}

void KisLayerStyleKnockoutBlower::setKnockoutSelection(KisSelectionSP selection)
{
    m_knockoutSelection = selection;
}

void KisLayerStyleKnockoutBlower::apply(KisPainter *painter, KisPaintDeviceSP mergedStyle, const QRect &rect) const
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(m_knockoutSelection);

    painter->setOpacity(OPACITY_OPAQUE_U8);
    painter->setChannelFlags(QBitArray());
    painter->setCompositeOp(COMPOSITE_COPY);
    painter->setSelection(m_knockoutSelection);
    painter->bitBlt(rect.topLeft(), mergedStyle, rect);
}

bool KisLayerStyleKnockoutBlower::isEmpty() const {
    return !m_knockoutSelection;
}
