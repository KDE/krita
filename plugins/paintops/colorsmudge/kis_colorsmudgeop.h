/*
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <kis_pressure_ratio_option.h>
#include <kis_pressure_spacing_option.h>
#include <kis_pressure_rate_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_pressure_scatter_option.h>
#include <kis_pressure_gradient_option.h>
#include <kis_pressure_hsv_option.h>
#include <kis_pressure_lightness_strength_option.h>
#include <kis_airbrush_option_widget.h>

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
    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    // Sets the m_maskDab _and m_maskDabRect
    void updateMask(const KisPaintInformation& info, const KisDabShape &shape, const QPointF &cursorPoint);
    KoColor getDullingFillColor(const KisPaintInformation& info, KisPrecisePaintDeviceWrapper& activeWrapper, QPoint canvasLocalSamplePoint);
    void mixSmudgePaintAt(const KisPaintInformation& info, KisPrecisePaintDeviceWrapper& activeWrapper, QRect srcDabRect, QPoint canvasLocalSamplePoint, bool useDullingMode);

    inline void getTopLeftAligned(const QPointF &pos, const QPointF &hotSpot, qint32 *x, qint32 *y);

private:
    bool                      m_firstRun;
    bool                      m_useNewEngine;

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
    KisPressureRatioOption    m_ratioOption;
    KisPressureSpacingOption  m_spacingOption;
    KisPressureRateOption     m_rateOption;
    KisPressureLightnessStrengthOption   m_lightnessStrengthOption;
    KisSmudgeOption           m_smudgeRateOption;
    KisRateOption             m_colorRateOption;
    KisSmudgeRadiusOption     m_smudgeRadiusOption;
    KisOverlayModeOption      m_overlayModeOption;
    KisPressureRotationOption m_rotationOption;
    KisPressureScatterOption  m_scatterOption;
    KisPressureGradientOption m_gradientOption;
    QList<KisPressureHSVOption*> m_hsvOptions;
    KisAirbrushOptionProperties m_airbrushOption;
    QRect                     m_dstDabRect;
    KisFixedPaintDeviceSP     m_maskDab;
    KisFixedPaintDeviceSP     m_canvasDab;
    KisFixedPaintDeviceSP     m_canvasSrc;
    KisFixedPaintDeviceSP     m_mixedDab;
    KisFixedPaintDeviceSP     m_origDab;
    KisFixedPaintDeviceSP     m_origDabCopy;
    QPointF                   m_lastPaintPos;

    KoColorTransformation *m_hsvTransform {0};
    const KoCompositeOp *m_preciseColorRateCompositeOp {0};
};

#endif // _KIS_COLORSMUDGEOP_H_
