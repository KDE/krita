/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#include "psd_loader.h"

#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoCompositeOp.h>

#include <kis_annotation.h>
#include <kis_types.h>
#include <kis_paint_layer.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>

#include "psd.h"
#include "psd_header.h"
#include "psd_colormode_block.h"
#include "psd_utils.h"
#include "psd_resource_section.h"
#include "psd_layer_section.h"
#include "psd_resource_block.h"
#include "psd_image_data.h"

PSDLoader::PSDLoader(KisDoc2 *doc)
{
    m_image = 0;
    m_doc = doc;
    m_job = 0;
    m_stop = false;
}

PSDLoader::~PSDLoader()
{
}

KisImageBuilder_Result PSDLoader::decode(const KUrl& uri)
{
    // open the file
    QFile f(uri.toLocalFile());
    if (!f.exists()) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }
    if (!f.open(QIODevice::ReadOnly)) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << "pos:" << f.pos();

    PSDHeader header;
    if (!header.read(&f)) {
        dbgFile << "failed reading header: " << header.error;
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << header;
    dbgFile << "Read header. pos:" << f.pos();

    PSDColorModeBlock colorModeBlock(header.colormode);
    if (!colorModeBlock.read(&f)) {
        dbgFile << "failed reading colormode block: " << colorModeBlock.error;
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << "Read color mode block. pos:" << f.pos();

    PSDResourceSection resourceSection;
    if (!resourceSection.read(&f)) {
        dbgFile << "failed reading resource section: " << resourceSection.error;
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << "Read resource section. pos:" << f.pos();

    PSDLayerSection layerSection(header);
    if (!layerSection.read(&f)) {
        dbgFile << "failed reading layer section: " << layerSection.error;
        return KisImageBuilder_RESULT_FAILURE;
    }
    // XXX: add all the image resource blocks as annotations to the image

    dbgFile << "Read layer section. " << layerSection.nLayers << "layers. pos:" << f.pos();

    // Get the right colorspace
    QPair<QString, QString> colorSpaceId = psd_colormode_to_colormodelid(header.colormode,
                                                                         header.channelDepth);
    if (colorSpaceId.first.isNull()) return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;

    // Get the icc profile!
    const KoColorProfile* profile = 0;
    if (resourceSection.resources.contains(PSDResourceSection::ICC_PROFILE)) {

        QByteArray profileData = resourceSection.resources[PSDResourceSection::ICC_PROFILE]->data;
        profile = KoColorSpaceRegistry::instance()->createColorProfile(colorSpaceId.first,
                                                                       colorSpaceId.second,
                                                                       profileData);

    }

    // Create the colorspace
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceId.first, colorSpaceId.second, profile);
    if (!cs) {
        if (colorSpaceId.first.contains("LABA") && colorSpaceId.second.contains("U8"))
        {
            qDebug()<<"Krita Has Got LAB with 8 Bit Depth, which does not seem to be in kolospacemath so i upscale to 16 bit";
            cs = KoColorSpaceRegistry::instance()->colorSpace("LABA",  "U16", profile);
        }
        else{
            return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
        }
    }
    // Creating the KisImageWSP
    m_image = new KisImage(m_doc->createUndoStore(),  header.width, header.height, cs, "built image");
    Q_CHECK_PTR(m_image);
    m_image->lock();

    // set the correct resolution
    RESN_INFO_1005 *resInfo = dynamic_cast<RESN_INFO_1005*>(resourceSection.resources[PSDResourceSection::RESN_INFO]->resource);
    if (resInfo) {
        m_image->setResolution(resInfo->hRes, resInfo->vRes);
        // let's skip the unit for now; we can only set that on the KoDocument, and krita doesn't use it.
    }

    // Preserve the duotone colormode block for saving back to psd
    if (header.colormode == DuoTone) {
        KisAnnotationSP annotation = new KisAnnotation("DuotoneColormodeBlock",
                                                       i18n("Duotone Colormode Block"),
                                                       colorModeBlock.data);
        m_image->addAnnotation(annotation);
    }


    // read the projection into our single layer
    if (layerSection.nLayers == 0) {
        dbgFile << "Position" << f.pos() << "Going to read the projection into the first layer, which Photoshop calls 'Background'";

        KisPaintLayerSP layer = new KisPaintLayer(m_image, i18n("Background"), OPACITY_OPAQUE_U8);
        KisTransaction("", layer -> paintDevice());

        PSDImageData imageData(&header);
        imageData.read(layer->paintDevice(), &f);

        //readLayerData(&f, layer->paintDevice(), f.pos(), QRect(0, 0, header.width, header.height));
        m_image->addNode(layer, m_image->rootLayer());

    }
    else {

        // read the channels for the various layers
        for(int i = 0; i < layerSection.nLayers; ++i) {

            // XXX: work out the group layer structure in Photoshop, as well as the adjustment layers

            PSDLayerRecord* layerRecord = layerSection.layers.at(i);
            dbgFile << "Going to read channels for layer " << i << layerRecord->layerName;

            KisPaintLayerSP layer = new KisPaintLayer(m_image, layerRecord->layerName, layerRecord->opacity);
            layer->setCompositeOp(psd_blendmode_to_composite_op(layerRecord->blendModeKey));
            if (!layerRecord->readChannels(&f, layer->paintDevice())) {
                dbgFile << "failed reading channels for layer: " << layerRecord->layerName << layerRecord->error;
                return KisImageBuilder_RESULT_FAILURE;
            }

            m_image->addNode(layer, m_image->rootLayer());
            layer->setVisible(layerRecord->visible);
        }
    }

    m_image->unlock();
    return KisImageBuilder_RESULT_OK;
}


KisImageBuilder_Result PSDLoader::buildImage(const KUrl& uri)
{
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, KIO::NetAccess::SourceSide, qApp->activeWindow())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    // We're not set up to handle asynchronous loading at the moment.
    KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, qApp->activeWindow())) {
        KUrl uriTF;
        uriTF.setPath( tmpFile );
        result = decode(uriTF);
        KIO::NetAccess::removeTempFile(tmpFile);
    }

    return result;
}


KisImageWSP PSDLoader::image()
{
    return m_image;
}

void PSDLoader::cancel()
{
    m_stop = true;
}

#include "psd_loader.moc"

