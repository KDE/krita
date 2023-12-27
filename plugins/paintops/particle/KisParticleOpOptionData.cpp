/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisParticleOpOptionData.h"

#include "kis_properties_configuration.h"
#include <kis_paintop_lod_limitations.h>


const QString PARTICLE_COUNT = "Particle/count";
const QString PARTICLE_GRAVITY = "Particle/gravity";
const QString PARTICLE_WEIGHT = "Particle/weight";
const QString PARTICLE_ITERATIONS = "Particle/iterations";
const QString PARTICLE_SCALE_X = "Particle/scaleX";
const QString PARTICLE_SCALE_Y = "Particle/scaleY";


bool KisParticleOpOptionData::read(const KisPropertiesConfiguration *setting)
{
    particleCount = setting->getInt(PARTICLE_COUNT, 50);
    particleIterations = setting->getInt(PARTICLE_ITERATIONS, 10);
    particleGravity = setting->getDouble(PARTICLE_GRAVITY, 0.989);
    particleWeight = setting->getDouble(PARTICLE_WEIGHT, 0.2);
    particleScaleX = setting->getDouble(PARTICLE_SCALE_X, 0.3);
    particleScaleY = setting->getDouble(PARTICLE_SCALE_Y, 0.3);

    return true;
}

void KisParticleOpOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(PARTICLE_COUNT, particleCount);
    setting->setProperty(PARTICLE_ITERATIONS, particleIterations);
    setting->setProperty(PARTICLE_GRAVITY, particleGravity);
    setting->setProperty(PARTICLE_WEIGHT, particleWeight);
    setting->setProperty(PARTICLE_SCALE_X, particleScaleX);
    setting->setProperty(PARTICLE_SCALE_Y, particleScaleY);
}

KisPaintopLodLimitations KisParticleOpOptionData::lodLimitations() const
{
    KisPaintopLodLimitations l;
    l.blockers << KoID("particle-brush", i18nc("PaintOp instant preview limitation", "Particle Brush (not supported)"));
    return l;
}
