/*
 * Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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
#include "kis_convert_height_to_normal_map_filter.h"
#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter_configuration.h>
#include "kis_lod_transform.h"
#include <kis_edge_detection_kernel.h>


K_PLUGIN_FACTORY_WITH_JSON(KritaConvertHeightToNormalMapFilterFactory, "kritaconvertheighttonormalmap.json", registerPlugin<KritaConvertHeightToNormalMapFilter>();)

KritaConvertHeightToNormalMapFilter::KritaConvertHeightToNormalMapFilter(QObject *parent, const QVariantList &)
: QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KisConvertHeightToNormalMapFilter()));
}

KritaConvertHeightToNormalMapFilter::~KritaConvertHeightToNormalMapFilter()
{
}

KisConvertHeightToNormalMapFilter::KisConvertHeightToNormalMapFilter(): KisFilter(id(), categoryEdgeDetection(), i18n("&Height to Normal Map..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setShowConfigurationWidget(false);//for now...
}

void KisConvertHeightToNormalMapFilter::processImpl(KisPaintDeviceSP device, const QRect &rect, const KisFilterConfigurationSP config, KoUpdater *progressUpdater) const
{
    Q_ASSERT(device != 0);

    KisFilterConfigurationSP configuration = config ? config : new KisFilterConfiguration(id().id(), 1);

    KisLodTransformScalar t(device);

    QVariant value;
    configuration->getProperty("horizRadius", value);
    float horizontalRadius = t.scale(value.toFloat());
    configuration->getProperty("vertRadius", value);
    float verticalRadius = t.scale(value.toFloat());

    QBitArray channelFlags;
    if (configuration) {
        channelFlags = configuration->channelFlags();
    }
    if (channelFlags.isEmpty() || !configuration) {
        channelFlags = device->colorSpace()->channelFlags();
    }

    KisEdgeDetectionKernel::FilterType type = KisEdgeDetectionKernel::SobolVector;
    if (configuration->getString("type") == "prewitt") {
        type = KisEdgeDetectionKernel::Prewit;
    } else if (configuration->getString("type") == "simple") {
        type = KisEdgeDetectionKernel::Simple;
    }

    int channelToConvert = configuration->getInt("channelToConvert", 0);

    QVector<int> channelOrder(3);
    channelOrder[0] = 0;
    channelOrder[1] = 1;
    channelOrder[2] = 2;
    QVector<bool> channelFlip(3);
    channelFlip[0] = false;
    channelFlip[1] = true;
    channelFlip[2] = false;
    KisEdgeDetectionKernel::convertToNormalMap(device,
                                              rect,
                                              horizontalRadius,
                                              verticalRadius,
                                              type,
                                              channelToConvert,
                                              channelOrder,
                                              channelFlip,
                                              channelFlags,
                                              progressUpdater);
}

KisFilterConfigurationSP KisConvertHeightToNormalMapFilter::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("horizRadius", 1);
    config->setProperty("vertRadius", 1);
    config->setProperty("type", "prewitt");
    config->setProperty("channelToConvert", 0);
    config->setProperty("lockAspect", true);

    return config;
}

QRect KisConvertHeightToNormalMapFilter::neededRect(const QRect &rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;
    /**
     * NOTE: integer devision by two is done on purpose,
     *       because the kernel size is always odd
     */
    const int halfWidth = _config->getProperty("horizRadius", value) ? KisEdgeDetectionKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;
    const int halfHeight = _config->getProperty("vertRadius", value) ? KisEdgeDetectionKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;

    return rect.adjusted(-halfWidth * 2, -halfHeight * 2, halfWidth * 2, halfHeight * 2);
}

QRect KisConvertHeightToNormalMapFilter::changedRect(const QRect &rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;

    const int halfWidth = _config->getProperty("horizRadius", value) ? KisEdgeDetectionKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;
    const int halfHeight = _config->getProperty("vertRadius", value) ? KisEdgeDetectionKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;

    return rect.adjusted( -halfWidth, -halfHeight, halfWidth, halfHeight);
}

#include "kis_convert_height_to_normal_map_filter.moc"
