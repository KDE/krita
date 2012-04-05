/*
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_curves_opacity_option.h"
#include <klocale.h>

KisCurvesOpacityOption::KisCurvesOpacityOption()
        : KisCurveOption(i18n("Curves opacity"), "Curves opacity", KisPaintOpOption::brushCategory(), false)
{
    setMinimumLabel(i18n("0%"));
    setMaximumLabel(i18n("100%"));
}


qreal KisCurvesOpacityOption::apply(const KisPaintInformation & info, qreal opacity) const
{
    if (!isChecked()) {
        return opacity;
    }
    return computeValue(info) * opacity;
}
