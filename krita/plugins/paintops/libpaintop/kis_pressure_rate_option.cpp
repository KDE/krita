/* This file is part of the KDE project
 * Copyright (C) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_pressure_rate_option.h"


#include <klocale.h>

#include <kis_painter.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>
#include <KoColorSpace.h>

KisPressureRateOption::KisPressureRateOption()
        : KisCurveOption(i18n("Rate"), "Rate", KisPaintOpOption::brushCategory(), true)
{
}

void KisPressureRateOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty("RateValue", m_rate);
}

void KisPressureRateOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_rate = setting->getDouble("RateValue");
}

void KisPressureRateOption::apply(KisPainter* painter, const KisPaintInformation& info, qreal scaleMin, qreal scaleMax) const
{
    if(!isChecked()) {
        painter->setOpacity((quint8)(scaleMax * 255.0));
        return;
    }
    
    qreal  rate    = scaleMin + (scaleMax - scaleMin) * m_rate; // scale m_rate into the range scaleMin - scaleMax
    quint8 opacity = qBound(OPACITY_TRANSPARENT_U8, (quint8)(rate * computeValue(info) * 255.0), OPACITY_OPAQUE_U8);

    painter->setOpacity(opacity);
}

