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
#include <KisDitherUtil.h>

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
        QScopedPointer<QGradient> qGradient(gradientAb->toQGradient());
        KoStopGradient::fromQGradient(qGradient.data())->toXML(doc, elt);
        doc.appendChild(elt);
    } else {
        doc.setContent(config->getString("gradientXML", ""));
    }
    KoStopGradient gradient = KoStopGradient::fromXML(doc.firstChildElement());

    const ColorMode colorMode = ColorMode(config->getInt("colorMode"));
    KisDitherUtil ditherUtil;
    if (colorMode == ColorMode::Dither) ditherUtil.setConfiguration(*config, "dither/");

    KoColor outColor(Qt::white, device->colorSpace());
    KisSequentialIteratorProgress it(device, applyRect, progressUpdater);
    qreal grey;
    const int pixelSize = device->colorSpace()->pixelSize();
    while (it.nextPixel()) {
        grey = qreal(device->colorSpace()->intensity8(it.oldRawData())) / 255;
        if (colorMode == ColorMode::Nearest) {
            KoGradientStop leftStop, rightStop;
            if (!gradient.stopsAt(leftStop, rightStop, grey)) continue;
            if (std::abs(grey - leftStop.first) < std::abs(grey - rightStop.first)) {
                outColor = leftStop.second;
            }
            else {
                outColor = rightStop.second;
            }
        }
        else if (colorMode == ColorMode::Dither) {
            KoGradientStop leftStop, rightStop;
            if (!gradient.stopsAt(leftStop, rightStop, grey)) continue;
            qreal localT = (grey - leftStop.first) / (rightStop.first - leftStop.first);
            if (localT < ditherUtil.threshold(QPoint(it.x(), it.y()))) {
                outColor = leftStop.second;
            }
            else {
                outColor = rightStop.second;
            }
        }
        else {
            gradient.colorAt(outColor, grey);
        }
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
    QScopedPointer<QGradient> qGradient(gradient->toQGradient());
    stopGradient.fromQGradient(qGradient.data());
    QDomDocument doc;
    QDomElement elt = doc.createElement("gradient");
    stopGradient.toXML(doc, elt);
    doc.appendChild(elt);
    config->setProperty("gradientXML", doc.toString());

    config->setProperty("colorMode", false);
    KisDitherWidget::factoryConfiguration(*config, "dither/");

    return config;
}

KisConfigWidget * KritaFilterGradientMap::createConfigurationWidget(QWidget * parent, const KisPaintDeviceSP dev, bool) const
{
    return new KritaGradientMapConfigWidget(parent, dev);
}

