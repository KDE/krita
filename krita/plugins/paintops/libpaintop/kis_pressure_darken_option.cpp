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
#include "kis_pressure_darken_option.h"
#include <klocale.h>
#include <kis_painter.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include "kis_color_source.h"

KisPressureDarkenOption::KisPressureDarkenOption()
        : KisCurveOption(i18n("Darken"), "Darken", false)
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
    quint32 darkenAmount = (qint32)(255  - 75 * computeValue(info));
    KoColorTransformation* darkenTransformation  = darkened.colorSpace()->createDarkenAdjustment(darkenAmount, false, 0.0);
    darkenTransformation ->transform(painter->paintColor().data(), darkened.data(), 1);
    painter->setPaintColor(darkened);
    delete darkenTransformation;

    return origColor;
}

void KisPressureDarkenOption::apply(KisColorSource* colorSource, const KisPaintInformation& info) const
{
    if (!isChecked()) return;
    
    quint32 darkenAmount = (qint32)(255  - 75 * computeValue(info));
    KoColorTransformation* darkenTransformation  = colorSource->colorSpace()->createDarkenAdjustment(darkenAmount, false, 0.0);
    
    colorSource->applyColorTransformation(darkenTransformation);

    delete darkenTransformation;
}
