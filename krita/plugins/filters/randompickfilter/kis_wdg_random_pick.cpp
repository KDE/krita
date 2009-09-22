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

#include "kis_wdg_random_pick.h"


#include <qlayout.h>

#include <knuminput.h>

#include <filter/kis_filter_configuration.h>

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
}

void KisWdgRandomPick::setConfiguration(const KisPropertiesConfiguration* config)
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


KisPropertiesConfiguration* KisWdgRandomPick::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("randompick", 1);
    config->setProperty("level", this->widget()->intLevel->value());
    config->setProperty("windowsize", this->widget()->intWindowSize->value());
    config->setProperty("opacity", this->widget()->intOpacity->value());
    config->setProperty("seedH", m_seedH);
    config->setProperty("seedV", m_seedV);
    config->setProperty("seedThreshold", m_seedThreshold);
    return config;
}

#include "kis_wdg_random_pick.moc"

