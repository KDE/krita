/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Emanuele Tamponi <emanuele@valinor.it>
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
#include "kis_bidirectional_mixing_option_widget.h"
#include <klocale.h>

#include <QLabel>
#include <kis_properties_configuration.h>


KisBidirectionalMixingOptionWidget::KisBidirectionalMixingOptionWidget()
        : KisPaintOpOption(i18n("Mixing"), KisPaintOpOption::colorCategory(), false)
{
    m_checkable = true;
    m_optionWidget = new QLabel(i18n("The mixing option mixes the paint on the brush with that on the canvas."));
    m_optionWidget->hide();
    setConfigurationPage(m_optionWidget);
}

KisBidirectionalMixingOptionWidget::~KisBidirectionalMixingOptionWidget()
{
}

void KisBidirectionalMixingOptionWidget::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(BIDIRECTIONAL_MIXING_ENABLED, isChecked());
}

void KisBidirectionalMixingOptionWidget::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    setChecked(setting->getBool(BIDIRECTIONAL_MIXING_ENABLED, false));
}

