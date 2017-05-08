/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_PARTICLEOP_OPTION_H
#define KIS_PARTICLEOP_OPTION_H

#include <kis_paintop_option.h>

const QString PARTICLE_COUNT = "Particle/count";
const QString PARTICLE_GRAVITY = "Particle/gravity";
const QString PARTICLE_WEIGHT = "Particle/weight";
const QString PARTICLE_ITERATIONS = "Particle/iterations";
const QString PARTICLE_SCALE_X = "Particle/scaleX";
const QString PARTICLE_SCALE_Y = "Particle/scaleY";

class KisParticleOpOptionsWidget;
class KisPaintopLodLimitations;

class KisParticleOpOption : public KisPaintOpOption
{
public:
    KisParticleOpOption();
    ~KisParticleOpOption() override;

    int particleCount() const;
    qreal weight() const;
    qreal gravity() const;
    int iterations() const;
    QPointF scale() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

private:
    KisParticleOpOptionsWidget * m_options;
};

struct ParticleOption {
    int particle_count;
    int particle_iterations;
    qreal particle_gravity;
    qreal particle_weight;
    qreal particle_scale_x;
    qreal particle_scale_y;


    void readOptionSetting(const KisPropertiesConfigurationSP setting) {
        particle_count = setting->getInt(PARTICLE_COUNT);
        particle_iterations = setting->getInt(PARTICLE_ITERATIONS);
        particle_gravity = setting->getDouble(PARTICLE_GRAVITY);
        particle_weight = setting->getDouble(PARTICLE_WEIGHT);
        particle_scale_x = setting->getDouble(PARTICLE_SCALE_X);
        particle_scale_y = setting->getDouble(PARTICLE_SCALE_Y);
    }

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const {
        setting->setProperty(PARTICLE_COUNT, particle_count);
        setting->setProperty(PARTICLE_ITERATIONS, particle_iterations);
        setting->setProperty(PARTICLE_GRAVITY, particle_gravity);
        setting->setProperty(PARTICLE_WEIGHT, particle_weight);
        setting->setProperty(PARTICLE_SCALE_X, particle_scale_x);
        setting->setProperty(PARTICLE_SCALE_Y, particle_scale_y);
    }
};

#endif
