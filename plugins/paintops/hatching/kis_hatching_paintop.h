/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008, 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara Toloza <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HATCHING_PAINTOP_H_
#define KIS_HATCHING_PAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_brush_based_paintop.h>
#include <kis_types.h>

#include "hatching_brush.h"
#include "kis_hatching_paintop_settings.h"

#include "KisHatchingStandardOptions.h"

#include "KisHatchingOptionsData.h"
#include "KisHatchingPreferencesData.h"

#include <KisStandardOptions.h>
#include "KisOpacityOption.h"


class KisPainter;

class KisHatchingPaintOp : public KisBrushBasedPaintOp
{

public:

    KisHatchingPaintOp(const KisPaintOpSettingsSP settings, KisPainter * painter, KisNodeSP node, KisImageSP image);
    ~KisHatchingPaintOp() override;

    /**
     *  Returns a number between -90 and 90, and corresponds to the
     *  angle that results from adding angle 'spin' to 'm_hatchingOptions.angle',
     *  corrected to coincide with the way the GUI operates.
     */
    double spinAngle(double spin);

protected:
    /**
     *  Paint a hatched dab around the mouse cursor according to
     *  sensor settings and user preferences.
     */
    KisSpacingInformation paintAt(const KisPaintInformation& info) override;

    KisSpacingInformation updateSpacingImpl(const KisPaintInformation &info) const override;

private:
    KisHatchingPaintOpSettingsSP m_settings;
    KisHatchingOptionsData m_hatchingOptions;
    KisHatchingPreferencesData m_hatchingPreferences;
    HatchingBrush *m_hatchingBrush;

    /**
     *  PaintDevice that will be filled with a single pass of
     *  hatching by HatchingBrush::hatch
     */
    KisPaintDeviceSP m_hatchedDab;

    /**
     *  Curve to control the hatching angle
     *  according to user preferences set in the GUI
     */
    KisAngleOption m_angleOption;

    /**
     *  Curve to control the intensity of crosshatching
     *  according to user preferences set in the GUI
     */
    KisCrosshatchingOption m_crosshatchingOption;

    /**
     *  Curve to control the dynamics of separation with
     *  device input
     */
    KisSeparationOption m_separationOption;

    /**
     *  Curve to control the thickness of the hatching lines
     *  with device input
     */
    KisThicknessOption m_thicknessOption;

    /**
     *  Curve to control the opacity of the entire dab
     *  with device input
     */
    KisOpacityOption m_opacityOption;

    /**
     *  Curve to control the size of the entire dab
     *  with device input
     */
    KisSizeOption m_sizeOption;
};

#endif // KIS_HATCHING_PAINTOP_H_
