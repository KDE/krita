#include "kis_edge_detection_filter.h"
#include <kis_edge_detection_kernel.h>
#include <kis_convolution_kernel.h>
#include <kis_convolution_painter.h>

#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include "kis_lod_transform.h"

KisEdgeDetectionFilter::KisEdgeDetectionFilter(): KisFilter(id(), categoryEdgeDetection(), i18n("&Edge Detection..."))
{
    setSupportsPainting(true);
    setSupportsAdjustmentLayers(true);
    setSupportsLevelOfDetail(true);
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setShowConfigurationWidget(false);
}

void KisEdgeDetectionFilter::processImpl(KisPaintDeviceSP device, const QRect &rect, const KisFilterConfigurationSP config, KoUpdater *progressUpdater) const
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
        channelFlags = QBitArray(device->colorSpace()->channelCount(), true);
    }

    KisEdgeDetectionKernel::FilterType type = KisEdgeDetectionKernel::Prewit;

    KisEdgeDetectionKernel::applyEdgeDetection(device, rect,
                                     horizontalRadius, verticalRadius,
                                               type,
                                     channelFlags, progressUpdater);
}

KisFilterConfigurationSP KisEdgeDetectionFilter::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("horizRadius", 5);
    config->setProperty("vertRadius", 5);
    config->setProperty("type", "prewit");
    config->setProperty("lockAspect", true);

    return config;
}

QRect KisEdgeDetectionFilter::neededRect(const QRect &rect, const KisFilterConfigurationSP _config, int lod) const
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

QRect KisEdgeDetectionFilter::changedRect(const QRect &rect, const KisFilterConfigurationSP _config, int lod) const
{
    KisLodTransformScalar t(lod);

    QVariant value;

    const int halfWidth = _config->getProperty("horizRadius", value) ? KisEdgeDetectionKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;
    const int halfHeight = _config->getProperty("vertRadius", value) ? KisEdgeDetectionKernel::kernelSizeFromRadius(t.scale(value.toFloat())) / 2 : 5;

    return rect.adjusted( -halfWidth, -halfHeight, halfWidth, halfHeight);
}
