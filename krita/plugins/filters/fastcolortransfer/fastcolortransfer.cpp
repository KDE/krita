/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "fastcolortransfer.h"
#include <kpluginfactory.h>
#include <kurlrequester.h>

#include <QUndoCommand>

#include <KoColorSpaceRegistry.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <kis_doc2.h>
#include <filter/kis_filter_registry.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_fastcolortransfer.h"
#include "ui_wdgfastcolortransfer.h"

K_PLUGIN_FACTORY(KritaFastColorTransferFactory, registerPlugin<FastColorTransferPlugin>();)
K_EXPORT_PLUGIN(KritaFastColorTransferFactory("krita"))


FastColorTransferPlugin::FastColorTransferPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //setComponentData(KritaFastColorTransferFactory::componentData());
    KisFilterRegistry::instance()->add(new KisFilterFastColorTransfer());

}

FastColorTransferPlugin::~FastColorTransferPlugin()
{
}

KisFilterFastColorTransfer::KisFilterFastColorTransfer() : KisFilter(id(), categoryColors(), i18n("&Color Transfer..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
    setSupportsAdjustmentLayers(false);
}


KisConfigWidget * KisFilterFastColorTransfer::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, const KisImageWSP image) const
{
    Q_UNUSED(dev);
    Q_UNUSED(image);
    return new KisWdgFastColorTransfer(parent);
}

KisFilterConfiguration* KisFilterFastColorTransfer::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("filename", "");  // TODO: put an exemple image in share/krita, like a sunset that what's give the best results
    return config;
}

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

