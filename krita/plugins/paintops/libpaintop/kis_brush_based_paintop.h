/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_BRUSH_BASED_PAINTOP_H
#define KIS_BRUSH_BASED_PAINTOP_H

#include "krita_export.h"
#include "kis_paintop.h"
#include "kis_dab_cache.h"
#include "kis_brush.h"
#include "kis_texture_option.h"
#include "kis_precision_option.h"


class KisPropertiesConfiguration;
class KisPressureSpacingOption;

/**
 * This is a base class for paintops that use a KisBrush or derived
 * brush to paint with. This is mainly important for the spacing
 * generation.
 */
class PAINTOP_EXPORT KisBrushBasedPaintOp : public KisPaintOp
{

public:

    KisBrushBasedPaintOp(const KisPropertiesConfiguration* settings, KisPainter* painter);
    ~KisBrushBasedPaintOp();
    double spacing(double scale) const;

    KisSpacingInformation effectiveSpacing(int dabWidth, int dabHeight) const;
    KisSpacingInformation effectiveSpacing(int dabWidth, int dabHeight, const KisPressureSpacingOption &spacingOption, const KisPaintInformation &pi) const;
    KisSpacingInformation effectiveSpacing(int dabWidth, int dabHeight, qreal extraScale, bool isotropicSpacing) const;

    ///Reimplemented, false if brush is 0
    virtual bool canPaint() const;

protected: // XXX: make private!

    KisBrushSP m_brush;
    KisTextureProperties m_textureProperties;
    KisPrecisionOption m_precisionOption;
    KisDabCache *m_dabCache;
};

#endif
