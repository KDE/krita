/*
 * SPDX-FileCopyrightText: 2020 Peter Schatz <voronwe13@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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



