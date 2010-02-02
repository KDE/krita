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
#include <krita_export.h>

const QString PARTICLE_COUNT = "Particle/count";
const QString PARTICLE_GRAVITY = "Particle/gravity";
const QString PARTICLE_WEIGHT = "Particle/weight";
const QString PARTICLE_ITERATIONS = "Particle/iterations";
const QString PARTICLE_SCALE_X = "Particle/scaleX";
const QString PARTICLE_SCALE_Y = "Particle/scaleY";

class KisParticleOpOptionsWidget;

class KisParticleOpOption : public KisPaintOpOption
{
public:
    KisParticleOpOption();
    ~KisParticleOpOption();

    int particleCount() const;
    qreal weight() const;
    qreal gravity() const;
    int iterations() const;
    QPointF scale() const;
    
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);


private:
   KisParticleOpOptionsWidget * m_options;
};

#endif
