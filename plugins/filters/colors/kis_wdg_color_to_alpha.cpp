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

#include <KoColor.h>
#include <KoToolManager.h>

#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KoColorSpaceRegistry.h>

#include "ui_wdgcolortoalphabase.h"

KisWdgColorToAlpha::KisWdgColorToAlpha(QWidget * parent)
    : KisConfigWidget(parent),
      m_view(0)
{
    m_widget = new Ui_WdgColorToAlphaBase();
    m_widget->setupUi(this);

    m_widget->textLabel1->hide();

    m_widget->intThreshold->setRange(1, 255, 0);

    connect(m_widget->colorSelector, SIGNAL(sigNewColor(KoColor)), SLOT(slotColorSelectorChanged(KoColor)));
    connect(m_widget->intThreshold, SIGNAL(valueChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->btnCustomColor, SIGNAL(changed(KoColor)), SLOT(slotCustomColorSelected(KoColor)));

    KoColor c(Qt::white, KoColorSpaceRegistry::instance()->rgb8());
    m_widget->btnCustomColor->setColor(c);
}

KisWdgColorToAlpha::~KisWdgColorToAlpha()
{
    delete m_widget;
}

void KisWdgColorToAlpha::setView(KisViewManager *view)
{
    m_view = view;
}

void KisWdgColorToAlpha::slotFgColorChanged(const KoColor &color)
{
    m_widget->btnCustomColor->setColor(color);
}

void KisWdgColorToAlpha::slotColorSelectorChanged(const KoColor &color)
{
    m_widget->btnCustomColor->setColor(color);
}

void KisWdgColorToAlpha::slotCustomColorSelected(const KoColor &color)
{
    KoColor c(color, KoColorSpaceRegistry::instance()->rgb8());
    m_widget->colorSelector->slotSetColor(color);
    emit sigConfigurationItemChanged();
}

void KisWdgColorToAlpha::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("targetcolor", value)) {
        KoColor c;
        if (value.value<QColor>().isValid()) {
            c = KoColor(value.value<QColor>(), KoColorSpaceRegistry::instance()->rgb8());
        } else {
            c = value.value<KoColor>();
        }
        m_widget->colorSelector->slotSetColor(c);
    }
    if (config->getProperty("threshold", value)) {
        m_widget->intThreshold->setValue(value.toInt());
    }
}

KisPropertiesConfigurationSP KisWdgColorToAlpha::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("colortoalpha", 1);
    config->setProperty("targetcolor", widget()->colorSelector->getCurrentColor().toQColor());
    config->setProperty("threshold", widget()->intThreshold->value());
    return config;
}

void KisWdgColorToAlpha::hideEvent(QHideEvent *)
{
    if (m_view) {
        disconnect(m_view->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), this, SLOT(slotFgColorChanged(KoColor)));
    }
}

void KisWdgColorToAlpha::showEvent(QShowEvent *)
{
    if (m_view) {
        connect(m_view->canvasResourceProvider(), SIGNAL(sigFGColorChanged(KoColor)), this, SLOT(slotFgColorChanged(KoColor)));
    }
}


