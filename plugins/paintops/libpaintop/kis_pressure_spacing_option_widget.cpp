/*
 * Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
    hl->setContentsMargins(9,9,9,0); // no bottom spacing
    hl->addWidget(m_isotropicSpacing);
    hl->addWidget(m_useSpacingUpdates);

    QVBoxLayout *vl = new QVBoxLayout;
    vl->setMargin(0);
    vl->addLayout(hl);
    vl->addWidget(KisCurveOptionWidget::curveWidget());

    QWidget *w = new QWidget;
    w->setLayout(vl);

    KisCurveOptionWidget::setConfigurationPage(w);

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
