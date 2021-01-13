/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRADIENT_MAP_FILTER_DITHER_CACHED_GRADIENT_H
#define KIS_GRADIENT_MAP_FILTER_DITHER_CACHED_GRADIENT_H

#include <QVector>

#include <KoColor.h>

class KoColorSpace;

class KisGradientMapFilterDitherCachedGradient
{
public:
    struct CachedEntry
    {
        KoColor leftStop;
        KoColor rightStop;
        qreal localT;
    };

    KisGradientMapFilterDitherCachedGradient(const KoAbstractGradientSP gradient, qint32 steps, const KoColorSpace *cs);

    /// gets the color data at position 0 <= t <= 1
    const CachedEntry& cachedAt(qreal t) const;

private:
    const qint32 m_max;
    QVector<CachedEntry> m_cachedEntries;
    const CachedEntry m_nullEntry;
};

#endif
