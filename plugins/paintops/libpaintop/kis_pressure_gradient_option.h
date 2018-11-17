/* This file is part of the KDE project
 * Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
