/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2019 Carl Olsson <carl.olsson@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_DITHER_UTIL_H
#define KIS_DITHER_UTIL_H

#include <kritaui_export.h>

#include <kis_types.h>

#include <KoPattern.h>

class KisPropertiesConfiguration;

class KRITAUI_EXPORT KisDitherUtil
{
public:
    enum ThresholdMode {
        Pattern,
        Noise
    };
    enum PatternValueMode {
        Auto,
        Lightness,
        Alpha
    };

    KisDitherUtil();

    void setThresholdMode(const ThresholdMode thresholdMode);
    void setPattern(const QString &name, const PatternValueMode valueMode, KisResourcesInterfaceSP resourcesInterface);
    void setNoiseSeed(const quint64 &noiseSeed);
    void setSpread(const qreal &spread);

    qreal threshold(const QPoint &pos);

    void setConfiguration(const KisFilterConfiguration &config, const QString &prefix = "");

private:
    ThresholdMode m_thresholdMode;
    PatternValueMode m_patternValueMode;
    KoPatternSP m_pattern;
    quint64 m_noiseSeed;
    bool m_patternUseAlpha;
    qreal m_spread;
};

#endif
