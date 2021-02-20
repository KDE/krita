/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_wave.h"
#include <QSpinBox>

#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KisGlobalResourcesInterface.h>

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
    delete m_widget;
}

void KisWdgWave::setConfiguration(const KisPropertiesConfigurationSP config)
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

KisPropertiesConfigurationSP KisWdgWave::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("wave", 1, KisGlobalResourcesInterface::instance());
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


