/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_SPRAY_PAINTOP_H_
#define KIS_SPRAY_PAINTOP_H_

//#define BENCHMARK

#include <kis_paintop.h>
#include <kis_types.h>

#include "spray_brush.h"
#include "kis_spray_paintop_settings.h"

class QPointF;
class KisPainter;
//class KisColorProperties;

class KisSprayPaintOp : public KisPaintOp
{

public:

    KisSprayPaintOp(const KisSprayPaintOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisSprayPaintOp();

    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const;
    void paintAt(const KisPaintInformation& info);

    virtual bool incremental() const {
        return true;
    }

private:
    KisSprayProperties m_properties;
    KisColorProperties m_colorProperties;
    
    const KisSprayPaintOpSettings *m_settings;

    KisImageWSP m_image;
    KisPaintDeviceSP m_dab;
    SprayBrush m_sprayBrush;
    double m_xSpacing, m_ySpacing, m_spacing;
    KisPressureRotationOption m_rotationOption;
    KisPressureSizeOption m_sizeOption;
    KisPressureOpacityOption m_opacityOption;
    
    void loadSettings(const KisSprayPaintOpSettings *settings);
#ifdef BENCHMARK
    int m_total;
    int m_count;
#endif

};

#endif // KIS_SPRAY_PAINTOP_H_
