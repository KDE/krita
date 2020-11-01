/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
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

#include <QDomDocument>

#include <KoStopGradient.h>
#include <KoAbstractGradient.h>
#include <KisResourcesInterface.h>
#include <KisDitherWidget.h>

#include "KisGradientMapFilterConfiguration.h"

KisGradientMapFilterConfiguration::KisGradientMapFilterConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface)
    : KisFilterConfiguration(name, version, resourcesInterface)
{
}

KisGradientMapFilterConfiguration::KisGradientMapFilterConfiguration(const KisGradientMapFilterConfiguration &rhs)
    : KisFilterConfiguration(rhs)
{
}

KisFilterConfigurationSP KisGradientMapFilterConfiguration::clone() const
{
    return new KisGradientMapFilterConfiguration(*this);
}

KoStopGradientSP KisGradientMapFilterConfiguration::gradientImpl(KisResourcesInterfaceSP resourcesInterface) const
{
    KoStopGradientSP gradient;

    if (this->version() == 1) {
        auto source = resourcesInterface->source<KoAbstractGradient>(ResourceType::Gradients);

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

KoStopGradientSP KisGradientMapFilterConfiguration::gradient() const
{
    return gradientImpl(resourcesInterface());
}

QList<KoResourceSP> KisGradientMapFilterConfiguration::linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceSP> resources;

    // only the first version of the filter loaded the gradient by name
    if (this->version() == 1) {
        KoStopGradientSP gradient = gradientImpl(globalResourcesInterface);
        if (gradient) {
            resources << gradient;
        }
    }

    resources << KisDitherWidget::prepareLinkedResources(*this, "dither/", globalResourcesInterface);

    return resources;
}

QList<KoResourceSP> KisGradientMapFilterConfiguration::embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const
{
    QList<KoResourceSP> resources;

    // the second version of the filter embeds the gradient
    if (this->version() > 1) {
        KoStopGradientSP gradient = gradientImpl(globalResourcesInterface);

        if (gradient) {
            resources << gradient;
        }
    }

    return resources;
}
