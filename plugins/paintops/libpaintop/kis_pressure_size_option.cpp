/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#include "kis_pressure_size_option.h"
#include "kis_pressure_opacity_option.h"
#include <klocalizedstring.h>
#include <kis_painter.h>
#include <KoColor.h>
#include "kis_dynamic_sensor.h"
#include <brushengine/kis_paintop_lod_limitations.h>


KisPressureSizeOption::KisPressureSizeOption()
    : KisCurveOption("Size", KisPaintOpOption::GENERAL, true)
{

}


double KisPressureSizeOption::apply(const KisPaintInformation & info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}

void KisPressureSizeOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    // HINT: FUZZY_PER_STROKE doesn't affect instant preview
    if (sensor(FUZZY_PER_DAB, true)) {
        l->limitations << KoID("size-fade", i18nc("PaintOp instant preview limitation", "Size -> Fuzzy (sensor)"));
    }

    if (sensor(FADE, true)) {
        l->blockers << KoID("size-fuzzy", i18nc("PaintOp instant preview limitation", "Size -> Fade (sensor)"));
    }
}
