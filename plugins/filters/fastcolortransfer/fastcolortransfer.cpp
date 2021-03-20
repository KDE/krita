/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "fastcolortransfer.h"

#include <math.h>

#include <kpluginfactory.h>

#include <kundo2command.h>

#include <KoColorSpaceRegistry.h>
#include <KoUpdater.h>

#include <filter/kis_filter_registry.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_fastcolortransfer.h"
#include "ui_wdgfastcolortransfer.h"
#include <KisSequentialIteratorProgress.h>
#include <KoProgressUpdater.h>


K_PLUGIN_FACTORY_WITH_JSON(KritaFastColorTransferFactory, "kritafastcolortransfer.json", registerPlugin<FastColorTransferPlugin>();)


FastColorTransferPlugin::FastColorTransferPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterFastColorTransfer());

}

FastColorTransferPlugin::~FastColorTransferPlugin()
{
}

KisFilterFastColorTransfer::KisFilterFastColorTransfer() : KisFilter(id(), FiltersCategoryColorId, i18n("&Color Transfer..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsThreading(false);
    setSupportsPainting(false);
    setSupportsAdjustmentLayers(false);
}


KisConfigWidget * KisFilterFastColorTransfer::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    return new KisWdgFastColorTransfer(parent);
}

KisFilterConfigurationSP KisFilterFastColorTransfer::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty("filename", "");
    return config;
}

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

void KisFilterFastColorTransfer::processImpl(KisPaintDeviceSP device,
                                             const QRect& applyRect,
                                             const KisFilterConfigurationSP config,
                                             KoUpdater* progressUpdater) const
{
    Q_ASSERT(device != 0);

    dbgPlugins << "Start transferring color";

    // Convert ref and src to LAB
    const KoColorSpace* labCS = KoColorSpaceRegistry::instance()->lab16();
    if (!labCS) {
        dbgPlugins << "The LAB colorspace is not available.";
        return;
    }
    
    dbgPlugins << "convert a copy of src to lab";
    const KoColorSpace* oldCS = device->colorSpace();
    KisPaintDeviceSP srcLAB = new KisPaintDevice(*device.data());
    dbgPlugins << "srcLab : " << srcLAB->extent();
    srcLAB->convertTo(labCS, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());

    KoProgressUpdater compositeUpdater(progressUpdater, KoProgressUpdater::Unthreaded);
    KoUpdater *updaterStats = compositeUpdater.startSubtask(1);
    KoUpdater *updaterMap = compositeUpdater.startSubtask(2);

    // Compute the means and sigmas of src
    dbgPlugins << "Compute the means and sigmas of src";
    double meanL_src = 0., meanA_src = 0., meanB_src = 0.;
    double sigmaL_src = 0., sigmaA_src = 0., sigmaB_src = 0.;

    {
        KisSequentialConstIteratorProgress srcIt(srcLAB, applyRect, updaterStats);
        while (srcIt.nextPixel()) {
            const quint16* data = reinterpret_cast<const quint16*>(srcIt.oldRawData());
            quint32 L = data[0];
            quint32 A = data[1];
            quint32 B = data[2];
            meanL_src += L;
            meanA_src += A;
            meanB_src += B;
            sigmaL_src += L * L;
            sigmaA_src += A * A;
            sigmaB_src += B * B;
        }
    }
    
    double totalSize = 1. / (applyRect.width() * applyRect.height());
    meanL_src *= totalSize;
    meanA_src *= totalSize;
    meanB_src *= totalSize;
    sigmaL_src *= totalSize;
    sigmaA_src *= totalSize;
    sigmaB_src *= totalSize;
    
    dbgPlugins << totalSize << "" << meanL_src << "" << meanA_src << "" << meanB_src << "" << sigmaL_src << "" << sigmaA_src << "" << sigmaB_src;
    
    double meanL_ref = config->getDouble("meanL");
    double meanA_ref = config->getDouble("meanA");
    double meanB_ref = config->getDouble("meanB");
    double sigmaL_ref = config->getDouble("sigmaL");
    double sigmaA_ref = config->getDouble("sigmaA");
    double sigmaB_ref = config->getDouble("sigmaB");
    
    // Transfer colors
    dbgPlugins << "Transfer colors";
    {
        double coefL = sqrt((sigmaL_ref - meanL_ref * meanL_ref) / (sigmaL_src - meanL_src * meanL_src));
        double coefA = sqrt((sigmaA_ref - meanA_ref * meanA_ref) / (sigmaA_src - meanA_src * meanA_src));
        double coefB = sqrt((sigmaB_ref - meanB_ref * meanB_ref) / (sigmaB_src - meanB_src * meanB_src));

        quint16 labPixel[4];

        KisSequentialConstIteratorProgress srcLabIt(srcLAB, applyRect, updaterMap);
        KisSequentialIterator dstIt(device, applyRect);
        while (srcLabIt.nextPixel() && dstIt.nextPixel()) {
            const quint16* data = reinterpret_cast<const quint16*>(srcLabIt.oldRawData());

            labPixel[0] = (quint16)CLAMP(((double)data[0] - meanL_src) * coefL + meanL_ref, 0., 65535.);
            labPixel[1] = (quint16)CLAMP(((double)data[1] - meanA_src) * coefA + meanA_ref, 0., 65535.);
            labPixel[2] = (quint16)CLAMP(((double)data[2] - meanB_src) * coefB + meanB_ref, 0., 65535.);
            labPixel[3] = data[3];
            oldCS->fromLabA16(reinterpret_cast<const quint8*>(labPixel), dstIt.rawData(), 1);
        }
    }
}

#include "fastcolortransfer.moc"
