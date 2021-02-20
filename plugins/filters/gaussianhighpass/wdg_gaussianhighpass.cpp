/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2019 Miguel Lopez <reptillia39@live.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "wdg_gaussianhighpass.h"
#include <QLayout>
#include <QToolButton>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdggaussianhighpass.h"

KisWdgGaussianHighPass::KisWdgGaussianHighPass(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgGaussianHighPass();
    m_widget->setupUi(this);
    widget()->doubleblurAmount->setRange(0.0, 250.0, 2);
    widget()->doubleblurAmount->setSingleStep(1.00);
    connect(widget()->doubleblurAmount, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgGaussianHighPass::~KisWdgGaussianHighPass()
{
    delete m_widget;
}

void KisWdgGaussianHighPass::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    widget()->doubleblurAmount->setValue((config->getProperty("blurAmount", value)) ? value.toDouble() : 1.0);
}

KisPropertiesConfigurationSP KisWdgGaussianHighPass::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("gaussianhighpass", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("blurAmount", widget()->doubleblurAmount->value());
    return config;
}


