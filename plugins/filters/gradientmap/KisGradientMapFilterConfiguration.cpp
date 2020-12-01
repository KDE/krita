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
#include <KoResourceServerProvider.h>

#include "KisGradientMapFilterConfiguration.h"

KisGradientMapFilterConfiguration::KisGradientMapFilterConfiguration(const QString & name, qint32 version)
    : KisFilterConfiguration(name, version)
{
}

KoStopGradientSP KisGradientMapFilterConfiguration::gradientImpl() const
{
    KoStopGradientSP gradient;

    if (this->version() == 1) {
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
        gradient = KoStopGradientSP(dynamic_cast<KoStopGradient*>(KoStopGradient::fromXML(doc.firstChildElement()).clone()));
    } else {
        QDomDocument doc;
        doc.setContent(getString("gradientXML", ""));
        gradient = KoStopGradientSP(dynamic_cast<KoStopGradient*>(KoStopGradient::fromXML(doc.firstChildElement()).clone()));
    }
    return gradient;
}

KoStopGradientSP KisGradientMapFilterConfiguration::gradient() const
{
    return gradientImpl();
}
