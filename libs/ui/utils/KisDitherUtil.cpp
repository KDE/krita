/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2019 Carl Olsson <carl.olsson@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisDitherUtil.h"

#include <kis_filter_configuration.h>
#include <kis_random_generator.h>
#include <KisResourcesInterface.h>

KisDitherUtil::KisDitherUtil()
    : m_thresholdMode(ThresholdMode::Pattern), m_patternValueMode(PatternValueMode::Auto)
    , m_noiseSeed(0), m_patternUseAlpha(false), m_spread(1.0)
{
}

void KisDitherUtil::setThresholdMode(const ThresholdMode thresholdMode)
{
    m_thresholdMode = thresholdMode;
}

void KisDitherUtil::setPattern(const QString &md5sum, const QString &patternName, const PatternValueMode valueMode, KisResourcesInterfaceSP resourcesInterface)
{
    m_patternValueMode = valueMode;

    auto source = resourcesInterface->source<KoPattern>(ResourceType::Patterns);
    QVector<KoPatternSP> patterns = source.resources(md5sum, "", patternName);
    if (!patterns.isEmpty()) {
        m_pattern = patterns.first();
    }
    if (m_pattern && m_thresholdMode == ThresholdMode::Pattern && m_patternValueMode == PatternValueMode::Auto) {
        // Automatically pick between lightness-based and alpha-based patterns by whichever has maximum range
        qreal lightnessMin = 1.0, lightnessMax = 0.0;
        qreal alphaMin = 1.0, alphaMax = 0.0;
        const QImage &image = m_pattern->pattern();
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const QColor pixel = image.pixelColor(x, y);
                lightnessMin = std::min(lightnessMin, pixel.lightnessF());
                lightnessMax = std::max(lightnessMax, pixel.lightnessF());
                alphaMin = std::min(alphaMin, pixel.alphaF());
                alphaMax = std::max(alphaMax, pixel.alphaF());
            }
        }
        m_patternUseAlpha = (alphaMax - alphaMin > lightnessMax - lightnessMin);
    }
    else {
        m_patternUseAlpha = (m_patternValueMode == PatternValueMode::Alpha);
    }
}

void KisDitherUtil::setNoiseSeed(const quint64 &noiseSeed)
{
    m_noiseSeed = noiseSeed;
}

void KisDitherUtil::setSpread(const qreal &spread)
{
    m_spread = spread;
}

qreal KisDitherUtil::threshold(const QPoint &pos)
{
    qreal threshold;
    if (m_thresholdMode == ThresholdMode::Pattern && m_pattern) {
        const QImage &image = m_pattern->pattern();
        const QColor color = image.pixelColor(pos.x() % image.width(), pos.y() % image.height());
        threshold = (m_patternUseAlpha ? color.alphaF() : color.lightnessF());
    }
    else if (m_thresholdMode == ThresholdMode::Noise) {
        KisRandomGenerator random(m_noiseSeed);
        threshold = random.doubleRandomAt(pos.x(), pos.y());
    }
    else threshold = 0.5;

    return 0.5 - (m_spread / 2.0) + threshold * m_spread;
}

void KisDitherUtil::setConfiguration(const KisFilterConfiguration &config, const QString &prefix)
{
    setThresholdMode(ThresholdMode(config.getInt(prefix + "thresholdMode")));
    setPattern(config.getString(prefix + "md5sum"), config.getString(prefix + "pattern"), PatternValueMode(config.getInt(prefix + "patternValueMode")), config.resourcesInterface());
    setNoiseSeed(quint64(config.getInt(prefix + "noiseSeed")));
    setSpread(config.getDouble(prefix + "spread"));
}
