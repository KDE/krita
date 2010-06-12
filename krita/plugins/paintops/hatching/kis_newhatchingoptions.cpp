 
 
/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2010 José Luis Vergara <pentalis@gmail.com>
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
#include "kis_newhatchingoptions.h"
#include <klocale.h>

#include "ui_newhatchingoptions.h"

class KisNewHatchingOptionsWidget: public QWidget, public Ui::NewHatchingOptions
{
public:
    KisNewHatchingOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
/*
        pigmentRetentionSlider->setRange(0.0, 100.0, 0);
        pigmentRetentionSlider->setValue(40.0);
        pigmentRetentionSlider->setSuffix(QChar(Qt::Key_Percent));
        pigmentRetentionSlider->setSingleStep(1);
*/
    }
};

KisNewHatchingOptions::KisNewHatchingOptions()
        : KisPaintOpOption(i18n("Other settings"), KisPaintOpOption::brushCategory(), false)
{
    m_checkable = false;
    m_options = new KisNewHatchingOptionsWidget();

    // signals
/*    connect(m_options->mousePressureCBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->thresholdCBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->pigmentRetentionSlider, SIGNAL(valueChanged(qreal)), SIGNAL(sigSettingChanged()));
*/
    setConfigurationPage(m_options);
}

KisNewHatchingOptions::~KisNewHatchingOptions()
{
  //  delete m_options;
}



void KisNewHatchingOptions::readOptionSetting(const KisPropertiesConfiguration* config)
{/*
    m_options->thresholdCBox->setChecked(config->getBool(HAIRY_BRISTLE_THRESHOLD));
    m_options->mousePressureCBox->setChecked(config->getBool(HAIRY_BRISTLE_USE_MOUSEPRESSURE));
    m_options->shearBox->setValue(config->getDouble(HAIRY_BRISTLE_SHEAR));
    m_options->rndBox->setValue(config->getDouble(HAIRY_BRISTLE_RANDOM));
    m_options->scaleBox->setValue(config->getDouble(HAIRY_BRISTLE_SCALE));
    m_options->pigmentRetentionSlider->setValue(config->getDouble(PIGMENT_RETENTION));
*/
}


void KisNewHatchingOptions::writeOptionSetting(KisPropertiesConfiguration* config) const
{/*
    config->setProperty(HAIRY_BRISTLE_THRESHOLD,m_options->thresholdCBox->isChecked());
    config->setProperty(HAIRY_BRISTLE_USE_MOUSEPRESSURE,m_options->mousePressureCBox->isChecked());
    config->setProperty(HAIRY_BRISTLE_SCALE,m_options->scaleBox->value());
    config->setProperty(HAIRY_BRISTLE_SHEAR,m_options->shearBox->value());
    config->setProperty(HAIRY_BRISTLE_RANDOM,m_options->rndBox->value());
    config->setProperty(PIGMENT_RETENTION,m_options->pigmentRetentionSlider->value());
*/
}

