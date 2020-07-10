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

    widget()->sldDivisions->setRange(0, 100);
    widget()->sldDivisions->setPrefix(i18n("Divisions:"));

    widget()->sldDimensions->setRange(3, 36);
    widget()->sldDimensions->setPrefix(i18n("Dimensions:"));

    widget()->sldOffset->setRange(0.01, 0.49, 2);
    widget()->sldOffset->setPrefix(i18n("Offset:"));

    widget()->sldColorRatio->setRange(-2.0, 2.0, 2);
    widget()->sldColorRatio->setPrefix(i18n("Ratio:"));

    widget()->sldColorIndex->setRange(-1.0, 1.0, 2);
    widget()->sldColorIndex->setPrefix(i18n("Index:"));

    widget()->sldColorIntersect->setRange(0.0, 1.0, 2);
    widget()->sldColorIntersect->setPrefix(i18n("Intersect:"));

    connect(m_widget->bnColor1, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->bnColor2, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldDivisions, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldDimensions, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldOffset, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->sldColorIndex, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldColorRatio, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldColorIntersect, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->spnLineWidth, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->bnLineColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
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

    c = config->getColor("lineColor");
    c.convertTo(m_cs);
    widget()->bnLineColor->setColor(c);

    widget()->spnLineWidth->setValue(config->getInt("lineWidth", 1));

    widget()->sldDivisions->setValue(config->getInt("divisions", 1));

    widget()->sldDimensions->setValue(config->getInt("dimensions", 5));
    widget()->sldOffset->setValue(config->getFloat("offset", 0.2));

    widget()->sldColorIndex->setValue(config->getFloat("colorIndex", 1.0));
    widget()->sldColorRatio->setValue(config->getFloat("colorRatio", 0.0));
    widget()->sldColorIntersect->setValue(config->getFloat("colorIntersect", 0.0));
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

    c.fromKoColor(widget()->bnLineColor->color());
    v.setValue(c);
    config->setProperty("lineColor", v);
    config->setProperty("lineWidth", widget()->spnLineWidth->value());

    config->setProperty("divisions", widget()->sldDivisions->value());

    config->setProperty("dimensions", widget()->sldDimensions->value());
    config->setProperty("offset", widget()->sldOffset->value());

    config->setProperty("colorRatio", widget()->sldColorRatio->value());
    config->setProperty("colorIndex", widget()->sldColorIndex->value());
    config->setProperty("colorIntersect", widget()->sldColorIntersect->value());

    return config;
}


