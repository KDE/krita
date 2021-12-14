/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_gradient_option.h"

#include <kis_curve_label.h>
#include <klocalizedstring.h>
#include <KoColor.h>
#include <resources/KoAbstractGradient.h>

KisPressureGradientOption::KisPressureGradientOption()
    : KisCurveOption(KoID("Gradient", i18n("Gradient")), KisPaintOpOption::GENERAL, false)
{
}

void KisPressureGradientOption::apply(KoColor& color, const KoAbstractGradientSP gradient, const KisPaintInformation& info) const
{
    if (isChecked() && gradient) {
        gradient->colorAt(color, computeSizeLikeValue(info));
    }
}
