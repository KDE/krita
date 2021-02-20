/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_WRAPPED_RANDOM_ACCESSOR_H
#define __KIS_WRAPPED_RANDOM_ACCESSOR_H

#include "tiles3/kis_random_accessor.h"


class KisWrappedRandomAccessor : public KisRandomAccessor2
{
public:
    KisWrappedRandomAccessor(KisTiledDataManager *ktm,
                             qint32 offsetX, qint32 offsetY,
                             bool writable,
                             KisIteratorCompleteListener *completeListener,
                             const QRect &wrapRect);

    void moveTo(qint32 x, qint32 y) override;
    qint32 numContiguousColumns(qint32 x) const override;
    qint32 numContiguousRows(qint32 y) const override;
    qint32 rowStride(qint32 x, qint32 y) const override;

    qint32 x() const override;
    qint32 y() const override;

private:
    QRect m_wrapRect;
    QPoint m_currentPos;
};

#endif /* __KIS_WRAPPED_RANDOM_ACCESSOR_H */
