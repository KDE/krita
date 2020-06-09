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
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoColorSet.h>
#include "gradientmap.h"
#include <KisDitherUtil.h>
#include <KisGlobalResourcesInterface.h>
#include <QDomDocument>

#include <KisSequentialIteratorProgress.h>

class KritaFilterGradientMapConfiguration : public KisFilterConfiguration
{
public:
    KritaFilterGradientMapConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface)
        : KisFilterConfiguration(name, version, resourcesInterface)
    {
    }

    KritaFilterGradientMapConfiguration(const KritaFilterGradientMapConfiguration &rhs)
        : KisFilterConfiguration(rhs)
    {
    }

    virtual KisFilterConfigurationSP clone() const override {
        return new KritaFilterGradientMapConfiguration(*this);
    }

    KoStopGradientSP gradientImpl(KisResourcesInterfaceSP resourcesInterface) const {
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

    KoStopGradientSP gradient() const {
        return gradientImpl(resourcesInterface());
    }

    QList<KoResourceSP> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const override
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

    QList<KoResourceSP> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const override
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
};

using KritaFilterGradientMapConfigurationSP = KisPinnedSharedPtr<KritaFilterGradientMapConfiguration>;

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
                                         const KisFilterConfigurationSP _config,
                                         KoUpdater *progressUpdater) const
{
    Q_ASSERT(!device.isNull());

    const KritaFilterGradientMapConfiguration* config =
        dynamic_cast<const KritaFilterGradientMapConfiguration*>(_config.data());

    KIS_SAFE_ASSERT_RECOVER_RETURN(config);

    KoStopGradientSP gradient = config->gradient();

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
            if (!gradient->stopsAt(leftStop, rightStop, grey)) continue;
            if (std::abs(grey - leftStop.first) < std::abs(grey - rightStop.first)) {
                outColor = leftStop.second.first;
            }
            else {
                outColor = rightStop.second.first;
            }
        }
        else if (colorMode == ColorMode::Dither) {
            KoGradientStop leftStop, rightStop;
            if (!gradient->stopsAt(leftStop, rightStop, grey)) continue;
            qreal localT = (grey - leftStop.first) / (rightStop.first - leftStop.first);
            if (localT < ditherUtil.threshold(QPoint(it.x(), it.y()))) {
                outColor = leftStop.second.first;
            }
            else {
                outColor = rightStop.second.first;
            }
        }
        else {
            gradient->colorAt(outColor, grey);
        }
        outColor.setOpacity(qMin(KoColor(it.oldRawData(), device->colorSpace()).opacityF(), outColor.opacityF()));
        outColor.convertTo(device->colorSpace());
        memcpy(it.rawData(), outColor.data(), pixelSize);
    }

}

KisFilterConfigurationSP KritaFilterGradientMap::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{

    return new KritaFilterGradientMapConfiguration(id().id(), 2, resourcesInterface);
}

KisFilterConfigurationSP KritaFilterGradientMap::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);

    auto source = resourcesInterface->source<KoAbstractGradient>(ResourceType::Gradients);
    KoAbstractGradientSP gradient = source.fallbackResource();

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

KisConfigWidget *KritaFilterGradientMap::createConfigurationWidget(QWidget * parent, const KisPaintDeviceSP dev, bool) const
{
    return new KritaGradientMapConfigWidget(parent, dev);
}

