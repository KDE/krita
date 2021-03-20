/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_SOFTNESS_OPTION_H
#define KIS_PRESSURE_SOFTNESS_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>

/**
 * This option is responsible to deliver values suitable for softness
 * They are in range 0.0..1.0
 */
class PAINTOP_EXPORT KisPressureSoftnessOption : public KisCurveOption
{
public:
    KisPressureSoftnessOption();
    double apply(const KisPaintInformation & info) const;


};

#endif
