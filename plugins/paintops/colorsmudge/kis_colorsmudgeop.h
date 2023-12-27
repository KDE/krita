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

#include "KisOverlayPaintDeviceWrapper.h"
#include <KisOpacityOption.h>
#include <KisSpacingOption.h>
#include <KisScatterOption.h>
#include <KisRotationOption.h>
#include <KisHSVOption.h>
#include <KisAirbrushOptionData.h>
#include <KisPaintThicknessOption.h>
#include <KisGradientOption.h>
#include <KisSmudgeLengthOption.h>
#include <KisColorRateOption.h>
#include <KisSmudgeRadiusOption.h>
#include <KisSmudgeOverlayModeOptionData.h>

class QPointF;

class KisBrushBasedPaintOpSettings;
class KisPainter;
class KoColorSpace;
class KisInterstrokeDataFactory;

class KisColorSmudgeStrategy;

class KisColorSmudgeOp: public KisBrushBasedPaintOp
{
public:
    KisColorSmudgeOp(const KisPaintOpSettingsSP settings, KisPainter* painter, KisNodeSP node, KisImageSP image);
    ~KisColorSmudgeOp() override;

    static KisInterstrokeDataFactory* createInterstrokeDataFactory(const KisPaintOpSettingsSP settings, KisResourcesInterfaceSP resourcesInterface);

protected:
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;
    KisTimingInformation updateTimingImpl(const KisPaintInformation &info) const override;

private:
    bool                      m_firstRun;

    KoColor                   m_paintColor;
    KoAbstractGradientSP      m_gradient;
    KisSizeOption m_sizeOption;
    KisRatioOption m_ratioOption;
    KisOpacityOption m_opacityOption;
    KisSpacingOption m_spacingOption;
    KisRateOption m_rateOption;
    KisRotationOption m_rotationOption;
    KisScatterOption m_scatterOption;
    KisPaintThicknessOption m_paintThicknessOption;
    KisGradientOption m_gradientOption;
    KisSmudgeLengthOption m_smudgeRateOption;
    KisColorRateOption2 m_colorRateOption;
    KisSmudgeRadiusOption2 m_smudgeRadiusOption;

    QList<KisHSVOption*> m_hsvOptions;
    KisAirbrushOptionData m_airbrushData;
    KisSmudgeOverlayModeOptionData m_overlayModeData;

    QRect                     m_dstDabRect;
    QPointF                   m_lastPaintPos;

    KoColorTransformation *m_hsvTransform {0};
    QScopedPointer<KisColorSmudgeStrategy> m_strategy;
};

#endif // _KIS_COLORSMUDGEOP_H_
