/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
#include <KoColorSpaceTraits.h>

#include <kis_doc2.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>
#include <kis_undo_adapter.h>
#include <boost/concept_check.hpp>

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

ImageType imfTypeToKisType(Imf::PixelType type)
{
    switch (type) {
    case Imf::UINT:
    case Imf::NUM_PIXELTYPES:
        return IT_UNSUPPORTED;
    case Imf::HALF:
        return IT_FLOAT16;
    case Imf::FLOAT:
        return IT_FLOAT32;
    }
}

const KoColorSpace* kisTypeToColorSpace(ImageType imageType)
{
    switch (imageType) {
    case IT_FLOAT16:
        return KoColorSpaceRegistry::instance()->colorSpace(KoID("RgbAF16", ""), "");
    case IT_FLOAT32:
        return KoColorSpaceRegistry::instance()->colorSpace(KoID("RgbAF32", ""), "");
    case IT_UNKNOWN:
    case IT_UNSUPPORTED:
        return 0;
    }
}

template<typename _T_>
struct Rgba {
    _T_ r;
    _T_ g;
    _T_ b;
    _T_ a;
};

struct ExrLayerInfo {
    ExrLayerInfo() : colorSpace(0), imageType(IT_UNKNOWN) {
    }
    const KoColorSpace* colorSpace;
    ImageType imageType;
    QString name;
    QMap< QString, QString> channelMap; ///< first is either R, G, B or A second is the EXR channel name
    void updateImageType(ImageType channelType);
};

void ExrLayerInfo::updateImageType(ImageType channelType)
{
    if (imageType == IT_UNKNOWN) {
        imageType = channelType;
    } else if (imageType != channelType) {
        imageType = IT_UNSUPPORTED;
    }
}

