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

#include "kis_config_widget.h"
#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoColorSet.h>
#include "gradientmap.h"

#include <kis_sequential_iterator.h>


KritaFilterGradientMap::KritaFilterGradientMap() : KisFilter(id(), categoryMap(), i18n("&Gradient Map..."))
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

    if (progressUpdater) {
        progressUpdater->setRange(0, applyRect.height() * applyRect.width());
    }

    QDomDocument doc;
    if (config->version()==1) {
        QDomElement elt = doc.createElement("gradient");
        KoAbstractGradient *gradientAb = KoResourceServerProvider::instance()->gradientServer(false)->resourceByName(config->getString("gradientName"));
            if (!gradientAb) {
                qDebug() << "Could not find gradient" << config->getString("gradientName");
            }
        gradientAb = KoResourceServerProvider::instance()->gradientServer(false)->resources().first();
        KoStopGradient::fromQGradient(gradientAb->toQGradient())->toXML(doc, elt);
        doc.appendChild(elt);
    } else {
        doc.setContent(config->getString("gradientXML", ""));
    }
   KoStopGradient gradient = KoStopGradient::fromXML(doc.firstChildElement());


    KoColorSet *gradientCache = new KoColorSet();
    for (int i=0; i<256; i++) {
        KoColor gc;
        gradient.colorAt(gc, ((qreal)i/255.0));
        KoColorSetEntry col;
        col.color = gc;
        gradientCache->add(col);
    }

    KoColor outColor(Qt::white, device->colorSpace());
    KisSequentialIterator it(device, applyRect);
    int p = 0;
    quint8 grey;
    const int pixelSize = device->colorSpace()->pixelSize();
    do {
        grey = device->colorSpace()->intensity8(it.oldRawData());
        outColor = gradientCache->getColorGlobal((quint32)grey).color;
        outColor.setOpacity(qMin(KoColor(it.oldRawData(), device->colorSpace()).opacityF(), outColor.opacityF()));
        outColor.convertTo(device->colorSpace());
        memcpy(it.rawData(), outColor.data(), pixelSize);
        if (progressUpdater) progressUpdater->setValue(p++);

    } while (it.nextPixel());

}

KisFilterConfigurationSP KritaFilterGradientMap::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("gradientmap", 2);
    KoAbstractGradient *gradient = KoResourceServerProvider::instance()->gradientServer(false)->resources().first();
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

