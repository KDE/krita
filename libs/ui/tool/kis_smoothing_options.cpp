/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_smoothing_options.h"

#include "kis_config.h"
#include "kis_signal_compressor.h"

struct KisSmoothingOptions::Private {
    Private() : writeCompressor(500, KisSignalCompressor::FIRST_ACTIVE) {}
    KisSignalCompressor writeCompressor;

    SmoothingType smoothingType;
    qreal smoothnessDistance;
    qreal tailAggressiveness;
    bool smoothPressure;
    bool useScalableDistance;
    qreal delayDistance;
    bool useDelayDistance;
    bool finishStabilizedCurve;
    bool stabilizeSensors;
};

KisSmoothingOptions::KisSmoothingOptions(bool useSavedSmoothing)
    : m_d(new Private)
{
    KisConfig cfg(true);
    m_d->smoothingType = (SmoothingType)cfg.lineSmoothingType(!useSavedSmoothing);
    m_d->smoothnessDistance = cfg.lineSmoothingDistance(!useSavedSmoothing);
    m_d->tailAggressiveness = cfg.lineSmoothingTailAggressiveness(!useSavedSmoothing);
    m_d->smoothPressure = cfg.lineSmoothingSmoothPressure(!useSavedSmoothing);
    m_d->useScalableDistance = cfg.lineSmoothingScalableDistance(!useSavedSmoothing);
    m_d->delayDistance = cfg.lineSmoothingDelayDistance(!useSavedSmoothing);
    m_d->useDelayDistance = cfg.lineSmoothingUseDelayDistance(!useSavedSmoothing);
    m_d->finishStabilizedCurve = cfg.lineSmoothingFinishStabilizedCurve(!useSavedSmoothing);
    m_d->stabilizeSensors = cfg.lineSmoothingStabilizeSensors(!useSavedSmoothing);

    connect(&m_d->writeCompressor, SIGNAL(timeout()), this, SLOT(slotWriteConfig()));
}

KisSmoothingOptions::~KisSmoothingOptions()
{
}

KisSmoothingOptions::SmoothingType KisSmoothingOptions::smoothingType() const
{
    return m_d->smoothingType;
}

void KisSmoothingOptions::setSmoothingType(KisSmoothingOptions::SmoothingType value)
{
    m_d->smoothingType = value;
    emit sigSmoothingTypeChanged();
    m_d->writeCompressor.start();
}

qreal KisSmoothingOptions::smoothnessDistance() const
{
    return m_d->smoothnessDistance;
}

void KisSmoothingOptions::setSmoothnessDistance(qreal value)
{
    m_d->smoothnessDistance = value;
    m_d->writeCompressor.start();
}

qreal KisSmoothingOptions::tailAggressiveness() const
{
    return m_d->tailAggressiveness;
}

void KisSmoothingOptions::setTailAggressiveness(qreal value)
{
    m_d->tailAggressiveness = value;
    m_d->writeCompressor.start();
}

bool KisSmoothingOptions::smoothPressure() const
{
    return m_d->smoothPressure;
}

void KisSmoothingOptions::setSmoothPressure(bool value)
{
    m_d->smoothPressure = value;
    m_d->writeCompressor.start();
}

bool KisSmoothingOptions::useScalableDistance() const
{
    return m_d->useScalableDistance;
}

void KisSmoothingOptions::setUseScalableDistance(bool value)
{
    m_d->useScalableDistance = value;
    m_d->writeCompressor.start();
}

qreal KisSmoothingOptions::delayDistance() const
{
    return m_d->delayDistance;
}

void KisSmoothingOptions::setDelayDistance(qreal value)
{
    m_d->delayDistance = value;
    m_d->writeCompressor.start();
}

bool KisSmoothingOptions::useDelayDistance() const
{
    return m_d->useDelayDistance;
}

void KisSmoothingOptions::setUseDelayDistance(bool value)
{
    m_d->useDelayDistance = value;
    m_d->writeCompressor.start();
}

void KisSmoothingOptions::setFinishStabilizedCurve(bool value)
{
    m_d->finishStabilizedCurve = value;
    m_d->writeCompressor.start();
}

bool KisSmoothingOptions::finishStabilizedCurve() const
{
    return m_d->finishStabilizedCurve;
}

void KisSmoothingOptions::setStabilizeSensors(bool value)
{
    m_d->stabilizeSensors = value;
    m_d->writeCompressor.start();
}

bool KisSmoothingOptions::stabilizeSensors() const
{
    return m_d->stabilizeSensors;
}

void KisSmoothingOptions::slotWriteConfig()
{
    KisConfig cfg(false);
    cfg.setLineSmoothingType(m_d->smoothingType);
    cfg.setLineSmoothingDistance(m_d->smoothnessDistance);
    cfg.setLineSmoothingTailAggressiveness(m_d->tailAggressiveness);
    cfg.setLineSmoothingSmoothPressure(m_d->smoothPressure);
    cfg.setLineSmoothingScalableDistance(m_d->useScalableDistance);
    cfg.setLineSmoothingDelayDistance(m_d->delayDistance);
    cfg.setLineSmoothingUseDelayDistance(m_d->useDelayDistance);
    cfg.setLineSmoothingFinishStabilizedCurve(m_d->finishStabilizedCurve);
    cfg.setLineSmoothingStabilizeSensors(m_d->stabilizeSensors);
}
