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
#include "kis_pressure_gradient_option.h"

#include <kis_curve_label.h>
#include <klocale.h>
#include <KoColor.h>
#include <KoAbstractGradient.h>

KisPressureGradientOption::KisPressureGradientOption(): KisCurveOption(i18n("Gradient"), "Gradient", KisPaintOpOption::brushCategory(), false)
{
    setMinimumLabel(i18n("0%"));
    setMaximumLabel(i18n("100%"));
}

void KisPressureGradientOption::apply(KoColor& color, const KoAbstractGradient* gradient, const KisPaintInformation& info) const
{
    if(isChecked() && gradient)
        gradient->colorAt(color, computeValue(info));
}
