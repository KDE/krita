/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_multigrid_pattern.h"

#include <QLayout>
#include <QDomDocument>
#include <QDomElement>

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

    QLinearGradient grad;
    grad.setColorAt(0, Qt::green);
    grad.setColorAt(1.0, Qt::blue);
    m_gradient = KoStopGradient::fromQGradient(&grad);
    widget()->wdgGradient->setGradient(m_gradient);
    widget()->wdgGradient->setCompactMode(true);

    widget()->sldDivisions->setRange(0, 100);
    widget()->sldDivisions->setPrefix(i18n("Divisions:"));

    widget()->sldDimensions->setRange(3, 36);
    widget()->sldDimensions->setPrefix(i18nc("The Dimensions of Multigrid pattern generator", "Dimensions:"));

    widget()->sldOffset->setRange(0.01, 0.49, 2);
    widget()->sldOffset->setPrefix(i18n("Offset:"));

    widget()->sldColorRatio->setRange(-2.0, 2.0, 2);
    widget()->sldColorRatio->setPrefix(i18nc("Ratio as in fill layer options", "Ratio:"));

    widget()->sldColorIndex->setRange(-2.0, 2.0, 2);
    widget()->sldColorIndex->setPrefix(i18nc("Index number of how far away from center", "Index:"));

    widget()->sldColorIntersect->setRange(-2.0, 2.0, 2);
    widget()->sldColorIntersect->setPrefix(i18n("Intersect:"));

    widget()->cmbConnectorType->addItem(i18n("No Connectors"));
    widget()->cmbConnectorType->addItem(i18n("Acute Angles"));
    widget()->cmbConnectorType->addItem(i18n("Obtuse Angles"));
    widget()->cmbConnectorType->addItem(i18n("Cross Shape"));
    widget()->cmbConnectorType->addItem(i18n("Center Dot"));
    widget()->cmbConnectorType->addItem(i18n("Corner Dot"));

    connect(m_widget->sldDivisions, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldDimensions, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldOffset, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->sldColorIndex, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldColorRatio, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldColorIntersect, SIGNAL(valueChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->wdgGradient, SIGNAL(sigGradientChanged()), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->spnLineWidth, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->bnLineColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->spnConnectorWidth, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->bnConnectorColor, SIGNAL(changed(const KoColor&)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->cmbConnectorType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
}

KisWdgMultigridPattern::~KisWdgMultigridPattern()
{
    delete m_widget;
}


void KisWdgMultigridPattern::setConfiguration(const KisPropertiesConfigurationSP config)
{
    if (config->hasProperty("gradientXML")) {
        QDomDocument doc;
        doc.setContent(config->getString("gradientXML", ""));
        KoStopGradient gradient = KoStopGradient::fromXML(doc.firstChildElement());
        if (gradient.stops().size() > 0) {
            m_gradient->setStops(gradient.stops());
        }
        widget()->wdgGradient->setGradient(m_gradient);
    }


    KoColor c = config->getColor("lineColor");
    c.convertTo(m_cs);
    widget()->bnLineColor->setColor(c);

    widget()->spnLineWidth->setValue(config->getInt("lineWidth", 1));

    widget()->sldDivisions->setValue(config->getInt("divisions", 1));

    widget()->sldDimensions->setValue(config->getInt("dimensions", 5));
    widget()->sldOffset->setValue(config->getFloat("offset", 0.2));

    widget()->sldColorIndex->setValue(config->getFloat("colorIndex", 1.0));
    widget()->sldColorRatio->setValue(config->getFloat("colorRatio", 0.0));
    widget()->sldColorIntersect->setValue(config->getFloat("colorIntersect", 0.0));

    c = config->getColor("connectorColor");
    c.convertTo(m_cs);
    widget()->bnConnectorColor->setColor(c);
    widget()->cmbConnectorType->setCurrentIndex(config->getInt("connectorType", 1));
    widget()->spnConnectorWidth->setValue(config->getInt("connectorWidth", 1));
}

KisPropertiesConfigurationSP KisWdgMultigridPattern::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("multigrid", 1, KisGlobalResourcesInterface::instance());

    if (m_gradient) {
        QDomDocument doc;
        QDomElement elt = doc.createElement("gradient");
        m_gradient->toXML(doc, elt);
        doc.appendChild(elt);
        config->setProperty("gradientXML", doc.toString());
    }

    QVariant v;
    KoColor c;
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

    c.fromKoColor(widget()->bnConnectorColor->color());
    v.setValue(c);
    config->setProperty("connectorColor", v);
    config->setProperty("connectorType", widget()->cmbConnectorType->currentIndex());
    config->setProperty("connectorWidth", widget()->spnConnectorWidth->value());

    return config;
}


