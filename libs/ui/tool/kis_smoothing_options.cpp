/*
 *  SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_smoothing_options.h"

#include "kis_config.h"
#include "kis_signal_compressor.h"

struct KisSmoothingOptions::Private {
    Private(bool useSavedSmoothing)
        : writeCompressor(500, KisSignalCompressor::FIRST_ACTIVE)
    {
        KisConfig cfg(true);
        smoothingType = (SmoothingType)cfg.lineSmoothingType(!useSavedSmoothing);
        smoothnessDistance = cfg.lineSmoothingDistance(!useSavedSmoothing);
        tailAggressiveness = cfg.lineSmoothingTailAggressiveness(!useSavedSmoothing);
        smoothPressure = cfg.lineSmoothingSmoothPressure(!useSavedSmoothing);
        useScalableDistance = cfg.lineSmoothingScalableDistance(!useSavedSmoothing);
        delayDistance = cfg.lineSmoothingDelayDistance(!useSavedSmoothing);
        useDelayDistance = cfg.lineSmoothingUseDelayDistance(!useSavedSmoothing);
        finishStabilizedCurve = cfg.lineSmoothingFinishStabilizedCurve(!useSavedSmoothing);
        stabilizeSensors = cfg.lineSmoothingStabilizeSensors(!useSavedSmoothing);
    }

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
    : m_d(new Private(useSavedSmoothing))
{

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
    return m_d->smoothingType != STABILIZER ? m_d->useScalableDistance : true;
}

void KisSmoothingOptions::setUseScalableDistance(bool value)
{
    // stabilizer mush always have scalable distance on
    // see bug 421314
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->smoothingType != STABILIZER);

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
