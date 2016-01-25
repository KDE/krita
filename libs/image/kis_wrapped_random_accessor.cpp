/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_wrapped_random_accessor.h"

#include "kis_wrapped_rect.h"


KisWrappedRandomAccessor::KisWrappedRandomAccessor(KisTiledDataManager *ktm,
                                                   qint32 x, qint32 y,
                                                   qint32 offsetX, qint32 offsetY,
                                                   bool writable,
                                                   const QRect &wrapRect)
    : KisRandomAccessor2(ktm, x, y, offsetX, offsetY, writable),
      m_wrapRect(wrapRect),
      m_currentPos(x, y)
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
