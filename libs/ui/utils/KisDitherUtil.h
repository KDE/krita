/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2019 Carl Olsson <carl.olsson@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_DITHER_UTIL_H
#define KIS_DITHER_UTIL_H

#include <kritaui_export.h>

#include <kis_types.h>

class KoPattern;
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
    void setPattern(const QString &name, const PatternValueMode valueMode);
    void setNoiseSeed(const quint64 &noiseSeed);
    void setSpread(const qreal &spread);

    qreal threshold(const QPoint &pos);

    void setConfiguration(const KisPropertiesConfiguration &config, const QString &prefix = "");

private:
    ThresholdMode m_thresholdMode;
    PatternValueMode m_patternValueMode;
    KoPattern* m_pattern;
    quint64 m_noiseSeed;
    bool m_patternUseAlpha;
    qreal m_spread;
};

#endif
