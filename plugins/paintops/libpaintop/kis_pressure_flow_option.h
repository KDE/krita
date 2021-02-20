/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Timothée Giet <animtim@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_FLOW_OPTION_H
#define KIS_PRESSURE_FLOW_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>

/**
 * The pressure flow option defines a curve that is used to
 * calculate the effect of pressure on the flow of the dab
 */
class PAINTOP_EXPORT KisPressureFlowOption : public KisCurveOption
{
public:
    KisPressureFlowOption();
    double apply(const KisPaintInformation & info) const;


};

#endif
 
