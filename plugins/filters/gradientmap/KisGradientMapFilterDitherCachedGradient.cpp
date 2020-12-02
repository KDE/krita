/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoStopGradient.h>
#include <KoColorSpace.h>

#include "KisGradientMapFilterDitherCachedGradient.h"

KisGradientMapFilterDitherCachedGradient::KisGradientMapFilterDitherCachedGradient(const KoStopGradientSP gradient, qint32 steps, const KoColorSpace *cs)
    : m_max(steps - 1)
    , m_nullEntry(CachedEntry{KoColor(cs), KoColor(cs), 0.0})
{
    for (qint32 i = 0; i < steps; i++) {
        qreal t = static_cast<qreal>(i) / m_max;
        KoGradientStop leftStop, rightStop;
        if (!gradient->stopsAt(leftStop, rightStop, t)) {
            m_cachedEntries << m_nullEntry;
        } else {
            const qreal localT = (t - leftStop.position) / (rightStop.position - leftStop.position);
            m_cachedEntries << CachedEntry{leftStop.color.convertedTo(cs), rightStop.color.convertedTo(cs), localT};
        }
    }
}

const KisGradientMapFilterDitherCachedGradient::CachedEntry& KisGradientMapFilterDitherCachedGradient::cachedAt(qreal t) const
{
    qint32 tInt = t * m_max + 0.5;
    if (m_cachedEntries.size() > tInt) {
        return m_cachedEntries[tInt];
    } else {
        return m_nullEntry;
    }
}