void KisFilterFastColorTransfer::process(KisConstProcessingInformation srcInfo,
        KisProcessingInformation dstInfo,
        const QSize& size,
        const KisFilterConfiguration* config,
        KoUpdater* progressUpdater
                                        ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    dbgPlugins << "Start transferring color";
    QVariant value;
    QString fileName;
    if (config && config->getProperty("filename", value)) {
        fileName = value.toString();
    } else {
        // XXX: Make this a warning message box!
        dbgPlugins << "No file name for the reference image was specified.";
        return;
    }

    KisPaintDeviceSP ref;

    dbgPlugins << "Use as reference file : " << fileName;
    KisDoc2 d;
    d.importDocument(fileName);
    KisImageWSP importedImage = d.image();

    if (importedImage) {
        ref = importedImage->projection();
    }
    if (!ref) {
        dbgPlugins << "No reference image was specified.";
        return;
    }

    // Convert ref and src to LAB
    const KoColorSpace* labCS = KoColorSpaceRegistry::instance()->lab16();
    if (!labCS) {
        dbgPlugins << "The LAB colorspace is not available.";
        return;
    }
    dbgPlugins << "convert a copy of src to lab";
    const KoColorSpace* oldCS = src->colorSpace();
    KisPaintDeviceSP srcLAB = new KisPaintDevice(*src.data());
    dbgPlugins << "srcLab : " << srcLAB->extent();
    QUndoCommand* cmd = srcLAB->convertTo(labCS);
    delete cmd;
    dbgPlugins << "convert ref to lab";
    dbgPlugins << "ref : " << srcLAB->extent();
    cmd = ref->convertTo(labCS);
    delete cmd;

    if (progressUpdater) {
        progressUpdater->setRange(0, 2*size.width() * size.height() + importedImage->width() * importedImage->height());
    }
    int count = 0;

    // Compute the means and sigmas of src
    dbgPlugins << "Compute the means and sigmas of src";
    double meanL_src = 0., meanA_src = 0., meanB_src = 0.;
    double sigmaL_src = 0., sigmaA_src = 0., sigmaB_src = 0.;
    KisRectConstIteratorPixel srcLABIt = srcLAB->createRectConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height(), srcInfo.selection());
    while (!srcLABIt.isDone() && !(progressUpdater && progressUpdater->interrupted())) {
        const quint16* data = reinterpret_cast<const quint16*>(srcLABIt.oldRawData());
        quint32 L = data[0];
        quint32 A = data[1];
        quint32 B = data[2];
        meanL_src += L;
        meanA_src += A;
        meanB_src += B;
        sigmaL_src += L * L;
        sigmaA_src += A * A;
        sigmaB_src += B * B;
        ++srcLABIt;
        if (progressUpdater) progressUpdater->setValue(++count);
    }
    double totalSize = 1. / (size.width() * size.height());
    meanL_src *= totalSize;
    meanA_src *= totalSize;
    meanB_src *= totalSize;
    sigmaL_src *= totalSize;
    sigmaA_src *= totalSize;
    sigmaB_src *= totalSize;
    dbgPlugins << totalSize << "" << meanL_src << "" << meanA_src << "" << meanB_src << "" << sigmaL_src << "" << sigmaA_src << "" << sigmaB_src;
    // Compute the means and sigmas of ref
    dbgPlugins << "Compute the means and sigmas of ref";
    double meanL_ref = 0., meanA_ref = 0., meanB_ref = 0.;
    double sigmaL_ref = 0., sigmaA_ref = 0., sigmaB_ref = 0.;
    KisRectConstIteratorPixel refIt = ref->createRectConstIterator(0, 0, importedImage->width(), importedImage->height());
    while (!refIt.isDone() && !(progressUpdater && progressUpdater->interrupted())) {
        const quint16* data = reinterpret_cast<const quint16*>(refIt.oldRawData());
        quint32 L = data[0];
        quint32 A = data[1];
        quint32 B = data[2];
        meanL_ref += L;
        meanA_ref += A;
        meanB_ref += B;
        sigmaL_ref += L * L;
        sigmaA_ref += A * A;
        sigmaB_ref += B * B;
        ++refIt;
        if (progressUpdater) progressUpdater->setValue(++count);
    }
    totalSize = 1. / (importedImage->width() * importedImage->height());
    meanL_ref *= totalSize;
    meanA_ref *= totalSize;
    meanB_ref *= totalSize;
    sigmaL_ref *= totalSize;
    sigmaA_ref *= totalSize;
    sigmaB_ref *= totalSize;
    dbgPlugins << totalSize << "" << meanL_ref << "" << meanA_ref << "" << meanB_ref << "" << sigmaL_ref << "" << sigmaA_ref << "" << sigmaB_ref;

    // Transfer colors
    dbgPlugins << "Transfer colors";
    {
        double coefL = sqrt((sigmaL_ref - meanL_ref * meanL_ref) / (sigmaL_src - meanL_src * meanL_src));
        double coefA = sqrt((sigmaA_ref - meanA_ref * meanA_ref) / (sigmaA_src - meanA_src * meanA_src));
        double coefB = sqrt((sigmaB_ref - meanB_ref * meanB_ref) / (sigmaB_src - meanB_src * meanB_src));
        KisHLineConstIteratorPixel srcLABIt = srcLAB->createHLineConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), srcInfo.selection());
        KisHLineIteratorPixel dstIt = dst->createHLineIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), dstInfo.selection());
        quint16 labPixel[4];
        for (int y = 0; y < size.height() && !(progressUpdater && progressUpdater->interrupted()); ++y) {
            while (!dstIt.isDone()) {
                const quint16* data = reinterpret_cast<const quint16*>(srcLABIt.oldRawData());
                labPixel[0] = (quint16)CLAMP(((double)data[0] - meanL_src) * coefL + meanL_ref, 0., 65535.);
                labPixel[1] = (quint16)CLAMP(((double)data[1] - meanA_src) * coefA + meanA_ref, 0., 65535.);
                labPixel[2] = (quint16)CLAMP(((double)data[2] - meanB_src) * coefB + meanB_ref, 0., 65535.);
                labPixel[3] = data[3];
                oldCS->fromLabA16(reinterpret_cast<const quint8*>(labPixel), dstIt.rawData(), 1);
                ++dstIt;
                ++srcLABIt;
                if (progressUpdater) progressUpdater->setValue(++count);
            }
            dstIt.nextRow();
            srcLABIt.nextRow();
        }

    }
}
