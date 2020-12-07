/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Spencer Brown <sbrown655@gmail.com>
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 * 
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDomDocument>

#include <KoStopGradient.h>
#include <KoAbstractGradient.h>
#include <KisDitherWidget.h>

#include "KisGradientMapFilterConfiguration.h"

KisGradientMapFilterConfiguration::KisGradientMapFilterConfiguration(KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(defaultName(), defaultVersion(), resourcesInterface)
{}

KisGradientMapFilterConfiguration::KisGradientMapFilterConfiguration(qint32 version, KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(defaultName(), version, resourcesInterface)
{}

KisGradientMapFilterConfiguration::KisGradientMapFilterConfiguration(const KisGradientMapFilterConfiguration &rhs)
    : KisFilterConfiguration(rhs)
{}

KisFilterConfigurationSP KisGradientMapFilterConfiguration::clone() const
{
    return new KisGradientMapFilterConfiguration(*this);
}

QList<KoResourceSP> KisGradientMapFilterConfiguration::linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceSP> resources;

    // only the first version of the filter loaded the gradient by name
    if (this->version() == 1) {
        KoStopGradientSP gradient = this->gradient();
        if (gradient) {
            resources << gradient;
        }
    }

    resources << KisDitherWidget::prepareLinkedResources(*this, "dither/", globalResourcesInterface);

    return resources;
}

QList<KoResourceSP> KisGradientMapFilterConfiguration::embeddedResources(KisResourcesInterfaceSP) const
{
    QList<KoResourceSP> resources;

    // the second version of the filter embeds the gradient
    if (this->version() > 1) {
        KoStopGradientSP gradient = this->gradient();

        if (gradient) {
            resources << gradient;
        }
    }

    return resources;
}

KoStopGradientSP KisGradientMapFilterConfiguration::gradient() const
{
    KoStopGradientSP gradient;

    if (this->version() == 1) {
        auto source = resourcesInterface()->source<KoAbstractGradient>(ResourceType::Gradients);

        KoAbstractGradientSP gradientAb = source.resourceForName(this->getString("gradientName"));
        if (!gradientAb) {
            qWarning() << "Could not find gradient" << this->getString("gradientName");
            gradientAb = source.fallbackResource();
        }

        gradient = gradientAb.dynamicCast<KoStopGradient>();

        if (!gradient) {
            QScopedPointer<QGradient> qGradient(gradientAb->toQGradient());

            QDomDocument doc;
            QDomElement elt = doc.createElement("gradient");
            KoStopGradient::fromQGradient(qGradient.data())->toXML(doc, elt);
            doc.appendChild(elt);

            gradient = KoStopGradient::fromXML(doc.firstChildElement())
                    .clone()
                    .dynamicCast<KoStopGradient>();
        }
    } else {
        QDomDocument doc;
        doc.setContent(this->getString("gradientXML", ""));
        gradient = KoStopGradient::fromXML(doc.firstChildElement())
                .clone()
                .dynamicCast<KoStopGradient>();

    }
    return gradient;
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
    setGradient(defaultGradient(resourcesInterface()));
    setColorMode(defaultColorMode());
    KisDitherWidget::factoryConfiguration(*this, "dither/");
}
