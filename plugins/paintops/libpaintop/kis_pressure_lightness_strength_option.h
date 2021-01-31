/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2020 Peter Schatz <voronwe13@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_LIGHTNESS_STRENGTH_OPTION_H
#define KIS_PRESSURE_LIGHTNESS_STRENGTH_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>


/**
 * The lightness strength option defines a curve that is used to
 * calculate the effect of pressure on the strength of the lightness overlay
 */
class PAINTOP_EXPORT KisPressureLightnessStrengthOption : public KisCurveOption
{
public:
    KisPressureLightnessStrengthOption();
    double apply(const KisPaintInformation & info) const;

};

#endif
