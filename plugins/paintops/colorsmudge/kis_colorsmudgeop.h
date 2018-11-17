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

#include "KoColorTransformation.h"
#include <KoAbstractGradient.h>

#include <kis_brush_based_paintop.h>
#include <kis_types.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_spacing_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option.h>
#include <kis_pressure_gradient_option.h>
#include <kis_pressure_hsv_option.h>

#include "kis_overlay_mode_option.h"
#include "kis_rate_option.h"
#include "kis_smudge_option.h"
#include "kis_smudge_radius_option.h"
#include "KisPrecisePaintDeviceWrapper.h"

class QPointF;

class KisBrushBasedPaintOpSettings;
class KisPainter;
class KoColorSpace;

class KisColorSmudgeOp: public KisBrushBasedPaintOp
{
public:
    KisColorSmudgeOp(const KisPaintOpSettingsSP settings, KisPainter* painter, KisNodeSP node, KisImageSP image);
    ~KisColorSmudgeOp() override;

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    // Sets the m_maskDab _and m_maskDabRect
    void updateMask(const KisPaintInformation& info, double scale, double rotation, const QPointF &cursorPoint);

    inline void getTopLeftAligned(const QPointF &pos, const QPointF &hotSpot, qint32 *x, qint32 *y);

private:
    bool                      m_firstRun;
    KisImageWSP               m_image;
    KisPrecisePaintDeviceWrapper m_precisePainterWrapper;
    KoColor                   m_paintColor;
    KisPaintDeviceSP          m_tempDev;
    QScopedPointer<KisPrecisePaintDeviceWrapper> m_preciseImageDeviceWrapper;
    QScopedPointer<KisPainter> m_backgroundPainter;
    QScopedPointer<KisPainter> m_smudgePainter;
    QScopedPointer<KisPainter> m_colorRatePainter;
    QScopedPointer<KisPainter> m_finalPainter;
    KoAbstractGradientSP      m_gradient;
    KisPressureSizeOption     m_sizeOption;
    KisPressureOpacityOption  m_opacityOption;
    KisPressureSpacingOption  m_spacingOption;
    KisSmudgeOption           m_smudgeRateOption;
    KisRateOption             m_colorRateOption;
    KisSmudgeRadiusOption     m_smudgeRadiusOption;
    KisOverlayModeOption      m_overlayModeOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureScatterOption  m_scatterOption;
    KisPressureGradientOption m_gradientOption;
    QList<KisPressureHSVOption*> m_hsvOptions;
    QRect                     m_dstDabRect;
    KisFixedPaintDeviceSP     m_maskDab;
    QPointF                   m_lastPaintPos;

    KoColorTransformation *m_hsvTransform {0};
    const KoCompositeOp *m_preciseColorRateCompositeOp {0};
};

#endif // _KIS_COLORSMUDGEOP_H_
