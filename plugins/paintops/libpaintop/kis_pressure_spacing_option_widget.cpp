/*
 * SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>


#include <klocalizedstring.h>

#include <kis_slider_spin_box.h>

#include "kis_curve_option_widget.h"
#include "kis_pressure_spacing_option.h"
#include "kis_pressure_spacing_option_widget.h"

KisPressureSpacingOptionWidget::KisPressureSpacingOptionWidget():
    KisCurveOptionWidget(new KisPressureSpacingOption(), i18n("0%"), i18n("100%"))
{
    m_isotropicSpacing = new QCheckBox(i18n("Isotropic Spacing"));
    m_useSpacingUpdates = new QCheckBox(i18n("Update Between Dabs"));

    QHBoxLayout *hl = new QHBoxLayout;
    hl->addWidget(m_isotropicSpacing);
    hl->addWidget(m_useSpacingUpdates);

    QWidget *page = new QWidget;
    QVBoxLayout *pageLayout = new QVBoxLayout(page);
    pageLayout->setMargin(0);
    pageLayout->addLayout(hl);
    pageLayout->addWidget(KisCurveOptionWidget::curveWidget());

    KisCurveOptionWidget::setConfigurationPage(page);

    connect(m_isotropicSpacing, SIGNAL(stateChanged(int)),
            this, SLOT(setIsotropicSpacing(int)));
    connect(m_useSpacingUpdates, SIGNAL(stateChanged(int)),
            this, SLOT(setUseSpacingUpdates(int)));

    setIsotropicSpacing(false);
}

void KisPressureSpacingOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    // First invoke superclass behavior.
    KisCurveOptionWidget::readOptionSetting(setting);

    KisPressureSpacingOption *option = dynamic_cast<KisPressureSpacingOption*>(curveOption());
    m_isotropicSpacing->setChecked(option->isotropicSpacing());
    m_useSpacingUpdates->setChecked(option->usingSpacingUpdates());
}

void KisPressureSpacingOptionWidget::setIsotropicSpacing(int isotropic)
{
    dynamic_cast<KisPressureSpacingOption*>(KisCurveOptionWidget::curveOption())->setIsotropicSpacing(isotropic);
    emitSettingChanged();
}

void KisPressureSpacingOptionWidget::setUseSpacingUpdates(int useSpacingUpdates)
{
    dynamic_cast<KisPressureSpacingOption*>(KisCurveOptionWidget::curveOption())->setUsingSpacingUpdates(useSpacingUpdates);
    emitSettingChanged();
}
