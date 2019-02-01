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

#include "kis_wdg_color.h"

#include <QLayout>

#include <KoColor.h>
#include <filter/kis_filter_configuration.h>

#include "ui_wdgcoloroptions.h"

KisWdgColor::KisWdgColor(QWidget* parent, const KoColorSpace *cs)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgColorOptions();
    m_widget->setupUi(this);
    m_cs = cs;
    connect(m_widget->bnColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
}

KisWdgColor::~KisWdgColor()
{
    delete m_widget;
}


void KisWdgColor::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    KoColor c =config->getColor("color");
    c.convertTo(m_cs);
    widget()->bnColor->setColor(c);
}

KisPropertiesConfigurationSP KisWdgColor::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("color", 1);
    KoColor c;
    c.fromKoColor(this->widget()->bnColor->color());
    QVariant v;
    v.setValue(c);
    config->setProperty("color", v);
    return config;
}


