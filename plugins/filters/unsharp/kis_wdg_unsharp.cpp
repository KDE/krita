/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_unsharp.h"
#include <QLayout>
#include <QToolButton>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdgunsharp.h"

KisWdgUnsharp::KisWdgUnsharp(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgUnsharp();
    m_widget->setupUi(this);

    widget()->doubleHalfSize->setRange(0.0, 99.99, 2);
    widget()->doubleHalfSize->setSingleStep(1.0);
    widget()->doubleAmount->setRange(0.0, 99.99, 2);
    widget()->doubleAmount->setSingleStep(0.2);

    connect(widget()->doubleHalfSize, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->doubleAmount, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intThreshold, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->chkLightnessOnly, SIGNAL(stateChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgUnsharp::~KisWdgUnsharp()
{
    delete m_widget;
}

void KisWdgUnsharp::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    widget()->doubleHalfSize->setValue((config->getProperty("halfSize", value)) ? value.toDouble() : 1.0);
    widget()->doubleAmount->setValue((config->getProperty("amount", value)) ? value.toDouble() : 0.0);
    widget()->intThreshold->setValue((config->getProperty("threshold", value)) ? value.toUInt() : 2);
    widget()->chkLightnessOnly->setChecked((config->getProperty("lightnessOnly", value)) ? value.toBool() : true);
}

KisPropertiesConfigurationSP KisWdgUnsharp::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("unsharp", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("halfSize", widget()->doubleHalfSize->value());
    config->setProperty("amount", widget()->doubleAmount->value());
    config->setProperty("threshold", widget()->intThreshold->value());
    config->setProperty("lightnessOnly", widget()->chkLightnessOnly->isChecked());
    return config;
}


