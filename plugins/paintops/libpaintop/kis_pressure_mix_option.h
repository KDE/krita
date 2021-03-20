/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_MIX_OPTION_H
#define KIS_PRESSURE_MIX_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>

/**
 * The pressure opacity option defines a curve that is used to
 * calculate the effect of pressure on the mix of the color
 */
class PAINTOP_EXPORT KisPressureMixOption : public KisCurveOption
{
public:
    KisPressureMixOption();
    double apply(const KisPaintInformation & info) const;


};

#endif
