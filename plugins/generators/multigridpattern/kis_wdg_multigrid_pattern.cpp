/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#include "kis_wdg_multigrid_pattern.h"

#include <QLayout>

#include <KoColor.h>
#include <filter/kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdgmultigridpatternoptions.h"

KisWdgMultigridPattern::KisWdgMultigridPattern(QWidget* parent, const KoColorSpace *cs)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgMultigridPatternOptions();
    m_widget->setupUi(this);
    m_cs = cs;
    connect(m_widget->bnColor1, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->bnColor2, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->spnDivisions, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->spnDimensions, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->spnOffset, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
}

KisWdgMultigridPattern::~KisWdgMultigridPattern()
{
    delete m_widget;
}


void KisWdgMultigridPattern::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    KoColor c =config->getColor("color1");
    c.convertTo(m_cs);
    widget()->bnColor1->setColor(c);

    c = config->getColor("color2");
    c.convertTo(m_cs);
    widget()->bnColor2->setColor(c);

    widget()->spnDivisions->setValue(config->getInt("divisions", 1));

    widget()->spnDimensions->setValue(config->getInt("dimensions", 5));
    widget()->spnOffset->setValue(config->getFloat("offset", 0.2));
}

KisPropertiesConfigurationSP KisWdgMultigridPattern::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("multigrid", 1, KisGlobalResourcesInterface::instance());
    KoColor c;
    c.fromKoColor(widget()->bnColor1->color());
    QVariant v;
    v.setValue(c);
    config->setProperty("color1", v);

    c.fromKoColor(widget()->bnColor2->color());
    v.setValue(c);
    config->setProperty("color2", v);

    config->setProperty("divisions", widget()->spnDivisions->value());

    config->setProperty("dimensions", widget()->spnDimensions->value());
    config->setProperty("offset", widget()->spnOffset->value());

    return config;
}


