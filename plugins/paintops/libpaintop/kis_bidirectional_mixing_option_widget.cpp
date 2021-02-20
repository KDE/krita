/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008 Emanuele Tamponi <emanuele@valinor.it>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_bidirectional_mixing_option_widget.h"
#include <klocalizedstring.h>

#include <QLabel>
#include <kis_properties_configuration.h>


KisBidirectionalMixingOptionWidget::KisBidirectionalMixingOptionWidget()
    : KisPaintOpOption(KisPaintOpOption::COLOR, false)
{
    m_checkable = true;
    m_optionWidget = new QLabel(i18n("The mixing option mixes the paint on the brush with that on the canvas."));
    m_optionWidget->hide();
    setConfigurationPage(m_optionWidget);
    setObjectName("KisBidirectionalMixingOptionWidget");
}

KisBidirectionalMixingOptionWidget::~KisBidirectionalMixingOptionWidget()
{
}

void KisBidirectionalMixingOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    setting->setProperty(BIDIRECTIONAL_MIXING_ENABLED, isChecked());
}

void KisBidirectionalMixingOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    setChecked(setting->getBool(BIDIRECTIONAL_MIXING_ENABLED, false));
}

