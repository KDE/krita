/*
 *  Copyright (c) 2008-2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_sprayop_option.h"
#include <klocale.h>

#include "ui_wdgsprayoptions.h"

class KisSprayOpOptionsWidget: public QWidget, public Ui::WdgSprayOptions
{
public:
    KisSprayOpOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};

KisSprayOpOption::KisSprayOpOption()
        : KisPaintOpOption(i18n("Brush size"), false)
{
    m_checkable = false;
    m_options = new KisSprayOpOptionsWidget();
    connect(m_options->diameterSpinBox, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->coverageSpin, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->amountSpin, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->spacingSpin, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->scaleSpin, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->particlesSpinBox, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->densityChBox, SIGNAL(stateChanged(int)), SIGNAL(sigSettingChanged()));

    setConfigurationPage(m_options);
}

KisSprayOpOption::~KisSprayOpOption()
{
    // delete m_options;
}

int KisSprayOpOption::diameter() const
{
    return m_options->diameterSpinBox->value();
}

bool KisSprayOpOption::jitterSize() const
{
    return m_options->jitterSizeBox->isChecked();
}

bool KisSprayOpOption::jitterMovement() const
{
    return m_options->jitterMoveBox->isChecked();
}

qreal KisSprayOpOption::coverage() const
{
    return m_options->coverageSpin->value();
}

qreal KisSprayOpOption::amount() const
{
    return m_options->amountSpin->value();
}

qreal KisSprayOpOption::spacing() const
{
    return m_options->spacingSpin->value();
}

qreal KisSprayOpOption::scale() const
{
    return m_options->scaleSpin->value();
}


int KisSprayOpOption::particleCount() const
{
    return m_options->particlesSpinBox->value();
}

bool KisSprayOpOption::useDensity() const
{
    return m_options->densityChBox->isChecked();
}


void KisSprayOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty("Spray/diameter", diameter());
    setting->setProperty("Spray/coverage", coverage());
    setting->setProperty("Spray/amount", amount());
    setting->setProperty("Spray/spacing", spacing());
    setting->setProperty("Spray/jitterSize", jitterSize());
    setting->setProperty("Spray/jitterMovement", jitterMovement());
}

void KisSprayOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->diameterSpinBox->setValue(setting->getInt("Spray/diameter"));
    m_options->coverageSpin->setValue(setting->getDouble("Spray/coverage"));
    m_options->amountSpin->setValue(setting->getDouble("Spray/amount"));
    m_options->spacingSpin->setValue(setting->getDouble("Spray/spacing"));
    m_options->jitterSizeBox->setChecked(setting->getBool("Spray/jitterSize"));
    m_options->jitterMoveBox->setChecked(setting->getBool("Spray/jitterMovement"));
}


void KisSprayOpOption::setDiamter(int diameter) const
{
    m_options->diameterSpinBox->setValue(diameter);
}

