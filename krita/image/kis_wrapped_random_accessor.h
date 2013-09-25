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

#ifndef __KIS_WRAPPED_RANDOM_ACCESSOR_H
#define __KIS_WRAPPED_RANDOM_ACCESSOR_H

#include "tiles3/kis_random_accessor.h"


class KisWrappedRandomAccessor : public KisRandomAccessor2
{
public:
    KisWrappedRandomAccessor(KisTiledDataManager *ktm,
                             qint32 x, qint32 y,
                             qint32 offsetX, qint32 offsetY,
                             bool writable,
                             const QRect &wrapRect);

    void moveTo(qint32 x, qint32 y);
    qint32 numContiguousColumns(qint32 x) const;
    qint32 numContiguousRows(qint32 y) const;
    qint32 rowStride(qint32 x, qint32 y) const;

    qint32 x() const;
    qint32 y() const;

private:
    QRect m_wrapRect;
    QPoint m_currentPos;
};

#endif /* __KIS_WRAPPED_RANDOM_ACCESSOR_H */
