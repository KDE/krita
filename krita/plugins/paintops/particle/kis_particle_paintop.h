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

#ifndef KIS_PARTICLE_PAINTOP_H_
#define KIS_PARTICLE_PAINTOP_H_

#include <kis_paintop.h>
#include <kis_types.h>

#include "kis_particle_paintop_settings.h"
#include "particle_brush.h"

class QPointF;
class KisPainter;
class KisPaintInformation;

class KisParticlePaintOp : public KisPaintOp
{

public:

    KisParticlePaintOp(const KisParticlePaintOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisParticlePaintOp();

    virtual void paintAt(const KisPaintInformation& info);
    virtual double paintLine(const KisPaintInformation& pi1, const KisPaintInformation& pi2, double savedDist = -1);
    
    virtual bool incremental() const {
        return true;
    }


    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const {
        Q_UNUSED(pressure1);
        Q_UNUSED(pressure2);
        xSpacing = 10;
        ySpacing = 10;
        return 10;
    }


private:
    const KisParticlePaintOpSettings* m_settings;
    KisParticleBrushProperties m_properties;
    KisPaintDeviceSP m_dab;
    ParticleBrush m_particleBrush;
    bool m_first;
};

#endif // KIS_PARTICLE_PAINTOP_H_
