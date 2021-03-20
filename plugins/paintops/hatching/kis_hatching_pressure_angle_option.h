/*  By Idiomdrottning <sandra.snan@idiomdrottning.org> 2018, after a file that
 *  was SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KIS_HATCHING_PRESSURE_ANGLE_OPTION_H
#define KIS_HATCHING_PRESSURE_ANGLE_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>

/**
 * The pressure angle option defines a curve that is used to
 * calculate the effect of pressure (or other parameters) on
 * angle in the hatching brush
 */
class KisHatchingPressureAngleOption : public KisCurveOption
{
public:
    KisHatchingPressureAngleOption();
    double apply(const KisPaintInformation & info) const;
};

#endif
