/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisLayerStyleKnockoutBlower.h"

#include "kis_painter.h"
#include "KoCompositeOpRegistry.h"

KisLayerStyleKnockoutBlower::KisLayerStyleKnockoutBlower()
{
}

KisLayerStyleKnockoutBlower::KisLayerStyleKnockoutBlower(const KisLayerStyleKnockoutBlower &rhs)
    : m_knockoutSelection(rhs.m_knockoutSelection ? new KisSelection(*rhs.m_knockoutSelection) : nullptr)
{
}

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
    painter->setSelection(0);
}

bool KisLayerStyleKnockoutBlower::isEmpty() const
{
    QReadLocker l(&m_lock);
    return !m_knockoutSelection;
}
