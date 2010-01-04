/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "exr_converter.h"

#include <half.h>

#include <ImfChannelList.h>
#include <ImfInputFile.h>

#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>

#include <kis_doc2.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>
#include <kis_undo_adapter.h>

exrConverter::exrConverter(KisDoc2 *doc, KisUndoAdapter *adapter)
{
    m_doc = doc;
    m_adapter = adapter;
    m_job = 0;
    m_stop = false;
}

exrConverter::~exrConverter()
{
}

enum ImageType {
    IT_UNKNOWN,
    IT_FLOAT16,
    IT_FLOAT32,
    IT_UNSUPPORTED
};

ImageType imfTypeToKisType( Imf::PixelType type)
{
    switch(type)
    {
        case Imf::UINT:
        case Imf::NUM_PIXELTYPES:
            return IT_UNSUPPORTED;
        case Imf::HALF:
            return IT_FLOAT16;
        case Imf::FLOAT:
            return IT_FLOAT32;
    }
}

KisImageBuilder_Result exrConverter::decode(const KUrl& uri)
{
    dbgFile << "Load exr: " << uri << " " << QFile::encodeName(uri.toLocalFile());
    Imf::InputFile file(QFile::encodeName(uri.toLocalFile()));

    Imath::Box2i dw = file.header().dataWindow();
    int width = dw.max.x - dw.min.x + 1;
    int height = dw.max.y - dw.min.y + 1;
    int dx = dw.min.x;
    int dy = dw.min.y;

    // For debug purpose print the content of the file
    const Imf::ChannelList &channels = file.header().channels();
    std::set<std::string> layerNames;
    channels.layers(layerNames);

    if (layerNames.empty()) {
        dbgFile << "Single layer:";
        for (Imf::ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i) {
            const Imf::Channel &channel = i.channel();
            dbgFile << "Channel name = " << i.name() << " type = " << channel.type;
        }
    } else {
        dbgFile << "Multi layers:";
        for (std::set<std::string>::const_iterator i = layerNames.begin();
                i != layerNames.end(); ++i) {
            dbgFile << "layer name = " << i->c_str();
            Imf::ChannelList::ConstIterator layerBegin, layerEnd;
            channels.channelsInLayer(*i, layerBegin, layerEnd);
            for (Imf::ChannelList::ConstIterator j = layerBegin;
                    j != layerEnd; ++j) {
                dbgFile << "\tchannel " << j.name();
            }
        }
        qFatal("Unimplemented");
    }
    
    // Check image type
    ImageType imageType = IT_UNKNOWN;
    for (Imf::ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i) {
        const Imf::Channel &channel = i.channel();
        ImageType channelType = imfTypeToKisType(channel.type);
        
        if( imageType == IT_UNKNOWN )
        {
            imageType = channelType;
        } else if( imageType != channelType )
        {
            imageType = IT_UNSUPPORTED;
        }
    }
    
    const KoColorSpace* colorSpace = 0;
    switch(imageType)
    {
        case IT_FLOAT16:
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace(KoID("RgbAF16", ""), "");
            break;
        case IT_FLOAT32:
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace(KoID("RgbAF32", ""), "");
            break;
        case IT_UNKNOWN:
        case IT_UNSUPPORTED:
            break;
    }
    if( !colorSpace) return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    dbgFile << "Colorspace: " << colorSpace->name();
    
    // Create the image
    m_image = new KisImage( m_adapter, width, height, colorSpace, "");

    if (!m_image) {
        return KisImageBuilder_RESULT_FAILURE;
    }
    m_image->lock();
    
    // Create the layer
    KisPaintLayerSP layer = new KisPaintLayer(m_image, m_image->nextLayerName(), OPACITY_OPAQUE, colorSpace);
    KisTransaction("", layer->paintDevice());

    layer->setCompositeOp(COMPOSITE_OVER);

    if (!layer) {
        return KisImageBuilder_RESULT_FAILURE;
    }

    m_image->addNode(layer, m_image->rootLayer());
    layer->setDirty();
    m_image->unlock();
    return KisImageBuilder_RESULT_OK;
}



KisImageBuilder_Result exrConverter::buildImage(const KUrl& uri)
{
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, false, QApplication::activeWindow())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    // We're not set up to handle asynchronous loading at the moment.
    KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, qApp->activeWindow())) {
        KUrl uriTF;
        uriTF.setPath(tmpFile);
        result = decode(uriTF);
        KIO::NetAccess::removeTempFile(tmpFile);
    }

    return result;
}


KisImageWSP exrConverter::image()
{
    return m_image;
}


KisImageBuilder_Result exrConverter::buildFile(const KUrl& uri, KisPaintLayerSP layer)
{
    if (!layer)
        return KisImageBuilder_RESULT_INVALID_ARG;

    KisImageWSP image = layer->image();
    if (!image)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;
    // Open file for writing
#if 0
    FILE *fp = fopen(QFile::encodeName(uri.path()), "wb");
    if (!fp) {
        return (KisImageBuilder_RESULT_FAILURE);
    }
    uint height = image->height();
    uint width = image->width();
#endif

    return KisImageBuilder_RESULT_OK;
}


void exrConverter::cancel()
{
    m_stop = true;
}

#include "exr_converter.moc"

