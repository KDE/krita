/* This file is part of the KDE project
 * Copyright (C) Scott Petrovic <scottpetrovic@gmail.com>, (C) 2016
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

#include "kis_performance_option.h"
#include <klocalizedstring.h>
#include <KoColor.h>
#include <kis_types.h>
#include <kis_config.h>
#include <QWidget>



KisPerformanceOption ::KisPerformanceOption (bool createConfigWidget):
        KisPaintOpOption(KisPaintOpOption::GENERAL, true),
        m_createConfigWidget(createConfigWidget)

{

    m_checkable = false;

    if (createConfigWidget) {

        QWidget* widget = new QWidget();
        ui.setupUi(widget);

        ui.precisionFixedSpinbox->setEnabled(false);

        // signals for when values changes with precision options
        ui.precisionFixedSpinbox->setRange(1, 5);
        ui.precisionFixedSpinbox->setSingleStep(1);
        ui.precisionFixedSpinbox->setValue(4);


        // default to auto setting appearance and assign tooltips to labels
        setAutoPrecisionEnabled(true);
        ui.startingSizeLabel->setToolTip(i18n("Use to set the size from which the Automatic Precision Setting should begin. \nThe Precision will remain 5 before this value."));
        ui.precisionDeltaSpinbox->setToolTip(i18n("Use to set the interval at which the Automatic Precision will change. \nThe Precision will decrease as brush size increases."));

        connect(ui.precisionFixedSpinbox, SIGNAL(valueChanged(int)), this, SLOT(precisionChanged(int)));
        connect(ui.precisionAutoCheckbox, SIGNAL(clicked(bool)), this, SLOT(setAutoPrecisionEnabled(bool)));
        connect(ui.precisionDeltaSpinbox, SIGNAL(valueChanged(double)), this, SLOT(setDeltaValue(double)));
        connect(ui.precisionStartingSizeSpinbox, SIGNAL(valueChanged(double)), this, SLOT(setSizeToStartFrom(double)));

        setConfigurationPage(widget);
        createConfigWidget = true;

    }

    setObjectName("KisPerformanceOption");

}

KisPerformanceOption::~KisPerformanceOption() {
}

void KisPerformanceOption::readOptionSetting(const KisPropertiesConfigurationSP config)
{
    m_precisionOption.readOptionSetting(config);

    ui.precisionFixedSpinbox->setValue(m_precisionOption.precisionLevel());
    ui.precisionAutoCheckbox->setChecked(m_precisionOption.autoPrecisionEnabled());
    ui.precisionDeltaSpinbox->setValue(m_precisionOption.deltaValue());
    ui.precisionStartingSizeSpinbox->setValue(m_precisionOption.sizeToStartFrom());

    setAutoPrecisionEnabled(m_precisionOption.autoPrecisionEnabled());
}

void KisPerformanceOption::writeOptionSetting(KisPropertiesConfigurationSP config) const
{
    m_precisionOption.writeOptionSetting(config);
}

void KisPerformanceOption::precisionChanged(int value)
{
    QString toolTip;

    switch (value) {
    case 1:
        toolTip =
            i18n("Precision Level 1 (fastest)\n"
                 "Subpixel precision: disabled\n"
                 "Brush size precision: 5%\n"
                 "\n"
                 "Optimal for very big brushes");
        break;
    case 2:
        toolTip =
            i18n("Precision Level 2\n"
                 "Subpixel precision: disabled\n"
                 "Brush size precision: 1%\n"
                 "\n"
                 "Optimal for big brushes");
        break;
    case 3:
        toolTip =
            i18n("Precision Level 3\n"
                 "Subpixel precision: disabled\n"
                 "Brush size precision: exact");
        break;
    case 4:
        toolTip =
            i18n("Precision Level 4 (optimal)\n"
                 "Subpixel precision: 50%\n"
                 "Brush size precision: exact\n"
                 "\n"
                 "Gives up to 50% better performance in comparison to Level 5");
        break;
    case 5:
        toolTip =
            i18n("Precision Level 5 (best quality)\n"
                 "Subpixel precision: exact\n"
                 "Brush size precision: exact\n"
                 "\n"
                 "The slowest performance. Best quality.");
        break;
    }
    ui.precisionFixedSpinbox->blockSignals(true);
    ui.precisionFixedSpinbox->setValue(value);
    ui.precisionFixedSpinbox->blockSignals(false);
    ui.precisionFixedSpinbox->setToolTip(toolTip);

    m_precisionOption.setPrecisionLevel(value);


    emitSettingChanged();
    emit sigPrecisionChanged();
}

void KisPerformanceOption::setAutoPrecisionEnabled(bool value)
{

    m_precisionOption.setAutoPrecisionEnabled(value);
    if(m_precisionOption.autoPrecisionEnabled())
    {
        ui.precisionFixedSpinbox->setEnabled(false);
        precisionChanged(m_precisionOption.precisionLevel());
        ui.startingSizeLabel->setVisible(true);
        ui.deltaLabel->setVisible(true);
        ui.precisionDeltaSpinbox->setVisible(true);
        ui.precisionStartingSizeSpinbox->setVisible(true);

        updatePrecisionCalculationsTable();
        showPrecisionCalculationsTable(true);
    }
    else
    {
        ui.precisionFixedSpinbox->setEnabled(true);

        // hide all the UI elements related to the auto configuration values
        ui.startingSizeLabel->setVisible(false);
        ui.deltaLabel->setVisible(false);
        ui.precisionDeltaSpinbox->setVisible(false);
        ui.precisionStartingSizeSpinbox->setVisible(false);

        showPrecisionCalculationsTable(false);
    }

    emitSettingChanged();
    emit sigPrecisionChanged();

}

void KisPerformanceOption::setDeltaValue(double value)
{

    m_precisionOption.setDeltaValue(value);
    updatePrecisionCalculationsTable();

    emitSettingChanged();
    emit sigPrecisionChanged();

}

void KisPerformanceOption::showPrecisionCalculationsTable(bool show)
{
    ui.precisionFiveCalculationsLabel->setVisible(show);
    ui.precisionFourCalculationsLabel->setVisible(show);
    ui.precisionThreeCalculationsLabel->setVisible(show);
    ui.precisionTwoCalculationsLabel->setVisible(show);
    ui.precisionOneCalculationsLabel->setVisible(show);

    ui.precisionFiveLabel->setVisible(show);
    ui.precisionFourLabel->setVisible(show);
    ui.precisionThreeLabel->setVisible(show);
    ui.precisionTwoLabel->setVisible(show);
    ui.precisionOneLabel->setVisible(show);

}


void KisPerformanceOption::updatePrecisionCalculationsTable()
{
    double startingValue = ui.precisionStartingSizeSpinbox->value();
    double deltaValue = ui.precisionDeltaSpinbox->value();

    // calculation is done slightly different if starting value is 0
    if (ui.precisionStartingSizeSpinbox->value() == 0) {
        ui.precisionFiveCalculationsLabel->setText(i18n("0 px to ").append(QString::number(deltaValue)));
        ui.precisionFourCalculationsLabel->setText( QString::number(deltaValue + 1).append(i18n(" px to ").append( QString::number(deltaValue*2))));
        ui.precisionThreeCalculationsLabel->setText( QString::number(deltaValue*2 + 1).append(i18n(" px to ")).append(QString::number(deltaValue*3)));
        ui.precisionTwoCalculationsLabel->setText( QString::number(deltaValue*3 + 1).append(i18n(" px to ")).append(QString::number(deltaValue*4)));
        ui.precisionOneCalculationsLabel->setText( QString::number(deltaValue*4 + 1).append(i18n(" px and higher")));
    } else {
         ui.precisionFiveCalculationsLabel->setText(i18n("0 px to ").append(QString::number(startingValue)));
         ui.precisionFourCalculationsLabel->setText( QString::number(startingValue + 1).append(i18n(" px to ").append( QString::number(startingValue + deltaValue))));
         ui.precisionThreeCalculationsLabel->setText( QString::number(startingValue + deltaValue + 1).append(i18n(" px to ")).append(QString::number(startingValue + deltaValue*2)));
         ui.precisionTwoCalculationsLabel->setText( QString::number(startingValue + deltaValue*2 + 1).append(i18n(" px to ")).append(QString::number(startingValue + deltaValue*3)));
         ui.precisionOneCalculationsLabel->setText( QString::number(startingValue + deltaValue*3 + 1).append(i18n(" px and higher")));
    }


}

void KisPerformanceOption::setSizeToStartFrom(double value)
{
    m_precisionOption.setSizeToStartFrom(value);

    updatePrecisionCalculationsTable();
    emitSettingChanged();
    emit sigPrecisionChanged();
}




