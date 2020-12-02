/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 * 
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDomDocument>

#include <KoStopGradient.h>
#include <KoAbstractGradient.h>
#include <KisDitherWidget.h>

#include "KisGradientMapFilterConfiguration.h"

KisGradientMapFilterConfiguration::KisGradientMapFilterConfiguration()
    : KisFilterConfiguration(defaultName(), defaultVersion())
{}

KisGradientMapFilterConfiguration::KisGradientMapFilterConfiguration(qint32 version)
    : KisFilterConfiguration(defaultName(), version)
{}

KoStopGradientSP KisGradientMapFilterConfiguration::gradient() const
{
    if (version() == 1) {
        QDomDocument doc;
        QDomElement elt = doc.createElement("gradient");
        KoAbstractGradient *gradientAb = KoResourceServerProvider::instance()->gradientServer()->resourceByName(getString("gradientName"));
        if (!gradientAb) {
            qWarning() << "Could not find gradient" << getString("gradientName");
        }
        gradientAb = KoResourceServerProvider::instance()->gradientServer()->resources().first();
        QScopedPointer<QGradient> qGradient(gradientAb->toQGradient());
        KoStopGradient::fromQGradient(qGradient.data())->toXML(doc, elt);
        doc.appendChild(elt);
        KoStopGradientSP gradient =
            KoStopGradientSP(dynamic_cast<KoStopGradient*>(KoStopGradient::fromXML(doc.firstChildElement()).clone()));
        gradient->setValid(true);
        return gradient;
    } else if (version() == 2) {
        QDomDocument document;
        if (document.setContent(getString("gradientXML", ""))) {
            const QDomElement gradientElement = document.firstChildElement();
            if (!gradientElement.isNull()) {
                KoStopGradientSP gradient =
                    KoStopGradientSP(dynamic_cast<KoStopGradient*>(KoStopGradient::fromXML(gradientElement).clone()));
                if (gradient) {
                    gradient->setName(gradientElement.attribute("name", ""));
                    gradient->setValid(true);
                    return gradient;
                }
            }
        }
    }
    return defaultGradient();
}

int KisGradientMapFilterConfiguration::colorMode() const
{
    return getInt("colorMode", defaultColorMode());
}

void KisGradientMapFilterConfiguration::setGradient(KoStopGradientSP newGradient)
{
    if (!newGradient) {
        setProperty("gradientXML", "");
        return;
    }

    QDomDocument document;
    QDomElement gradientElement = document.createElement("gradient");
    gradientElement.setAttribute("name", newGradient->name());

    newGradient->toXML(document, gradientElement);

    document.appendChild(gradientElement);
    setProperty("gradientXML", document.toString());
}

void KisGradientMapFilterConfiguration::setColorMode(int newColorMode)
{
    setProperty("colorMode", newColorMode);
}

void KisGradientMapFilterConfiguration::setDefaults()
{
    setGradient(defaultGradient());
    setColorMode(defaultColorMode());
    KisDitherWidget::factoryConfiguration(*this, "dither/");
}
