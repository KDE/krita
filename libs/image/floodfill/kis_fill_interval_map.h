/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FILL_INTERVAL_MAP_H
#define __KIS_FILL_INTERVAL_MAP_H

#include <QMap>
#include <QStack>
#include <QScopedPointer>
#include "kritaimage_export.h"
#include "kis_fill_interval.h"


class KRITAIMAGE_EXPORT KisFillIntervalMap
{
public:

public:
    KisFillIntervalMap();
    ~KisFillIntervalMap();

    void insertInterval(const KisFillInterval &interval);
    void cropInterval(KisFillInterval *interval);

    QStack<KisFillInterval> fetchAllIntervals(int rowCorrection = 0) const;
    void clear();

private:
    friend class KisFillIntervalMapTest;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_FILL_INTERVAL_MAP_H */
