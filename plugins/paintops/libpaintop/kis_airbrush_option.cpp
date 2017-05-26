/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_airbrush_option.h"
#include "kis_paintop_settings.h"
#include <klocalizedstring.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgairbrush.h"

const qreal MINIMUM_RATE = 0.0;
const qreal MAXIMUM_RATE = 1000.0;
const int RATE_NUM_DECIMALS = 2;
const qreal RATE_EXPONENT_RATIO = 3.0;
const qreal RATE_SINGLE_STEP = 1.0;
const qreal DEFAULT_RATE = 20.0;

class KisAirbrushWidget: public QWidget, public Ui::WdgAirbrush
{
public:
    KisAirbrushWidget(QWidget *parent = 0, bool canIgnoreSpacing = true)
        : QWidget(parent) {
        setupUi(this);

        sliderRate->setRange(MINIMUM_RATE, MAXIMUM_RATE, RATE_NUM_DECIMALS);
        sliderRate->setExponentRatio(RATE_EXPONENT_RATIO);
        sliderRate->setSingleStep(RATE_SINGLE_STEP);
        sliderRate->setValue(DEFAULT_RATE);

        checkBoxIgnoreSpacing->setVisible(canIgnoreSpacing);
        checkBoxIgnoreSpacing->setEnabled(canIgnoreSpacing);
    }
};


KisAirbrushOption::KisAirbrushOption(bool enabled, bool canIgnoreSpacing)
    : KisPaintOpOption(KisPaintOpOption::COLOR, enabled)
{
    setObjectName("KisAirBrushOption");
    m_checkable = true;
    m_optionWidget = new KisAirbrushWidget(nullptr, canIgnoreSpacing);
    connect(m_optionWidget->sliderRate, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_optionWidget->checkBoxIgnoreSpacing, SIGNAL(toggled(bool)),
            SLOT(emitSettingChanged()));
    setConfigurationPage(m_optionWidget);
}


KisAirbrushOption::~KisAirbrushOption()
{
}

void KisAirbrushOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    setting->setProperty(AIRBRUSH_ENABLED, isChecked());
    setting->setProperty(AIRBRUSH_RATE, m_optionWidget->sliderRate->value());
    setting->setProperty(AIRBRUSH_IGNORE_SPACING,
                         m_optionWidget->checkBoxIgnoreSpacing->isChecked());
}

void KisAirbrushOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    setChecked(setting->getBool(AIRBRUSH_ENABLED));
    m_optionWidget->sliderRate->setValue(setting->getDouble(AIRBRUSH_RATE, DEFAULT_RATE));
    m_optionWidget->checkBoxIgnoreSpacing->setChecked(setting->getBool(AIRBRUSH_IGNORE_SPACING,
                                                                       false));
}

