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

#include <kis_annotation.h>
#include <kis_types.h>
#include <kis_paint_layer.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_undo_adapter.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>

#include "psd_header.h"
#include "psd_colormode_block.h"
#include "psd_utils.h"
#include "psd_resource_section.h"
#include "psd_layer_section.h"

QString psd_colormode_to_colormodelid(PSDHeader::PSDColorMode colormode, quint16 channelDepth)
{
    KoID colorSpaceId;
    switch(colormode) {
    case(PSDHeader::Bitmap):
    case(PSDHeader::Indexed):
    case(PSDHeader::MultiChannel):
    case(PSDHeader::RGB):
        colorSpaceId = RGBAColorModelID;
        break;
    case(PSDHeader::CMYK):
        colorSpaceId = CMYKAColorModelID;
        break;
    case(PSDHeader::Grayscale):
    case(PSDHeader::DuoTone):
        colorSpaceId = GrayAColorModelID;
        break;
    case(PSDHeader::Lab):
        colorSpaceId = LABAColorModelID;
        break;
    default:
        return QString::null;
    }

    switch(channelDepth) {
    case(1):
    case(16):
        return KoColorSpaceRegistry::instance()->colorSpaceId(colorSpaceId, Integer16BitsColorDepthID);
    case(8):
    default:
        return KoColorSpaceRegistry::instance()->colorSpaceId(colorSpaceId, Integer8BitsColorDepthID);

    }

    return QString::null;
}

PSDLoader::PSDLoader(KisDoc2 *doc, KisUndoAdapter *adapter)
{
    m_img = 0;
    m_doc = doc;
    m_adapter = adapter;
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
        kDebug() << "failed reading header: " << header.error;
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << header;
    dbgFile << "Read header. pos:" << f.pos();

    PSDColorModeBlock colorModeBlock(header.m_colormode);
    if (!colorModeBlock.read(&f)) {
        kDebug() << "failed reading colormode block: " << colorModeBlock.error;
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << "Read color mode block. pos:" << f.pos();

    PSDResourceSection resourceSection;
    if (!resourceSection.read(&f)) {
        kDebug() << "failed reading resource section: " << resourceSection.error;
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << "Read resource section. pos:" << f.pos();

    PSDLayerSection layerSection(header);
    if (!layerSection.read(&f)) {
        kDebug() << "failed reading layer section: " << layerSection.error;
        return KisImageBuilder_RESULT_FAILURE;
    }

    dbgFile << "Read layer section. " << layerSection.layerInfo.nLayers << "layers. pos:" << f.pos();

    // Get the right colorspace
    QString colorSpaceId = psd_colormode_to_colormodelid(header.m_colormode, header.m_channelDepth);
    if (colorSpaceId.isNull()) return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;

    // XXX: get the icc profile!
    KoColorProfile* profile = 0;
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceId, profile);
    if (!cs) return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;

    // Creating the KisImageWSP
    m_img = new KisImage(m_doc->undoAdapter(),  header.m_width, header.m_height, cs, "built image");
    Q_CHECK_PTR(m_img);
    m_img->lock();

    // Preserve the duotone colormode block for saving back to psd
    if (header.m_colormode == PSDHeader::DuoTone) {
        KisAnnotationSP annotation = new KisAnnotation("Duotone Colormode Block",
                                                       i18n("Duotone Colormode Block"),
                                                       colorModeBlock.m_data);
        m_img->addAnnotation(annotation);
    }

    // read the projection into our single layer
    if (layerSection.layerInfo.nLayers == 0) {
        dbgFile << "Position" << f.pos() << "Going to read the projection into the first layer, which Photoshop calls 'Background'";
        KisPaintLayerSP layer = new KisPaintLayer(m_img, i18n("Background"), OPACITY_OPAQUE);
        //readLayerData(&f, layer->paintDevice(), f.pos(), QRect(0, 0, header.m_width, header.m_height));
        m_img->addNode(layer, m_img->rootLayer());
    }
    else {
        // read the channels for the various layers
    }

    return KisImageBuilder_RESULT_OK;
}


KisImageBuilder_Result PSDLoader::buildImage(const KUrl& uri)
{
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, true, qApp->activeWindow())) {
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
    return m_img;
}

void PSDLoader::cancel()
{
    m_stop = true;
}

#include "psd_loader.moc"

