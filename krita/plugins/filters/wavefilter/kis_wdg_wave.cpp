/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_wdg_wave.h"
#include <qspinbox.h>

#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "ui_wdgwaveoptions.h"

KisWdgWave::KisWdgWave(KisFilter* /*nfilter*/, QWidget* parent)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgWaveOptions();
    m_widget->setupUi(this);

    connect(widget()->intHWavelength, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intHShift, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intHAmplitude, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->cbHShape, SIGNAL(activated(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intVWavelength, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intVShift, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intVAmplitude, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->cbVShape, SIGNAL(activated(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgWave::~KisWdgWave()
{
}

void KisWdgWave::setConfiguration(const KisPropertiesConfiguration* config)
{
    QVariant value;
    if (config->getProperty("horizontalwavelength", value)) {
        widget()->intHWavelength->setValue(value.toUInt());
    }
    if (config->getProperty("horizontalshift", value)) {
        widget()->intHShift->setValue(value.toUInt());
    }
    if (config->getProperty("horizontalamplitude", value)) {
        widget()->intHAmplitude->setValue(value.toUInt());
    }
    if (config->getProperty("horizontalshape", value)) {
        widget()->cbHShape->setCurrentIndex(value.toUInt());
    }
    if (config->getProperty("verticalwavelength", value)) {
        widget()->intVWavelength->setValue(value.toUInt());
    }
    if (config->getProperty("verticalshift", value)) {
        widget()->intVShift->setValue(value.toUInt());
    }
    if (config->getProperty("verticalamplitude", value)) {
        widget()->intVAmplitude->setValue(value.toUInt());
    }
    if (config->getProperty("verticalshape", value)) {
        widget()->cbVShape->setCurrentIndex(value.toUInt());
    }
}

KisPropertiesConfiguration* KisWdgWave::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("wave", 1);
    config->setProperty("horizontalwavelength", this->widget()->intHWavelength->value());
    config->setProperty("horizontalshift", this->widget()->intHShift->value());
    config->setProperty("horizontalamplitude", this->widget()->intHAmplitude->value());
    config->setProperty("horizontalshape", this->widget()->cbHShape->currentIndex());
    config->setProperty("verticalwavelength", this->widget()->intVWavelength->value());
    config->setProperty("verticalshift", this->widget()->intVShift->value());
    config->setProperty("verticalamplitude", this->widget()->intVAmplitude->value());
    config->setProperty("verticalshape", this->widget()->cbVShape->currentIndex());
    return config;
}

#include "kis_wdg_wave.moc"