template<typename _T_>
void decodeData(Imf::InputFile& file, ExrLayerInfo& info, KisPaintLayerSP layer, int width, int xstart, int ystart, int height, Imf::PixelType ptype)
{
    typedef Rgba<_T_> Rgba;

    QVector<Rgba> pixels(width*height);

    bool hasAlpha = info.channelMap.contains("A");

    for (int y = 0; y < height; ++y) {
        Imf::FrameBuffer frameBuffer;
        Rgba* frameBufferData = (pixels.data()) - xstart - (ystart + y) * width;
        frameBuffer.insert(info.channelMap["R"].toAscii().data(),
                           Imf::Slice(ptype, (char *) &frameBufferData->r,
                                      sizeof(Rgba) * 1,
                                      sizeof(Rgba) * width));
        frameBuffer.insert(info.channelMap["G"].toAscii().data(),
                           Imf::Slice(ptype, (char *) &frameBufferData->g,
                                      sizeof(Rgba) * 1,
                                      sizeof(Rgba) * width));
        frameBuffer.insert(info.channelMap["B"].toAscii().data(),
                           Imf::Slice(ptype, (char *) &frameBufferData->b,
                                      sizeof(Rgba) * 1,
                                      sizeof(Rgba) * width));
        if (hasAlpha) {
            frameBuffer.insert(info.channelMap["A"].toAscii().data(),
                               Imf::Slice(ptype, (char *) &frameBufferData->a,
                                          sizeof(Rgba) * 1,
                                          sizeof(Rgba) * width));
        }

        file.setFrameBuffer(frameBuffer);
        file.readPixels(ystart + y);
        Rgba *rgba = pixels.data();
        KisHLineIterator it = layer->paintDevice()->createHLineIterator(0, y, width);
        while (!it.isDone()) {

            // XXX: For now unmultiply the alpha, though compositing will be faster if we
            // keep it premultiplied.
            _T_ unmultipliedRed = rgba -> r;
            _T_ unmultipliedGreen = rgba -> g;
            _T_ unmultipliedBlue = rgba -> b;

            if (rgba -> a >= HALF_EPSILON && hasAlpha) {
                unmultipliedRed /= rgba -> a;
                unmultipliedGreen /= rgba -> a;
                unmultipliedBlue /= rgba -> a;
            }
            typename KoRgbTraits<_T_>::Pixel* dst = reinterpret_cast<typename KoRgbTraits<_T_>::Pixel*>(it.rawData());

            dst->red = unmultipliedRed;
            dst->green = unmultipliedGreen;
            dst->blue = unmultipliedBlue;
            if (hasAlpha) {
                dst->alpha = rgba->a;
            } else {
                dst->alpha = 1.0;
            }

            ++it;
            ++rgba;
        }
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

    // Constuct the list of LayerInfo

    QList<ExrLayerInfo> infos;
    ImageType imageType = IT_UNKNOWN;

    const Imf::ChannelList &channels = file.header().channels();
    std::set<std::string> layerNames;
    channels.layers(layerNames);

    // Test if it is a multilayer EXR or singlelayer
    if (layerNames.empty()) {
        dbgFile << "Single layer:";
        ExrLayerInfo info;
        info.name = i18n("HDR Layer");
        for (Imf::ChannelList::ConstIterator i = channels.begin(); i != channels.end(); ++i) {
            const Imf::Channel &channel = i.channel();
            dbgFile << "Channel name = " << i.name() << " type = " << channel.type;

            info.updateImageType(imfTypeToKisType(channel.type));

            QString qname = i.name();
            if (qname != "A" && qname != "R" && qname != "G" && qname != "B") {
                dbgFile << "Unknow: " << i.name();
                info.imageType = IT_UNSUPPORTED;
            } else {
                info.channelMap[qname] = qname;
            }
        }
        infos.push_back(info);
        imageType = info.imageType;
    } else {
        dbgFile << "Multi layers:";
        for (std::set<std::string>::const_iterator i = layerNames.begin();
                i != layerNames.end(); ++i) {
            ExrLayerInfo info;
            dbgFile << "layer name = " << i->c_str();
            Imf::ChannelList::ConstIterator layerBegin, layerEnd;
            channels.channelsInLayer(*i, layerBegin, layerEnd);
            for (Imf::ChannelList::ConstIterator j = layerBegin;
                    j != layerEnd; ++j) {
                const Imf::Channel &channel = j.channel();
                dbgFile << "\tchannel " << j.name() << " type = " << channel.type;

                info.updateImageType(imfTypeToKisType(channel.type));

                QString qname = j.name();
                QStringList list = qname.split(".");
                QString layersuffix = list.last();
                if (layersuffix != "A" && layersuffix != "R" && layersuffix != "G" && layersuffix != "B") {
                    dbgFile << "Unknow: " << layersuffix;
                    info.imageType = IT_UNSUPPORTED;
                } else {
                    info.channelMap[layersuffix] = qname;
                }
            }
            if (info.imageType != IT_UNKNOWN && info.imageType != IT_UNSUPPORTED) {
                infos.push_back(info);
                if (imageType < info.imageType) {
                    imageType = info.imageType;
                }
            }
        }
    }
    dbgFile << "File has " << infos.size() << " layers";
    // Set the colorspaces
    for (int i = 0; i < infos.size(); ++i) {
        ExrLayerInfo& info = infos[i];
        info.colorSpace = kisTypeToColorSpace(info.imageType);
    }
    // Get colorspace
    dbgFile << "Image type = " << imageType;
    const KoColorSpace* colorSpace = kisTypeToColorSpace(imageType);

    if (!colorSpace) return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    dbgFile << "Colorspace: " << colorSpace->name();

    // Create the image
    m_image = new KisImage(m_adapter, width, height, colorSpace, "");

    if (!m_image) {
        return KisImageBuilder_RESULT_FAILURE;
    }
    m_image->lock();

    // Load the layers
    for (int i = 0; i < infos.size(); ++i) {
        ExrLayerInfo& info = infos[i];
        KisPaintLayerSP layer = new KisPaintLayer(m_image, info.name, OPACITY_OPAQUE, info.colorSpace);
        KisTransaction("", layer->paintDevice());

        layer->setCompositeOp(COMPOSITE_OVER);

        if (!layer) {
            return KisImageBuilder_RESULT_FAILURE;
        }

        // Decode the data
        switch (imageType) {
        case IT_FLOAT16:
            decodeData<half>(file, info, layer, width, dx, dy, height, Imf::HALF);
            break;
        case IT_FLOAT32:
            decodeData<float>(file, info, layer, width, dx, dy, height, Imf::FLOAT);
            break;
        case IT_UNKNOWN:
        case IT_UNSUPPORTED:
            qFatal("Impossible error");
        }

        m_image->addNode(layer, m_image->rootLayer());
        layer->setDirty();
    }
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

