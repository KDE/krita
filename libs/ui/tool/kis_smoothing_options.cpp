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

KisSmoothingOptions::KisSmoothingOptions()
{
    KisConfig cfg;
    m_smoothingType = (SmoothingType)cfg.lineSmoothingType();
    m_smoothnessDistance = cfg.lineSmoothingDistance();
    m_tailAggressiveness = cfg.lineSmoothingTailAggressiveness();
    m_smoothPressure = cfg.lineSmoothingSmoothPressure();
    m_useScalableDistance = cfg.lineSmoothingScalableDistance();
    m_delayDistance = cfg.lineSmoothingDelayDistance();
    m_useDelayDistance = cfg.lineSmoothingUseDelayDistance();
    m_finishStabilizedCurve = cfg.lineSmoothingFinishStabilizedCurve();
    m_stabilizeSensors = cfg.lineSmoothingStabilizeSensors();
}

KisSmoothingOptions::SmoothingType KisSmoothingOptions::smoothingType() const
{
    return m_smoothingType;
}

void KisSmoothingOptions::setSmoothingType(KisSmoothingOptions::SmoothingType value)
{
    KisConfig cfg;
    cfg.setLineSmoothingType(value);
    m_smoothingType = value;
}

qreal KisSmoothingOptions::smoothnessDistance() const
{
    return m_smoothnessDistance;
}

void KisSmoothingOptions::setSmoothnessDistance(qreal value)
{
    KisConfig cfg;
    cfg.setLineSmoothingDistance(value);
    m_smoothnessDistance = value;
}

qreal KisSmoothingOptions::tailAggressiveness() const
{
    return m_tailAggressiveness;
}

void KisSmoothingOptions::setTailAggressiveness(qreal value)
{
    KisConfig cfg;
    cfg.setLineSmoothingTailAggressiveness(value);
    m_tailAggressiveness = value;
}

bool KisSmoothingOptions::smoothPressure() const
{
    return m_smoothPressure;
}

void KisSmoothingOptions::setSmoothPressure(bool value)
{
    KisConfig cfg;
    cfg.setLineSmoothingSmoothPressure(value);
    m_smoothPressure = value;
}

bool KisSmoothingOptions::useScalableDistance() const
{
    return m_useScalableDistance;
}

void KisSmoothingOptions::setUseScalableDistance(bool value)
{
    KisConfig cfg;
    cfg.setLineSmoothingScalableDistance(value);
    m_useScalableDistance = value;
}

qreal KisSmoothingOptions::delayDistance() const
{
    return m_delayDistance;
}

void KisSmoothingOptions::setDelayDistance(qreal value)
{
    KisConfig cfg;
    cfg.setLineSmoothingDelayDistance(value);
    m_delayDistance = value;
}

bool KisSmoothingOptions::useDelayDistance() const
{
    return m_useDelayDistance;
}

void KisSmoothingOptions::setUseDelayDistance(bool value)
{
    KisConfig cfg;
    cfg.setLineSmoothingUseDelayDistance(value);
    m_useDelayDistance = value;
}

void KisSmoothingOptions::setFinishStabilizedCurve(bool value)
{
    KisConfig cfg;
    cfg.setLineSmoothingFinishStabilizedCurve(value);
    m_finishStabilizedCurve = value;
}

bool KisSmoothingOptions::finishStabilizedCurve() const
{
    return m_finishStabilizedCurve;
}

void KisSmoothingOptions::setStabilizeSensors(bool value)
{
    KisConfig cfg;
    cfg.setLineSmoothingStabilizeSensors(value);
    m_stabilizeSensors = value;
}

bool KisSmoothingOptions::stabilizeSensors() const
{
    return m_stabilizeSensors;
}
