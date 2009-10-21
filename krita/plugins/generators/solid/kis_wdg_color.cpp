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

#include <qlayout.h>

#include <knuminput.h>
#include <KoColor.h>
#include <filter/kis_filter_configuration.h>

#include "ui_wdgcoloroptions.h"

KisWdgColor::KisWdgColor(QWidget* parent)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgColorOptions();
    m_widget->setupUi(this);
}

KisWdgColor::~KisWdgColor()
{
}

void KisWdgColor::setConfiguration(const KisPropertiesConfiguration* config)
{
    QVariant value;
    if (config->getProperty("color", value)) {
        widget()->bnColor->setColor(value.value<KoColor>().toQColor());
    }
}

KisPropertiesConfiguration* KisWdgColor::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("color", 1);
    KoColor c;
    c.fromQColor(this->widget()->bnColor->color());
    QVariant v;
    v.setValue(c);
    config->setProperty("color", v);
    return config;
}

#include "kis_wdg_color.moc"

