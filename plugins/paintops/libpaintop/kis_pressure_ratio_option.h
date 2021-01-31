/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2016 Nishant Rodrigues <nishantjr@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>

/** Calculates effect of pressure on aspect ratio of brush tip */
class PAINTOP_EXPORT KisPressureRatioOption : public KisCurveOption
{
public:
    KisPressureRatioOption();
    double apply(const KisPaintInformation & info) const;
};
