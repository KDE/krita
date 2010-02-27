/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_SOFT_PAINTOP_H_
#define KIS_SOFT_PAINTOP_H_

#include <kis_paintop.h>
#include <kis_types.h>
#include <kis_pressure_rotation_option.h>

#include "kis_soft_paintop_settings.h"

#include <KoColor.h>

#include "kis_alpha_mask.h"
#include "kis_curve_mask.h"

class KisPainter;
class KisCubicCurve;
class KisBrushSizeProperties;

struct KisGaussSoftBrush{
    KisCircleAlphaMask * distMask;

};

enum SoftBrushType{
    CURVE,
    GAUSS
};


class KisSoftPaintOp : public KisPaintOp
{

public:
    KisSoftPaintOp(const KisSoftPaintOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisSoftPaintOp();

    double paintAt(const KisPaintInformation& info);

    virtual bool incremental() const {
        return true;
    }
    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const;

private:
    const KisSoftPaintOpSettings* m_settings;
    KisImageWSP m_image;
    KisPaintDeviceSP m_dab;
    int m_radius;
    KoColor m_color;

    
    KisGaussSoftBrush m_gaussBrush;
    SoftBrushType m_brushType;
    
    KisBrushSizeProperties m_sizeProperties;
    KisCurveProperties m_curveMaskProperties;
    KisCurveMask m_curveMask;

    qreal m_xSpacing;
    qreal m_ySpacing;
    qreal m_spacing;

    KisPressureRotationOption m_rotationOption;    
};

#endif // KIS_SOFT_PAINTOP_H_
