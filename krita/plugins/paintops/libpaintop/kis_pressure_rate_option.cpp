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

#include <QWidget>
#include <QCheckBox>
#include <QLabel>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klocale.h>

#include <kis_paint_device.h>
#include <widgets/kis_curve_widget.h>

#include <KoColor.h>
#include <KoColorSpace.h>

KisPressureRateOption::KisPressureRateOption()
        : KisCurveOption(i18n("Rate"), "Rate")
{
    QWidget* w = new QWidget;
    QLabel* rateLabel = new QLabel(i18n("Rate: "));
    m_rateSlider = new QSlider();
    m_rateSlider->setMinimum(0);
    m_rateSlider->setMaximum(100);
    m_rateSlider->setPageStep(1);
    m_rateSlider->setValue(90);
    m_rateSlider->setOrientation(Qt::Horizontal);
    QHBoxLayout* hl = new QHBoxLayout;
    hl->addWidget(rateLabel);
    hl->addWidget(m_rateSlider);

    QVBoxLayout* vl = new QVBoxLayout;
    vl->addLayout(hl);
    vl->addWidget(m_widget);

    w->setLayout(vl);
    setConfigurationPage(w);
}

int KisPressureRateOption::rate() const
{
    return (m_rateSlider->value() * 255) / 100;
}

void KisPressureRateOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    KisCurveOption::writeOptionSetting(setting);
    setting->setProperty("PressureRate", m_rateSlider->value());
}

void KisPressureRateOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KisCurveOption::readOptionSetting(setting);
    m_rateSlider->setValue(setting->getInt("PressureRate"));
}



quint8 KisPressureRateOption::apply(quint8 opacity, const KisPaintInformation& info) const
{
    opacity = rate();

    if (isChecked()) {
        opacity = qBound((qint32)OPACITY_TRANSPARENT,
                         (qint32)(double(opacity) * computeValue(info) / PRESSURE_DEFAULT),
                         (qint32)OPACITY_OPAQUE);
    }

    return opacity;

}

