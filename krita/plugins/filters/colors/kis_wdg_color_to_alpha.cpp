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
#include <QCheckBox>
#include <QLayout>
#include <QSpinBox>

#include <kcolorbutton.h>

#include <KoColor.h>

#include <kis_view2.h>
#include <kis_canvas_resource_provider.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include "ui_wdgcolortoalphabase.h"

KisWdgColorToAlpha::KisWdgColorToAlpha(QWidget * parent)
    : KisConfigWidget(parent),
      m_view(0)
{
    m_widget = new Ui_WdgColorToAlphaBase();
    m_widget->setupUi(this);
    connect(m_widget->colorSelector, SIGNAL(colorChanged(const QColor&)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->intThreshold, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgColorToAlpha::~KisWdgColorToAlpha()
{
    KoToolManager::instance()->switchBackRequested();
    delete m_widget;
}

void KisWdgColorToAlpha::setView(KisView2 *view)
{
    m_view = view;
    connect(view->resourceProvider(), SIGNAL(sigFGColorChanged(const KoColor&)), this, SLOT(slotFgColorChanged(const KoColor&)));

    KoToolManager::instance()->switchToolTemporaryRequested("KritaSelected/KisToolColorPicker");
}

void KisWdgColorToAlpha::slotFgColorChanged(const KoColor &color)
{
    m_widget->colorSelector->setQColor(color.toQColor());
    emit sigConfigurationItemChanged();
}

void KisWdgColorToAlpha::setConfiguration(const KisPropertiesConfiguration* config)
{
    QVariant value;
    if (config->getProperty("targetcolor", value)) {
        m_widget->colorSelector->setQColor(value.value<QColor>());
    }
    if (config->getProperty("threshold", value)) {
        m_widget->intThreshold->setValue(value.toInt());
    }
}

KisPropertiesConfiguration* KisWdgColorToAlpha::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("colortoalpha", 1);
    config->setProperty("targetcolor", widget()->colorSelector->color());
    config->setProperty("threshold", widget()->intThreshold->value());
    return config;
}


#include "kis_wdg_color_to_alpha.moc"
