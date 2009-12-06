/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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
#include "kis_pressure_mix_option.h"
#include <klocale.h>
#include <kis_painter.h>
#include <KoColor.h>
#include <KoColorSpace.h>

KisPressureMixOption::KisPressureMixOption()
        : KisCurveOption(i18n("Mix"), "Mix", false)
{
}


double KisPressureMixOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeValue(info);
}
