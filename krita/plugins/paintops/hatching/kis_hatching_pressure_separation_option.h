/*
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_HATCHING_PRESSURE_SEPARATION_OPTION_H
#define KIS_HATCHING_PRESSURE_SEPARATION_OPTION_H

#include "kis_curve_option.h"
#include <kis_paint_information.h>
#include <krita_export.h>

/**
 * The pressure separation option defines a curve that is used to
 * calculate the effect of pressure (or other parameters) on
 * separation in the hatching brush
 */
class KisHatchingPressureSeparationOption : public KisCurveOption
{
public:
    KisHatchingPressureSeparationOption();
    double apply(const KisPaintInformation & info) const;
};

#endif

