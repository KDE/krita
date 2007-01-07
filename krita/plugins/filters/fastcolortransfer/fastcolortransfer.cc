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

#include <kgenericfactory.h>
#include <kurlrequester.h>

#include <KoColorSpaceRegistry.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_meta_registry.h>
#include <kis_paint_device.h>

#include "kis_wdg_fastcolortransfer.h"
#include "ui_wdgfastcolortransfer.h"

typedef KGenericFactory<FastColorTransferPlugin> KritaFastColorTransferFactory;
K_EXPORT_COMPONENT_FACTORY( kritafastcolortransfer, KritaFastColorTransferFactory( "krita" ) )


FastColorTransferPlugin::FastColorTransferPlugin(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setInstance(KritaFastColorTransferFactory::instance());

    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new KisFilterFastColorTransfer()));
    }
}

FastColorTransferPlugin::~FastColorTransferPlugin()
{
}

KisFilterFastColorTransfer::KisFilterFastColorTransfer() : KisFilter(id(), "colors", i18n("&Color Transfer..."))
{
}


KisFilterConfigWidget * KisFilterFastColorTransfer::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP )
{
    return new KisWdgFastColorTransfer(this, parent);
}

KisFilterConfiguration* KisFilterFastColorTransfer::designerConfiguration(KisPaintDeviceSP)
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("filename", "" ); // TODO: put an exemple image in share/krita, like a sunset that what's give the best results
    return config;
}

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

void KisFilterFastColorTransfer::process(const KisPaintDeviceSP src, const QPoint& srcTopLeft, KisPaintDeviceSP dst, const QPoint& dstTopLeft, const QSize& size, KisFilterConfiguration* config)
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);

    kDebug() << "Start transferring color" << endl;
    QVariant value;
    QString fileName;
    if (config && config->getProperty("filename", value))
    {
        fileName = value.toString();
    } else {
        kDebug() << "No file name for the reference image was specified." << endl;
        return;
    }
    
    KisPaintDeviceSP ref;
    
    KisDoc2 d;
    d.import(fileName);
    KisImageSP importedImage = d.currentImage();
    
    if(importedImage)
    {
        ref = importedImage->projection();
    }
    if(!ref)
    {
        kDebug() << "No reference image was specified." << endl;
        return;
    }
    
    // Convert ref and src to LAB
    KoColorSpace* labCS = KoColorSpaceRegistry::instance()->colorSpace(KoID("LABA"),"");
    if(!labCS)
    {
        kDebug() << "The LAB colorspace is not available." << endl;
        return;
    }
    KoColorSpace* oldCS = src->colorSpace();
    KisPaintDeviceSP srcLAB = KisPaintDeviceSP(new KisPaintDevice(*src.data()));
    srcLAB->convertTo(labCS);
    ref->convertTo(labCS);
    
    // Compute the means and sigmas of src
    double meanL_src = 0., meanA_src = 0., meanB_src = 0.;
    double sigmaL_src = 0., sigmaA_src = 0., sigmaB_src = 0.;
    KisRectConstIteratorPixel srcLABIt = srcLAB->createRectConstIterator(srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height());
    while(!srcLABIt.isDone())
    {
        const Q_UINT16* data = reinterpret_cast<const Q_UINT16*>(srcLABIt.oldRawData());
        Q_UINT32 L = data[0];
        Q_UINT32 A = data[1];
        Q_UINT32 B = data[2];
        meanL_src += L;
        meanA_src += A;
        meanB_src += B;
        sigmaL_src += L*L;
        sigmaA_src += A*A;
        sigmaB_src += B*B;
        ++srcLABIt;
    }
    double totalSize = 1. / ( size.width() * size.height() );
    meanL_src *= totalSize;
    meanA_src *= totalSize;
    meanB_src *= totalSize;
    sigmaL_src *= totalSize;
    sigmaA_src *= totalSize;
    sigmaB_src *= totalSize;
    kDebug() << totalSize << " " << meanL_src << " " << meanA_src << " " << meanB_src << " " << sigmaL_src << " " << sigmaA_src << " " << sigmaB_src << endl;
    // Compute the means and sigmas of src
    double meanL_ref = 0., meanA_ref = 0., meanB_ref = 0.;
    double sigmaL_ref = 0., sigmaA_ref = 0., sigmaB_ref = 0.;
    KisRectConstIteratorPixel refIt = ref->createRectConstIterator(0, 0, importedImage->width(), importedImage->height());
    while(!refIt.isDone())
    {
        const Q_UINT16* data = reinterpret_cast<const Q_UINT16*>(refIt.oldRawData());
        Q_UINT32 L = data[0];
        Q_UINT32 A = data[1];
        Q_UINT32 B = data[2];
        meanL_ref += L;
        meanA_ref += A;
        meanB_ref += B;
        sigmaL_ref += L*L;
        sigmaA_ref += A*A;
        sigmaB_ref += B*B;
        ++refIt;
    }
    totalSize = 1. / ( importedImage->width() * importedImage->height() );
    meanL_ref *= totalSize;
    meanA_ref *= totalSize;
    meanB_ref *= totalSize;
    sigmaL_ref *= totalSize;
    sigmaA_ref *= totalSize;
    sigmaB_ref *= totalSize;
    kDebug() << totalSize << " " << meanL_ref << " " << meanA_ref << " " << meanB_ref << " " << sigmaL_ref << " " << sigmaA_ref << " " << sigmaB_ref << endl;
    
    // Transfer colors
    dst->convertTo(labCS); // FIXME: DON'T CONVERT to LAB !
    {
        double coefL = sqrt((sigmaL_ref - meanL_ref * meanL_ref) / (sigmaL_src - meanL_src * meanL_src));
        double coefA = sqrt((sigmaA_ref - meanA_ref * meanA_ref) / (sigmaA_src - meanA_src * meanA_src));
        double coefB = sqrt((sigmaB_ref - meanB_ref * meanB_ref) / (sigmaB_src - meanB_src * meanB_src));
        kDebug() << coefL << " " << coefA << " " << coefB << endl;
        KisRectIteratorPixel dstIt = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height());
        while(!dstIt.isDone())
        {
            Q_UINT16* data = reinterpret_cast<Q_UINT16*>(dstIt.rawData());
            data[0] = (Q_UINT16)CLAMP( ( (double)data[0] - meanL_src) * coefL + meanL_ref, 0., 65535.);
            data[1] = (Q_UINT16)CLAMP( ( (double)data[1] - meanA_src) * coefA + meanA_ref, 0., 65535.);
            data[2] = (Q_UINT16)CLAMP( ( (double)data[2] - meanB_src) * coefB + meanB_ref, 0., 65535.);
            ++dstIt;
        }
    }
    dst->convertTo(oldCS);
    setProgressDone(); // Must be called even if you don't really support progression
}
