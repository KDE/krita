/*
 *  Copyright (c) 2010 José Luis Vergara Toloza <pentalis@gmail.com>
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

#ifndef KIS_WATERYMIX_PAINTOP_H_
#define KIS_WATERYMIX_PAINTOP_H_

#include <kis_paintop.h>
#include <kis_brush_based_paintop.h>
#include <kis_types.h>

#include "kis_waterymix_paintop_settings.h"

#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>


class KisPainter;

struct HardcodedSettings
{
    qreal humidity;
    
    /** Fraction of the canvas color absorbed per pass
    * The color retained from the old cached dab is equal to
    * (1 - absorption).
    */
    qreal absortionrate;
};

class KisWateryMixPaintOp : public KisBrushBasedPaintOp
{

public:

    KisWateryMixPaintOp(const KisWateryMixPaintOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisWateryMixPaintOp();

    /**
     *  Document me
     */
    double paintAt(const KisPaintInformation& info);

    /*
    /// Absorb the color beneath the brush
    void absorb();
    
    /// Spread the absorbed color over the brush surface
    void dilute();
    */
    
private:
    KisWateryMixPaintOpSettings* m_settings;
    KisImageWSP m_image;
    KisPaintDeviceSP m_dab;

    /**
     *  Curve to control the opacity of the entire dab
     *  with device input
     */
    KisPressureOpacityOption m_opacityOption;
    
    /**
     *  Curve to control the size of the entire dab
     *  with device input
     */
    KisPressureSizeOption m_sizeOption;
    
    HardcodedSettings hardcodedSettings;
};

#endif // KIS_WATERYMIX_PAINTOP_H_
