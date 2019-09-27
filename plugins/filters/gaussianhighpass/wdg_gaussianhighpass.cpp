/*
 * This file is part of Krita
 *
 * Copyright (c) 2019 Miguel Lopez <reptillia39@live.com>
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

#include "wdg_gaussianhighpass.h"
#include <QLayout>
#include <QToolButton>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "ui_wdggaussianhighpass.h"

KisWdgGaussianHighPass::KisWdgGaussianHighPass(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgGaussianHighPass();
    m_widget->setupUi(this);

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
    KisFilterConfigurationSP config = new KisFilterConfiguration("gaussianhighpass", 1);
    config->setProperty("blurAmount", widget()->doubleblurAmount->value());
    return config;
}


