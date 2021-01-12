/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_MAP_FILTER_NEAREST_CACHED_GRADIENT_H
#define KIS_GRADIENT_MAP_FILTER_NEAREST_CACHED_GRADIENT_H

#include <QVector>

#include <KoColor.h>

class KoColorSpace;

class KisGradientMapFilterNearestCachedGradient
{
public:
    KisGradientMapFilterNearestCachedGradient(const KoAbstractGradientSP gradient, qint32 steps, const KoColorSpace *cs);

    /// gets the color data at position 0 <= t <= 1
    const quint8* cachedAt(qreal t) const;

private:
    const qint32 m_max;
    QVector<KoColor> m_colors;
    const KoColor m_black;
};


#endif
