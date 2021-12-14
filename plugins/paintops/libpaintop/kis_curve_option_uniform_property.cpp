/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_curve_option_uniform_property.h"

#include "kis_curve_option.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_preset.h"
#include "KisPaintOpPresetUpdateProxy.h"

KisCurveOptionUniformProperty::KisCurveOptionUniformProperty(const QString &name,
                                                             KisCurveOption *option,
                                                             KisPaintOpSettingsRestrictedSP settings,
                                                             QObject *parent)
    : KisDoubleSliderBasedPaintOpProperty(Double, KoID(name, option->id().name()), settings, parent)
    , m_option(option)
{
    setRange(option->minValue(), option->maxValue());
    setSingleStep(0.01);
    requestReadValue();
}

KisCurveOptionUniformProperty::KisCurveOptionUniformProperty(KisCurveOption *option, KisPaintOpSettingsRestrictedSP settings, QObject *parent)
    : KisDoubleSliderBasedPaintOpProperty(Double, option->id(), settings, parent)
    , m_option(option)
{
    setRange(option->minValue(), option->maxValue());
    setSingleStep(0.01);
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
