/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoSegmentGradient.h>
#include <KoColorSpace.h>

#include "KisGradientMapFilterDitherCachedGradient.h"

KisGradientMapFilterDitherCachedGradient::KisGradientMapFilterDitherCachedGradient(const KoAbstractGradientSP gradient, qint32 steps, const KoColorSpace *cs)
    : m_max(steps - 1)
    , m_nullEntry(CachedEntry{KoColor(cs), KoColor(cs), 0.0})
{
    if (gradient.dynamicCast<KoStopGradient>()) {
        KoStopGradient *stopGradient = static_cast<KoStopGradient*>(gradient.data());
        for (qint32 i = 0; i < steps; i++) {
            qreal t = static_cast<qreal>(i) / m_max;
            KoGradientStop leftStop, rightStop;
            if (!stopGradient->stopsAt(leftStop, rightStop, t)) {
                m_cachedEntries << m_nullEntry;
            } else {
                const qreal localT = (t - leftStop.position) / (rightStop.position - leftStop.position);
                m_cachedEntries << CachedEntry{leftStop.color.convertedTo(cs), rightStop.color.convertedTo(cs), localT};
            }
        }
    } else if (gradient.dynamicCast<KoSegmentGradient>()) {
        KoSegmentGradient *segmentGradient = static_cast<KoSegmentGradient*>(gradient.data());
        for (qint32 i = 0; i < steps; i++) {
            qreal t = static_cast<qreal>(i) / m_max;
            KoGradientSegment *segment = segmentGradient->segmentAt(t);
            if (!segment) {
                m_cachedEntries << m_nullEntry;
            } else {
                const qreal localT = (t - segment->startOffset()) / (segment->endOffset() - segment->startOffset());
                m_cachedEntries << CachedEntry{segment->startColor().convertedTo(cs), segment->endColor().convertedTo(cs), localT};
            }
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
