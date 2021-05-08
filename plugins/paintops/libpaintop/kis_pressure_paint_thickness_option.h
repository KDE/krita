/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2021 Peter Schatz <voronwe13@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_PAINT_THICKNESS_OPTION_H
#define KIS_PRESSURE_PAINT_THICKNESS_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>


/**
 * The paint thickness option defines a curve that is used to calculate
 * the effect of curve widget value on the strength of the lightness overlay
 * used to simulate paint thickness
 */
class PAINTOP_EXPORT KisPressurePaintThicknessOption : public KisCurveOption
{
public:
    KisPressurePaintThicknessOption();

    enum ThicknessMode {
        SMUDGE,
        OVERWRITE,
        OVERLAY
    };

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void setThicknessMode(ThicknessMode mode);
    ThicknessMode getThicknessMode();

    double apply(const KisPaintInformation& info) const;

private:
    ThicknessMode m_mode;

};

#endif
