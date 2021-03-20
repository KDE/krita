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

#include "KisGradientMapFilterNearestCachedGradient.h"

KisGradientMapFilterNearestCachedGradient::KisGradientMapFilterNearestCachedGradient(const KoAbstractGradientSP gradient, qint32 steps, const KoColorSpace *cs)
    : m_max(steps - 1)
    , m_black(KoColor(cs))
{
    if (dynamic_cast<KoStopGradient*>(gradient.data())) {
        KoStopGradient *stopGradient = static_cast<KoStopGradient*>(gradient.data());
        for (qint32 i = 0; i < steps; i++) {
            qreal t = static_cast<qreal>(i) / m_max;
            KoGradientStop leftStop, rightStop;
            if (!stopGradient->stopsAt(leftStop, rightStop, t)) {
                m_colors << m_black;
            } else {
                if (std::abs(t - leftStop.position) < std::abs(t - rightStop.position)) {
                    m_colors << leftStop.color.convertedTo(cs);
                } else {
                    m_colors << rightStop.color.convertedTo(cs);
                }
            }
        }
    } else if (dynamic_cast<KoSegmentGradient*>(gradient.data())) {
        KoSegmentGradient *segmentGradient = static_cast<KoSegmentGradient*>(gradient.data());
        for (qint32 i = 0; i < steps; i++) {
            qreal t = static_cast<qreal>(i) / m_max;
            KoGradientSegment *segment = segmentGradient->segmentAt(t);
            if (!segment) {
                m_colors << m_black;
            } else {
                if (std::abs(t - segment->startOffset()) < std::abs(t - segment->endOffset())) {
                    m_colors << segment->startColor().convertedTo(cs);
                } else {
                    m_colors << segment->endColor().convertedTo(cs);
                }
            }
        }
    }

}

const quint8* KisGradientMapFilterNearestCachedGradient::cachedAt(qreal t) const
{
    qint32 tInt = t * m_max + 0.5;
    if (m_colors.size() > tInt) {
        return m_colors[tInt].data();
    } else {
        return m_black.data();
    }
}
