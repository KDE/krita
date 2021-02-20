/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
