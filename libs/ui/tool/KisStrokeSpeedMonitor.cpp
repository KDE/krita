/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisStrokeSpeedMonitor.h"

#include <QGlobalStatic>
#include <QMutex>
#include <QMutexLocker>

#include <KisRollingMeanAccumulatorWrapper.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"

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

    qreal lastCursorSpeed = 0;
    qreal lastRenderingSpeed = 0;
    qreal lastFps = 0;

    QByteArray lastPresetMd5;
    QString lastPresetName;
    qreal lastPresetSize = 0;

    bool haveStrokeSpeedMeasurement = false;

    QMutex mutex;
};

KisStrokeSpeedMonitor::KisStrokeSpeedMonitor()
    : m_d(new Private())
{
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

void KisStrokeSpeedMonitor::notifyStrokeFinished(qreal cursorSpeed, qreal renderingSpeed, qreal fps, KisPaintOpPresetSP preset)
{
    if (qFuzzyCompare(cursorSpeed, 0.0) || qFuzzyCompare(renderingSpeed, 0.0)) return;

    QMutexLocker locker(&m_d->mutex);

    const bool isSamePreset =
        m_d->lastPresetName == preset->name() &&
        qFuzzyCompare(m_d->lastPresetSize, preset->settings()->paintOpSize());

    ENTER_FUNCTION() << ppVar(isSamePreset);

    if (!isSamePreset) {
        m_d->avgCursorSpeed.reset(m_d->averageWindow);
        m_d->avgRenderingSpeed.reset(m_d->averageWindow);
        m_d->avgFps.reset(m_d->averageWindow);
        m_d->lastPresetName = preset->name();
        m_d->lastPresetSize = preset->settings()->paintOpSize();
    }

    m_d->lastCursorSpeed = cursorSpeed;
    m_d->lastRenderingSpeed = renderingSpeed;
    m_d->lastFps = fps;


    static const qreal saturationSpeedThreshold = 0.30; // cursor speed should be at least 30% higher
    const bool isStrokeSturated = cursorSpeed / renderingSpeed > (1.0 + saturationSpeedThreshold);


    if (isStrokeSturated) {
        m_d->avgCursorSpeed(cursorSpeed);
        m_d->avgRenderingSpeed(renderingSpeed);
        m_d->avgFps(fps);
    }


    ENTER_FUNCTION() <<
        QString(" CS: %1  RS: %2  FPS: %3 %4")
            .arg(m_d->lastCursorSpeed, 5)
            .arg(m_d->lastRenderingSpeed, 5)
            .arg(m_d->lastFps, 5)
            .arg(isStrokeSturated ? "(saturated)" : "");
    ENTER_FUNCTION() <<
        QString("ACS: %1 ARS: %2 AFPS: %3")
            .arg(m_d->avgCursorSpeed.rollingMean(), 5)
            .arg(m_d->avgRenderingSpeed.rollingMean(), 5)
                        .arg(m_d->avgFps.rollingMean(), 5);
}
