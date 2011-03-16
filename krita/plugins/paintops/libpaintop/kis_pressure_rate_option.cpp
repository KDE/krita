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

#include "kis_pressure_rate_option.h"


#include <klocale.h>

#include <kis_paint_device.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>
#include <KoColorSpace.h>

KisPressureRateOption::KisPressureRateOption()
        : KisCurveOption(i18n("Rate"), "Rate", KisPaintOpOption::brushCategory(), true)
{
}

void KisPressureRateOption::setRate(int rate)
{
    m_rate = rate;
}

int KisPressureRateOption::rate() const
{
    return m_rate;
}

void KisPressureRateOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty("RateValue", m_rate);
    setting->setProperty("RateVersion", "2");
}

void KisPressureRateOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
    if (setting->getString("RateVersion", "1") == "1") {
        m_rate = setting->getInt("RatePressure");
        setChecked(true);
    } else {
        m_rate = setting->getInt("RateValue");
    }
}

quint8 KisPressureRateOption::apply(quint8 opacity, const KisPaintInformation& info) const
{
    opacity = (m_rate * 255) / 100;

    if (isChecked()) {
        opacity = qBound((qint32)OPACITY_TRANSPARENT_U8,
                         (qint32)(double(opacity) * computeValue(info) / PRESSURE_DEFAULT),
                         (qint32)OPACITY_OPAQUE_U8);
    }

    return opacity;
}

