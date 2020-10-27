/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_curve_option_uniform_property.h"

#include "kis_curve_option.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"


KisCurveOptionUniformProperty::KisCurveOptionUniformProperty(const QString &name,
                                                             KisCurveOption *option,
                                                             KisPaintOpSettingsRestrictedSP settings,
                                                             QObject *parent)
    : KisDoubleSliderBasedPaintOpProperty(Double,
                                          name,
                                          option->name(),
                                          settings,
                                          parent),
      m_option(option)
{
    setRange(option->minValue(), option->maxValue());
    setSingleStep(0.01);
    connect(settings->updateProxy(), SIGNAL(sigSettingsChanged()), this, SLOT(requestReadValue()));
    requestReadValue();
}

KisCurveOptionUniformProperty::~KisCurveOptionUniformProperty()
{
}

void KisCurveOptionUniformProperty::readValueImpl()
{
    m_option->readOptionSetting(settings().data());
    setValue(m_option->value());
}

void KisCurveOptionUniformProperty::writeValueImpl()
{
    m_option->readOptionSetting(settings().data());
    m_option->setValue(value().toReal());
    m_option->writeOptionSetting(settings().data());
}

bool KisCurveOptionUniformProperty::isVisible() const
{
    return !m_option->isCheckable() || m_option->isChecked();
}
