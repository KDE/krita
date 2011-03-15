/*
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef _KIS_COLORSMUDGEOP_H_
#define _KIS_COLORSMUDGEOP_H_

#include <KoColorSpace.h>

#include <kis_brush_based_paintop.h>
#include <kis_types.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_spacing_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option.h>
#include <kis_merged_paint_option.h>

#include "kis_rate_option.h"

class KisBrushBasedPaintOpSettings;

class QPointF;
class KisPainter;

class KisColorSmudgeOp: public KisBrushBasedPaintOp
{
public:
    KisColorSmudgeOp(const KisBrushBasedPaintOpSettings* settings, KisPainter* painter, KisImageWSP image);
    virtual ~KisColorSmudgeOp();

    qreal paintAt(const KisPaintInformation& info);

private:
    bool                      m_firstRun;
    KisPaintDeviceSP          m_tempDev;
    KisImageWSP               m_image;
    KisPainter*               m_tempPainter;
    KisPressureSizeOption     m_sizeOption;
    KisPressureSpacingOption  m_spacingOption;
    KisRateOption             m_smudgeRateOption;
    KisRateOption             m_colorRateOption;
    KisMergedPaintOption      m_mergedPaintOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureScatterOption  m_scatterOption;
};

#endif // _KIS_COLORSMUDGEOP_H_
