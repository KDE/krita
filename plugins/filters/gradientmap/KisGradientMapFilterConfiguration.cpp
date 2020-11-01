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
