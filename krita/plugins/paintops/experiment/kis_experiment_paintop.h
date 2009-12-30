/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef KIS_EXPERIMENT_PAINTOP_H_
#define KIS_EXPERIMENT_PAINTOP_H_

#include <klocale.h>
#include <kis_paintop.h>
#include <kis_types.h>

#include "kis_experiment_paintop_settings.h"

class QPointF;
class KisPainter;

class KisExperimentPaintOp : public KisPaintOp
{

public:

    KisExperimentPaintOp(const KisExperimentPaintOpSettings *settings, KisPainter * painter, KisImageSP image);
    virtual ~KisExperimentPaintOp();

    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const;
    void paintAt(const KisPaintInformation& info);

    virtual bool incremental() const {
        return false;
    }

private:
    const KisExperimentPaintOpSettings* m_settings;
    KisImageSP m_image;

    KisPaintDeviceSP m_dab;
    KisPaintDeviceSP m_dev;
    KisPaintDeviceSP m_oldData;
    
    int m_size;
   
    double m_xSpacing, m_ySpacing, m_spacing;
    QRect m_previousDab;
    bool m_isFirst;
    
    KisPainter * m_painter;
    
    QVector<KisPaintInformation> m_positions;
    qreal m_startSize;
    qreal m_endSize;
    KisPressureRotationOption m_rotationOption;
    KisPressureSizeOption m_sizeOption;
    KisPressureOpacityOption m_opacityOption;
};

#endif // KIS_EXPERIMENT_PAINTOP_H_
