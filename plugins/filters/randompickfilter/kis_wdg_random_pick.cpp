/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_random_pick.h"


#include <QLayout>

#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdgrandompickoptions.h"

KisWdgRandomPick::KisWdgRandomPick(KisFilter* /*nfilter*/, QWidget* parent)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgRandomPickOptions();
    m_widget->setupUi(this);

    connect(widget()->intLevel, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intWindowSize, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intOpacity, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    m_seedH = rand();
    m_seedV = rand();
    m_seedThreshold = rand();
}

KisWdgRandomPick::~KisWdgRandomPick()
{
    delete m_widget;
}

void KisWdgRandomPick::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("level", value)) {
        widget()->intLevel->setValue(value.toUInt());
    }
    if (config->getProperty("windowsize", value)) {
        widget()->intWindowSize->setValue(value.toUInt());
    }
    if (config->getProperty("opacity", value)) {
        widget()->intOpacity->setValue(value.toUInt());
    }
}


KisPropertiesConfigurationSP KisWdgRandomPick::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("randompick", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("level", this->widget()->intLevel->value());
    config->setProperty("windowsize", this->widget()->intWindowSize->value());
    config->setProperty("opacity", this->widget()->intOpacity->value());
    config->setProperty("seedH", m_seedH);
    config->setProperty("seedV", m_seedV);
    config->setProperty("seedThreshold", m_seedThreshold);
    return config;
}


