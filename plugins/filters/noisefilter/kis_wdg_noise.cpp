/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_noise.h"

#include <QLayout>

#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdgnoiseoptions.h"

KisWdgNoise::KisWdgNoise(KisFilter* /*filter*/, QWidget* parent)
        : KisConfigWidget(parent)
{
    m_widget.reset(new Ui_WdgNoiseOptions());
    m_widget->setupUi(this);

    connect(m_widget->intLevel, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->intOpacity, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->chkGrayscale, SIGNAL(stateChanged(int)), SIGNAL(sigConfigurationItemChanged()));

    m_seedThreshold = rand();
    m_seedRed = rand();
    m_seedGreen = rand();
    m_seedBlue = rand();
    m_isGrayscale = false;
}

KisWdgNoise::~KisWdgNoise() = default;

void KisWdgNoise::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("level", value)) {
        m_widget->intLevel->setValue(value.toUInt());
    }
    if (config->getProperty("opacity", value)) {
        m_widget->intOpacity->setValue(value.toUInt());
    }
    if (config->getProperty("grayscale", value)) {
        m_widget->chkGrayscale->setChecked(value.toBool());
    }
}

KisPropertiesConfigurationSP KisWdgNoise::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("noise", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("level", m_widget->intLevel->value());
    config->setProperty("opacity", m_widget->intOpacity->value());
    config->setProperty("seedThreshold", m_seedThreshold);
    config->setProperty("seedRed", m_seedRed);
    config->setProperty("seedGreen", m_seedGreen);
    config->setProperty("seedBlue", m_seedBlue);
    config->setProperty("grayscale", m_widget->chkGrayscale->isChecked());
    return config;
}


