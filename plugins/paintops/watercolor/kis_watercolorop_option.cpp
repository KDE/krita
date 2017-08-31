/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
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

#include "kis_watercolorop_option.h"

#include <klocalizedstring.h>

#include <QWidget>
#include <QComboBox>

#include "ui_wdgwatercoloroptions.h"

class KisWatercolorOpOptionsWidget: public QWidget, public Ui::WdgWatercolorOptions
{
public:
    KisWatercolorOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        gravityX->setRange(-10.0, 10.0);
        gravityX->setValue(0);
        gravityX->setSingleStep(1.0);

        gravityY->setRange(-10.0, 10.0);
        gravityY->setValue(0);
        gravityY->setSingleStep(1.0);

        radius->setRange(0.0, 1000.0);
        radius->setSuffix(i18n(" px"));
        radius->setValue(100.0);
        radius->setSingleStep(1.0);
    }
};

KisWatercolorOpOption::KisWatercolorOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisWatercolorOpOption");

    m_checkable = false;
    m_options = new KisWatercolorOpOptionsWidget();

    connect(m_options->gravityX, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->gravityY, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->brushType, SIGNAL(currentIndexChanged(QString)), SLOT(emitSettingChanged()));
    connect(m_options->radius, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    setConfigurationPage(m_options);
}

KisWatercolorOpOption::~KisWatercolorOpOption()
{
    delete m_options;
}

void KisWatercolorOpOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    setting->setProperty(WATERCOLOR_GRAVITY_X, m_options->gravityX->value());
    setting->setProperty(WATERCOLOR_GRAVITY_Y, m_options->gravityY->value());
    setting->setProperty(WATERCOLOR_TYPE, m_options->brushType->currentIndex());
    setting->setProperty(WATERCOLOR_RADIUS, m_options->radius->value());
}

void KisWatercolorOpOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    m_options->gravityX->setValue(setting->getDouble(WATERCOLOR_GRAVITY_X));
    m_options->gravityY->setValue(setting->getDouble(WATERCOLOR_GRAVITY_Y));
    m_options->brushType->setCurrentIndex(setting->getInt(WATERCOLOR_TYPE));
    m_options->radius->setValue(setting->getDouble(WATERCOLOR_RADIUS));
}
