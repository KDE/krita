/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wrapped_random_accessor.h"

#include "kis_wrapped_rect.h"


KisWrappedRandomAccessor::KisWrappedRandomAccessor(KisTiledDataManager *ktm,
                                                   qint32 offsetX, qint32 offsetY,
                                                   bool writable,
                                                   KisIteratorCompleteListener *completeListener,
                                                   const QRect &wrapRect)
    : KisRandomAccessor2(ktm, offsetX, offsetY, writable, completeListener),
      m_wrapRect(wrapRect),
      m_currentPos(QPoint())
{
}

void KisWrappedRandomAccessor::moveTo(qint32 x, qint32 y)
{
    m_currentPos = QPoint(x, y);

    x = KisWrappedRect::xToWrappedX(x, m_wrapRect);
    y = KisWrappedRect::yToWrappedY(y, m_wrapRect);

    KisRandomAccessor2::moveTo(x, y);
}

qint32 KisWrappedRandomAccessor::numContiguousColumns(qint32 x) const
{
    x = KisWrappedRect::xToWrappedX(x, m_wrapRect);
    qint32 distanceToBorder = m_wrapRect.x() + m_wrapRect.width() - x;

    return qMin(distanceToBorder, KisRandomAccessor2::numContiguousColumns(x));
}

qint32 KisWrappedRandomAccessor::numContiguousRows(qint32 y) const
{
    y = KisWrappedRect::yToWrappedY(y, m_wrapRect);
    qint32 distanceToBorder = m_wrapRect.y() + m_wrapRect.height() - y;

    return qMin(distanceToBorder, KisRandomAccessor2::numContiguousRows(y));
}

qint32 KisWrappedRandomAccessor::rowStride(qint32 x, qint32 y) const
{
    x = KisWrappedRect::xToWrappedX(x, m_wrapRect);
    y = KisWrappedRect::yToWrappedY(y, m_wrapRect);
    return KisRandomAccessor2::rowStride(x, y);
}

qint32 KisWrappedRandomAccessor::x() const
{
    return m_currentPos.x();
}

qint32 KisWrappedRandomAccessor::y() const
{
    return m_currentPos.y();
}
