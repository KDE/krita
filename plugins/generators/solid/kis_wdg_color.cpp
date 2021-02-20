/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_color.h"

#include <QLayout>

#include <KoColor.h>
#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

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
    KisFilterConfigurationSP config = new KisFilterConfiguration("color", 1, KisGlobalResourcesInterface::instance());
    KoColor c;
    c.fromKoColor(this->widget()->bnColor->color());
    QVariant v;
    v.setValue(c);
    config->setProperty("color", v);
    return config;
}


