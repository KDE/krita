/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoSegmentGradient.h>
#include <KoColorSpace.h>

#include "KisGradientMapFilterNearestCachedGradient.h"

KisGradientMapFilterNearestCachedGradient::KisGradientMapFilterNearestCachedGradient(const KoStopGradientSP gradient, qint32 steps, const KoColorSpace *cs)
    : m_max(steps - 1)
    , m_black(KoColor(cs))
{
    for (qint32 i = 0; i < steps; i++) {
        qreal t = static_cast<qreal>(i) / m_max;
        KoGradientStop leftStop, rightStop;
        if (!gradient->stopsAt(leftStop, rightStop, t)) {
            m_colors << m_black;
        } else {
            if (std::abs(t - leftStop.position) < std::abs(t - rightStop.position)) {
                m_colors << leftStop.color.convertedTo(cs);
            } else {
                m_colors << rightStop.color.convertedTo(cs);
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
