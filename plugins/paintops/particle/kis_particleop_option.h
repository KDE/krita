/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
