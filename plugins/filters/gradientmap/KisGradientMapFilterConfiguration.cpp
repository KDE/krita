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

KoAbstractGradientSP KisGradientMapFilterConfiguration::gradient() const
{
    if (version() == 1) {
        KoAbstractGradient *resourceGradient = KoResourceServerProvider::instance()->gradientServer()->resourceByName(getString("gradientName"));
        if (resourceGradient) {
            KoAbstractGradientSP gradient = KoAbstractGradientSP(resourceGradient->clone());
            gradient->setValid(true);
            return gradient;
        } else {
            qWarning() << "Could not find gradient" << getString("gradientName");
        }
    } else if (version() == 2) {
        QDomDocument document;
        if (document.setContent(getString("gradientXML", ""))) {
            const QDomElement gradientElement = document.firstChildElement();
            if (!gradientElement.isNull()) {
                const QString gradientType = gradientElement.attribute("type");
                KoAbstractGradientSP gradient;
                if (gradientType == "stop") {
                    gradient = KoAbstractGradientSP(KoStopGradient::fromXML(gradientElement).clone());
                } else if (gradientType == "segment") {
                    gradient = KoAbstractGradientSP(KoSegmentGradient::fromXML(gradientElement).clone());
                }
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

void KisGradientMapFilterConfiguration::setGradient(KoAbstractGradientSP newGradient)
{
    if (!newGradient) {
        setProperty("gradientXML", "");
        return;
    }
    
    QDomDocument document;
    QDomElement gradientElement = document.createElement("gradient");
    gradientElement.setAttribute("name", newGradient->name());

    if (dynamic_cast<KoStopGradient*>(newGradient.data())) {
        KoStopGradient *gradient = dynamic_cast<KoStopGradient*>(newGradient.data());
        gradient->toXML(document, gradientElement);
    } else if (dynamic_cast<KoSegmentGradient*>(newGradient.data())) {
        KoSegmentGradient *gradient = dynamic_cast<KoSegmentGradient*>(newGradient.data());
        gradient->toXML(document, gradientElement);
    }

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
