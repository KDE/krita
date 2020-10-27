/*
 * Copyright (c) Peter Schatz <voronwe13@gmail.com>, (C) 2020
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

#include "kis_pressure_lightness_strength_option_widget.h"

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

#include <klocalizedstring.h>

#include "kis_pressure_lightness_strength_option.h"

KisPressureLightnessStrengthOptionWidget::KisPressureLightnessStrengthOptionWidget()
    : KisCurveOptionWidget(new KisPressureLightnessStrengthOption(), i18n("0%"), i18n("100%"))
{
    setObjectName("KisPressureLightnessStrengthOptionWidget");

    m_enabledLabel = new QLabel(i18n("Disabled: brush must be in Lightness mode for this option to apply"));
    m_enabledLabel->setEnabled(true);
    m_enabledLabel->setAlignment(Qt::AlignHCenter);

    QWidget* page = new QWidget;
    QVBoxLayout* pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addWidget(m_enabledLabel);
    pageLayout->addWidget(curveWidget());

    setConfigurationPage(page);
}

void KisPressureLightnessStrengthOptionWidget::setEnabled(bool enabled)
{
    KisCurveOptionWidget::setEnabled(enabled);
    m_enabledLabel->setVisible(!enabled);
}



