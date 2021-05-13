/* This file is part of the KDE project
 * Copyright (C)Peter Schatz <voronwe13@gmail.com>, (C) 2021
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "kis_pressure_paint_thickness_option.h"

KisPressurePaintThicknessOption::KisPressurePaintThicknessOption()
    : KisCurveOption("PaintThickness", KisPaintOpOption::GENERAL, false)
{
    m_mode = OVERLAY;
}


void KisPressurePaintThicknessOption::setThicknessMode(ThicknessMode mode) {
    m_mode = mode;

}

KisPressurePaintThicknessOption::ThicknessMode KisPressurePaintThicknessOption::getThicknessMode() {
    return m_mode;
}

void KisPressurePaintThicknessOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty(name() + "ThicknessMode", m_mode);
}

void KisPressurePaintThicknessOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_mode = (ThicknessMode)setting->getInt(name() + "ThicknessMode", OVERLAY);

    if (m_mode == RESERVED) {
        m_mode = OVERLAY;
    }
}

double KisPressurePaintThicknessOption::apply(const KisPaintInformation& info) const
{
    if (!isChecked()) return 1.0;
    return computeSizeLikeValue(info);
}
