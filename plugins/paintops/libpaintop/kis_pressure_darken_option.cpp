/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_darken_option.h"
#include <klocalizedstring.h>
#include <kis_painter.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include "kis_color_source.h"
#include <kis_paint_device.h>

KisPressureDarkenOption::KisPressureDarkenOption()
    : KisCurveOption(KoID("Darken", i18n("Darken")), KisPaintOpOption::COLOR, false)
{
}


KoColor KisPressureDarkenOption::apply(KisPainter * painter, const KisPaintInformation& info) const
{
    if (!isChecked()) {
        return painter->paintColor();
    }

    KoColor darkened = painter->paintColor();
    KoColor origColor = darkened;

    // Darken docs aren't really clear about what exactly the amount param can have as value...
    quint32 darkenAmount = (qint32)(255  - 255 * computeSizeLikeValue(info));

    KoColorTransformation* darkenTransformation  = darkened.colorSpace()->createDarkenAdjustment(darkenAmount, false, 0.0);
    if (!darkenTransformation) return origColor;
    darkenTransformation ->transform(painter->paintColor().data(), darkened.data(), 1);
    painter->setPaintColor(darkened);
    delete darkenTransformation;

    return origColor;
}

void KisPressureDarkenOption::apply(KisColorSource* colorSource, const KisPaintInformation& info) const
{
    if (!isChecked()) return;

    // Darken docs aren't really clear about what exactly the amount param can have as value...
    quint32 darkenAmount = (qint32)(255  - 255 * computeSizeLikeValue(info));

    KoColorTransformation* darkenTransformation  = colorSource->colorSpace()->createDarkenAdjustment(darkenAmount, false, 0.0);
    if (!darkenTransformation) return;
    colorSource->applyColorTransformation(darkenTransformation);

    delete darkenTransformation;
}
