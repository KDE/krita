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

#include "kis_wdg_color_to_alpha.h"
#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h>

#include <kcolorbutton.h>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include "ui_wdgcolortoalphabase.h"

KisWdgColorToAlpha::KisWdgColorToAlpha(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgColorToAlphaBase();
    m_widget->setupUi(this);
    connect(m_widget->colorTarget, SIGNAL(changed(const QColor&)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->intThreshold, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

void KisWdgColorToAlpha::setConfiguration(const KisPropertiesConfiguration* config)
{
    QVariant value;
    if (config->getProperty("targetcolor", value)) {
        m_widget->colorTarget->setColor(value.value<QColor>());
    }
    if (config->getProperty("threshold", value)) {
        m_widget->intThreshold->setValue(value.toInt());
    }
}

KisPropertiesConfiguration* KisWdgColorToAlpha::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("colortoalpha", 1);
    config->setProperty("targetcolor", widget()->colorTarget->color());
    config->setProperty("threshold", widget()->intThreshold->value());
    return config;
}


#include "kis_wdg_color_to_alpha.moc"
