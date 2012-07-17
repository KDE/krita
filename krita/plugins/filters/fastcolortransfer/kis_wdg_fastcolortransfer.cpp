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

#include "kis_wdg_fastcolortransfer.h"

#include <QLayout>

#include <kurlrequester.h>

#include <KoFilter.h>
#include <KoFilterManager.h>

#include <filter/kis_filter_configuration.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_iterator_ng.h>
#include <kis_paint_device.h>
#include <kundo2command.h>
#include <KoColorSpaceRegistry.h>

#include "ui_wdgfastcolortransfer.h"

KisWdgFastColorTransfer::KisWdgFastColorTransfer(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgFastColorTransfer();
    m_widget->setupUi(this);
    connect(m_widget->fileNameURLRequester, SIGNAL(textChanged(const QString&)), this, SIGNAL(sigConfigurationItemChanged()));
}


KisWdgFastColorTransfer::~KisWdgFastColorTransfer()
{
    delete m_widget;
}

void KisWdgFastColorTransfer::setConfiguration(const KisPropertiesConfiguration* config)
{
    QVariant value;
    if (config->getProperty("filename", value)) {
        widget()->fileNameURLRequester->setUrl(value.toString());
    }

}

KisPropertiesConfiguration* KisWdgFastColorTransfer::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("colortransfer", 1);
    QString fileName = this->widget()->fileNameURLRequester->url().url();
    
    if (fileName.isEmpty()) return config;

    KisPaintDeviceSP ref;

    dbgPlugins << "Use as reference file : " << fileName;
    KisDoc2 d;
    KoFilterManager manager(&d);
    KoFilter::ConversionStatus status;
    QString s = manager.importDocument(fileName, QString(), status);
    dbgPlugins << "import returned" << s << "and status" << status;
    KisImageWSP importedImage = d.image();

    if (importedImage) {
        ref = importedImage->projection();
    }
    if (!ref) {
        dbgPlugins << "No reference image was specified.";
        return config;
    }
    
    // Convert ref to LAB
    const KoColorSpace* labCS = KoColorSpaceRegistry::instance()->lab16();
    if (!labCS) {
        dbgPlugins << "The LAB colorspace is not available.";
        return config;
    }
    
    dbgPlugins << "convert ref to lab";
    KUndo2Command* cmd = ref->convertTo(labCS, KoColorConversionTransformation::IntentPerceptual, KoColorConversionTransformation::BlackpointCompensation);
    delete cmd;
    
    // Compute the means and sigmas of ref
    double meanL_ref = 0., meanA_ref = 0., meanB_ref = 0.;
    double sigmaL_ref = 0., sigmaA_ref = 0., sigmaB_ref = 0.;
    
    KisRectConstIteratorSP refIt = ref->createRectConstIteratorNG(0, 0, importedImage->width(), importedImage->height());
    do {
        const quint16* data = reinterpret_cast<const quint16*>(refIt->oldRawData());

        quint32 L = data[0];
        quint32 A = data[1];
        quint32 B = data[2];

        meanL_ref += L;
        meanA_ref += A;
        meanB_ref += B;

        sigmaL_ref += L * L;
        sigmaA_ref += A * A;
        sigmaB_ref += B * B;

    } while (refIt->nextPixel());

    double totalSize = 1. / (importedImage->width() * importedImage->height());

    meanL_ref *= totalSize;
    meanA_ref *= totalSize;
    meanB_ref *= totalSize;
    sigmaL_ref *= totalSize;
    sigmaA_ref *= totalSize;
    sigmaB_ref *= totalSize;
    
    dbgPlugins << totalSize << "" << meanL_ref << "" << meanA_ref << "" << meanB_ref << "" << sigmaL_ref << "" << sigmaA_ref << "" << sigmaB_ref;

    config->setProperty("filename", fileName);
    config->setProperty("meanL", meanL_ref);
    config->setProperty("meanA", meanA_ref);
    config->setProperty("meanB", meanB_ref);
    config->setProperty("sigmaL", sigmaL_ref);
    config->setProperty("sigmaA", sigmaA_ref);
    config->setProperty("sigmaB", sigmaB_ref);
    
    return config;
}
