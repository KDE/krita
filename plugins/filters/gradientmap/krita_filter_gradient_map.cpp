/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
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

#include "krita_filter_gradient_map.h"
#include <KoColorSpace.h>
#include <KoColor.h>

#include <kis_paint_device.h>
#include <kis_global.h>
#include <kis_types.h>

#include <filter/kis_filter_category_ids.h>
#include "kis_config_widget.h"
#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoColorSet.h>
#include "gradientmap.h"

#include <KisSequentialIteratorProgress.h>


KritaFilterGradientMap::KritaFilterGradientMap() : KisFilter(id(), FiltersCategoryMapId, i18n("&Gradient Map..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setShowConfigurationWidget(true);
    setSupportsLevelOfDetail(true);
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsThreading(true);
}

void KritaFilterGradientMap::processImpl(KisPaintDeviceSP device,
                 const QRect& applyRect,
                 const KisFilterConfigurationSP config,
                 KoUpdater *progressUpdater) const
{
    Q_ASSERT(!device.isNull());

    QDomDocument doc;
    if (config->version()==1) {
        QDomElement elt = doc.createElement("gradient");
        KoAbstractGradient *gradientAb = KoResourceServerProvider::instance()->gradientServer()->resourceByName(config->getString("gradientName"));
            if (!gradientAb) {
                qWarning() << "Could not find gradient" << config->getString("gradientName");
            }
        gradientAb = KoResourceServerProvider::instance()->gradientServer()->resources().first();
        KoStopGradient::fromQGradient(gradientAb->toQGradient())->toXML(doc, elt);
        doc.appendChild(elt);
    } else {
        doc.setContent(config->getString("gradientXML", ""));
    }
   KoStopGradient gradient = KoStopGradient::fromXML(doc.firstChildElement());



    KoColor outColor(Qt::white, device->colorSpace());
    KisSequentialIteratorProgress it(device, applyRect, progressUpdater);
    quint8 grey;
    const int pixelSize = device->colorSpace()->pixelSize();
    while (it.nextPixel()) {
        grey = device->colorSpace()->intensity8(it.oldRawData());
        gradient.colorAt(outColor,(qreal)grey/255);
        outColor.setOpacity(qMin(KoColor(it.oldRawData(), device->colorSpace()).opacityF(), outColor.opacityF()));
        outColor.convertTo(device->colorSpace());
        memcpy(it.rawData(), outColor.data(), pixelSize);
    }

}

KisFilterConfigurationSP KritaFilterGradientMap::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("gradientmap", 2);
    KoAbstractGradient *gradient = KoResourceServerProvider::instance()->gradientServer()->resources().first();
    KoStopGradient stopGradient;
    stopGradient.fromQGradient(gradient->toQGradient());
    QDomDocument doc;
    QDomElement elt = doc.createElement("gradient");
    stopGradient.toXML(doc, elt);
    doc.appendChild(elt);
    config->setProperty("gradientXML", doc.toString());
    return config;
}

KisConfigWidget * KritaFilterGradientMap::createConfigurationWidget(QWidget * parent, const KisPaintDeviceSP dev) const
{
    return new KritaGradientMapConfigWidget(parent, dev);
}

