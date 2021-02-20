/* This file is part of the KDE project
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_RANDOM_ACCESSOR_NG_H_
#define _KIS_RANDOM_ACCESSOR_NG_H_

#include "kis_base_accessor.h"

class KRITAIMAGE_EXPORT KisRandomConstAccessorNG : public KisBaseConstAccessor
{
    Q_DISABLE_COPY(KisRandomConstAccessorNG)
public:
    KisRandomConstAccessorNG() {}
    ~KisRandomConstAccessorNG() override;
    virtual void moveTo(qint32 x, qint32 y) = 0;
    virtual qint32 numContiguousColumns(qint32 x) const = 0;
    virtual qint32 numContiguousRows(qint32 y) const = 0;
    virtual qint32 rowStride(qint32 x, qint32 y) const = 0;
};

class KRITAIMAGE_EXPORT KisRandomAccessorNG : public KisRandomConstAccessorNG, public KisBaseAccessor
{
    Q_DISABLE_COPY(KisRandomAccessorNG)
public:
    KisRandomAccessorNG() {}
    ~KisRandomAccessorNG() override;
};

#endif
