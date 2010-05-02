/* This file is part of the KDE project
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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
#include "kis_pressure_hsv_option.h"
#include <KoColorTransformation.h>

KisPressureHSVOption* KisPressureHSVOption::createHueOption()
{
    return new KisPressureHSVOption(i18n("Hue"), "h", -1, 1 );
}

KisPressureHSVOption* KisPressureHSVOption::createSaturationOption()
{
    return new KisPressureHSVOption(i18n("Saturation"), "s", -1, 1 );
}

KisPressureHSVOption* KisPressureHSVOption::createValueOption()
{
    return new KisPressureHSVOption(i18n("Value"), "v", -1, 1 );
}

struct KisPressureHSVOption::Private
{
    QString parameterName;
    int paramId;
    double min, max;
};

KisPressureHSVOption::KisPressureHSVOption(const QString& name, const QString& parameterName, double min, double max)
        : KisCurveOption(name, parameterName, KisPaintOpOption::colorCategory(), false), d(new Private)
{
    d->parameterName = parameterName;
    d->paramId = -1;
    d->min = min;
    d->max = max;
}


void KisPressureHSVOption::apply(KoColorTransformation* transfo, const KisPaintInformation& info) const
{
    if (!isChecked()) {
        return;
    }
    
    if(d->paramId == -1)
    {
        d->paramId = transfo->parameterId(d->parameterName);
    }

    double v = computeValue(info) * (d->max - d->min) + d->min;
    
    transfo->setParameter(d->paramId, v);
}
