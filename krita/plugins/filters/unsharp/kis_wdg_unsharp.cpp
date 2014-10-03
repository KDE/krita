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

#include "kis_wdg_unsharp.h"
#include <QLayout>
#include <QToolButton>

#include <knuminput.h>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "ui_wdgunsharp.h"

KisWdgUnsharp::KisWdgUnsharp(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgUnsharp();
    m_widget->setupUi(this);

    connect(widget()->doubleHalfSize, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->doubleAmount, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->intThreshold, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->chkLightnessOnly, SIGNAL(stateChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgUnsharp::~KisWdgUnsharp()
{
    delete m_widget;
}

void KisWdgUnsharp::setConfiguration(const KisPropertiesConfiguration* config)
{
    QVariant value;
    widget()->doubleHalfSize->setValue((config->getProperty("halfSize", value)) ? value.toDouble() : 1.0);
    widget()->doubleAmount->setValue((config->getProperty("amount", value)) ? value.toDouble() : 0.0);
    widget()->intThreshold->setValue((config->getProperty("threshold", value)) ? value.toUInt() : 25);
    widget()->chkLightnessOnly->setChecked((config->getProperty("lightnessOnly", value)) ? value.toBool() : true);
}

KisPropertiesConfiguration* KisWdgUnsharp::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("unsharp", 1);
    config->setProperty("halfSize", widget()->doubleHalfSize->value());
    config->setProperty("amount", widget()->doubleAmount->value());
    config->setProperty("threshold", widget()->intThreshold->value());
    config->setProperty("lightnessOnly", widget()->chkLightnessOnly->isChecked());
    return config;
}


#include "kis_wdg_unsharp.moc"
