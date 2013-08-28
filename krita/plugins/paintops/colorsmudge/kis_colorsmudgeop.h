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

#include <QRect>
#include <KoColorSpace.h>

#include <kis_brush_based_paintop.h>
#include <kis_types.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_spacing_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option.h>
#include <kis_pressure_gradient_option.h>

#include "kis_overlay_mode_option.h"
#include "kis_rate_option.h"
#include "kis_smudge_option.h"
#include <kis_cross_device_color_picker.h>

class QPointF;
class KoAbstractGradient;
class KisBrushBasedPaintOpSettings;
class KisPainter;

class KisColorSmudgeOp: public KisBrushBasedPaintOp
{
public:
    KisColorSmudgeOp(const KisBrushBasedPaintOpSettings* settings, KisPainter* painter, KisImageWSP image);
    virtual ~KisColorSmudgeOp();

    KisSpacingInformation paintAt(const KisPaintInformation& info);

private:
    void updateMask(const KisPaintInformation& info, double scale, double rotation);
    inline void getTopLeftAligned(const QPointF &pos, const QPointF &hotSpot, qint32 *x, qint32 *y);

private:
    bool                      m_firstRun;
    KisImageWSP               m_image;
    KisPaintDeviceSP          m_tempDev;
    KisPainter*               m_backgroundPainter;
    KisPainter*               m_smudgePainter;
    KisPainter*               m_colorRatePainter;
    const KoAbstractGradient* m_gradient;
    KisPressureSizeOption     m_sizeOption;
    KisPressureOpacityOption  m_opacityOption;
    KisPressureSpacingOption  m_spacingOption;
    KisSmudgeOption           m_smudgeRateOption;
    KisRateOption             m_colorRateOption;
    KisOverlayModeOption      m_overlayModeOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureScatterOption  m_scatterOption;
    KisPressureGradientOption m_gradientOption;
    QRect                     m_maskBounds;
    KisFixedPaintDeviceSP     m_maskDab;
    KisCrossDeviceColorPickerInt m_colorPicker;
    QPointF                   m_lastPaintPos;
};

#endif // _KIS_COLORSMUDGEOP_H_
