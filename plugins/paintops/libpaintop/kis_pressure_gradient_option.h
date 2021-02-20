/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_PRESSURE_GRADIENT_OPTION_H
#define KIS_PRESSURE_GRADIENT_OPTION_H

#include <KoAbstractGradient.h>

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>

class KoColor;


class PAINTOP_EXPORT KisPressureGradientOption: public KisCurveOption
{
public:
    KisPressureGradientOption();
    void apply(KoColor& color, const KoAbstractGradientSP gradient, const KisPaintInformation& info) const;
};

#endif // KIS_PRESSURE_GRADIENT_OPTION_H
