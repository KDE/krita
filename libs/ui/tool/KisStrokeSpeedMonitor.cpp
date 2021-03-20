/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisStrokeSpeedMonitor.h"

#include <QGlobalStatic>
#include <QMutex>
#include <QMutexLocker>

#include <KisRollingMeanAccumulatorWrapper.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "KisImageConfigNotifier.h"


Q_GLOBAL_STATIC(KisStrokeSpeedMonitor, s_instance)


struct KisStrokeSpeedMonitor::Private
{
    static const int averageWindow = 10;

    Private()
        : avgCursorSpeed(averageWindow),
          avgRenderingSpeed(averageWindow),
          avgFps(averageWindow)
    {
    }

    KisRollingMeanAccumulatorWrapper avgCursorSpeed;
    KisRollingMeanAccumulatorWrapper avgRenderingSpeed;
    KisRollingMeanAccumulatorWrapper avgFps;

    qreal cachedAvgCursorSpeed = 0;
    qreal cachedAvgRenderingSpeed = 0;
    qreal cachedAvgFps = 0;

    qreal lastCursorSpeed = 0;
    qreal lastRenderingSpeed = 0;
    qreal lastFps = 0;
    bool lastStrokeSaturated = false;

    QByteArray lastPresetMd5;
    QString lastPresetName;
    qreal lastPresetSize = 0;

    bool haveStrokeSpeedMeasurement = true;

    QMutex mutex;
};

KisStrokeSpeedMonitor::KisStrokeSpeedMonitor()
    : m_d(new Private())
{
    connect(KisImageConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(resetAccumulatedValues()));
    connect(KisImageConfigNotifier::instance(), SIGNAL(configChanged()), SIGNAL(sigStatsUpdated()));
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));

    slotConfigChanged();
}

KisStrokeSpeedMonitor::~KisStrokeSpeedMonitor()
{
}

KisStrokeSpeedMonitor *KisStrokeSpeedMonitor::instance()
{
    return s_instance;
}

bool KisStrokeSpeedMonitor::haveStrokeSpeedMeasurement() const
{
    return m_d->haveStrokeSpeedMeasurement;
}

void KisStrokeSpeedMonitor::setHaveStrokeSpeedMeasurement(bool value)
{
    m_d->haveStrokeSpeedMeasurement = value;
}

void KisStrokeSpeedMonitor::resetAccumulatedValues()
{
    m_d->avgCursorSpeed.reset(m_d->averageWindow);
    m_d->avgRenderingSpeed.reset(m_d->averageWindow);
    m_d->avgFps.reset(m_d->averageWindow);
}

void KisStrokeSpeedMonitor::slotConfigChanged()
{
    KisConfig cfg(true);
    m_d->haveStrokeSpeedMeasurement = cfg.enableBrushSpeedLogging();
    resetAccumulatedValues();
    emit sigStatsUpdated();
}

void KisStrokeSpeedMonitor::notifyStrokeFinished(qreal cursorSpeed, qreal renderingSpeed, qreal fps, KisPaintOpPresetSP preset)
{
    if (qFuzzyCompare(cursorSpeed, 0.0) || qFuzzyCompare(renderingSpeed, 0.0)) return;

    QMutexLocker locker(&m_d->mutex);

    const bool isSamePreset =
        m_d->lastPresetName == preset->name() &&
        qFuzzyCompare(m_d->lastPresetSize, preset->settings()->paintOpSize());

    //ENTER_FUNCTION() << ppVar(isSamePreset);

    if (!isSamePreset) {
        resetAccumulatedValues();
        m_d->lastPresetName = preset->name();
        m_d->lastPresetSize = preset->settings()->paintOpSize();
    }

    m_d->lastCursorSpeed = cursorSpeed;
    m_d->lastRenderingSpeed = renderingSpeed;
    m_d->lastFps = fps;


    static const qreal saturationSpeedThreshold = 0.30; // cursor speed should be at least 30% higher
    m_d->lastStrokeSaturated = cursorSpeed / renderingSpeed > (1.0 + saturationSpeedThreshold);


    if (m_d->lastStrokeSaturated) {
        m_d->avgCursorSpeed(cursorSpeed);
        m_d->avgRenderingSpeed(renderingSpeed);
        m_d->avgFps(fps);

        m_d->cachedAvgCursorSpeed = m_d->avgCursorSpeed.rollingMean();
        m_d->cachedAvgRenderingSpeed = m_d->avgRenderingSpeed.rollingMean();
        m_d->cachedAvgFps = m_d->avgFps.rollingMean();
    }

    emit sigStatsUpdated();


    ENTER_FUNCTION() <<
        QString(" CS: %1  RS: %2  FPS: %3 %4")
            .arg(m_d->lastCursorSpeed, 5)
            .arg(m_d->lastRenderingSpeed, 5)
            .arg(m_d->lastFps, 5)
            .arg(m_d->lastStrokeSaturated ? "(saturated)" : "");
    ENTER_FUNCTION() <<
        QString("ACS: %1 ARS: %2 AFPS: %3")
            .arg(m_d->cachedAvgCursorSpeed, 5)
            .arg(m_d->cachedAvgRenderingSpeed, 5)
            .arg(m_d->cachedAvgFps, 5);
}

QString KisStrokeSpeedMonitor::lastPresetName() const
{
    return m_d->lastPresetName;
}

qreal KisStrokeSpeedMonitor::lastPresetSize() const
{
    return m_d->lastPresetSize;
}

qreal KisStrokeSpeedMonitor::lastCursorSpeed() const
{
    return m_d->lastCursorSpeed;
}

qreal KisStrokeSpeedMonitor::lastRenderingSpeed() const
{
    return m_d->lastRenderingSpeed;
}

qreal KisStrokeSpeedMonitor::lastFps() const
{
    return m_d->lastFps;
}

bool KisStrokeSpeedMonitor::lastStrokeSaturated() const
{
    return m_d->lastStrokeSaturated;
}

qreal KisStrokeSpeedMonitor::avgCursorSpeed() const
{
    return m_d->cachedAvgCursorSpeed;
}

qreal KisStrokeSpeedMonitor::avgRenderingSpeed() const
{
    return m_d->cachedAvgRenderingSpeed;
}

qreal KisStrokeSpeedMonitor::avgFps() const
{
    return m_d->cachedAvgFps;
}
