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

#include "wdg_guassianhighpass.h"
#include <QLayout>
#include <QToolButton>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "ui_wdgguassianhighpass.h"

KisWdgGuassianHighPass::KisWdgGuassianHighPass(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgGuassianHighPass();
    m_widget->setupUi(this);

    connect(widget()->doubleblurAmount, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgGuassianHighPass::~KisWdgGuassianHighPass()
{
    delete m_widget;
}

void KisWdgGuassianHighPass::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    widget()->doubleblurAmount->setValue((config->getProperty("blurAmount", value)) ? value.toDouble() : 1.0);
}

KisPropertiesConfigurationSP KisWdgGuassianHighPass::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("guassianhighpass", 1);
    config->setProperty("blurAmount", widget()->doubleblurAmount->value());
    return config;
}


