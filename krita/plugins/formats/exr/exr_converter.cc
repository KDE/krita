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

#include <ImfAttribute.h>
#include <ImfChannelList.h>
#include <ImfInputFile.h>

#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>
#include <KoColorSpaceTraits.h>
#include <KoColorModelStandardIds.h>

#include <kis_doc2.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>
#include <kis_undo_adapter.h>

#include <metadata/kis_meta_data_entry.h>
#include <metadata/kis_meta_data_schema.h>
#include <metadata/kis_meta_data_schema_registry.h>
#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_value.h>

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

const KoColorSpace* kisTypeToColorSpace(QString model, ImageType imageType)
{
    switch (imageType) {
    case IT_FLOAT16:
        return KoColorSpaceRegistry::instance()->colorSpace(model, Float16BitsColorDepthID.id(), "");
    case IT_FLOAT32:
        return KoColorSpaceRegistry::instance()->colorSpace(model, Float32BitsColorDepthID.id(), "");
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

struct ExrGroupLayerInfo;

struct ExrLayerInfoBase {
    ExrLayerInfoBase() : colorSpace(0), parent(0) {
    }
    const KoColorSpace* colorSpace;
    QString name;
    const ExrGroupLayerInfo* parent;
};

struct ExrGroupLayerInfo : public ExrLayerInfoBase {
    ExrGroupLayerInfo() : groupLayer(0) {}
    KisGroupLayerSP groupLayer;
};

struct ExrPaintLayerInfo : public ExrLayerInfoBase {
    ExrPaintLayerInfo() : imageType(IT_UNKNOWN) {
    }
    ImageType imageType;
    QMap< QString, QString> channelMap; ///< first is either R, G, B or A second is the EXR channel name
    struct Remap {
        Remap(const QString& _original, const QString& _current) : original(_original), current(_current) {
        }
        QString original;
        QString current;
    };
    QList< Remap > remappedChannels; ///< this is used to store in the metadata the mapping between exr channel name, and channels used in Krita
    void updateImageType(ImageType channelType);
};

void ExrPaintLayerInfo::updateImageType(ImageType channelType)
{
    if (imageType == IT_UNKNOWN) {
        imageType = channelType;
    } else if (imageType != channelType) {
        imageType = IT_UNSUPPORTED;
    }
}

template<typename _T_>
void decodeData1(Imf::InputFile& file, ExrPaintLayerInfo& info, KisPaintLayerSP layer, int width, int xstart, int ystart, int height, Imf::PixelType ptype)
{
    QVector<_T_> pixels(width*height);

    Q_ASSERT(info.channelMap.contains("G"));
    dbgFile << "G -> " << info.channelMap["G"];

    for (int y = 0; y < height; ++y) {
        Imf::FrameBuffer frameBuffer;
        _T_* frameBufferData = (pixels.data()) - xstart - (ystart + y) * width;
        frameBuffer.insert(info.channelMap["G"].toAscii().data(),
                           Imf::Slice(ptype, (char *) frameBufferData,
                                      sizeof(_T_) * 1,
                                      sizeof(_T_) * width));

        file.setFrameBuffer(frameBuffer);
        file.readPixels(ystart + y);
        _T_ *rgba = pixels.data();
        KisHLineIterator it = layer->paintDevice()->createHLineIterator(0, y, width);
        while (!it.isDone()) {

            // XXX: For now unmultiply the alpha, though compositing will be faster if we
            // keep it premultiplied.
            _T_ unmultipliedRed = *rgba;

            _T_* dst = reinterpret_cast<_T_*>(it.rawData());

            *dst = unmultipliedRed;

            ++it;
            ++rgba;
        }
    }

}

template<typename _T_>
void decodeData4(Imf::InputFile& file, ExrPaintLayerInfo& info, KisPaintLayerSP layer, int width, int xstart, int ystart, int height, Imf::PixelType ptype)
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

            if (hasAlpha && rgba -> a >= HALF_EPSILON) {
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

bool recCheckGroup(const ExrGroupLayerInfo& group, QStringList list, int idx1, int idx2)
{
    if (idx1 > idx2) return true;
    if (group.name == list[idx2]) {
        return recCheckGroup(*group.parent, list, idx1, idx2 - 1);
    }
    return false;
}

ExrGroupLayerInfo* searchGroup(QList<ExrGroupLayerInfo>* groups, QStringList list, int idx1, int idx2)
{
    if (idx1 > idx2) {
        return 0;
    }
    // Look for the group
    for (int i = 0; i < groups->size(); ++i) {
        if (recCheckGroup(groups->at(i), list, idx1, idx2)) {
            return &(*groups)[i];
        }
    }
    // Create the group
    ExrGroupLayerInfo info;
    info.name = list.at(idx2);
    info.parent = searchGroup(groups, list, idx1, idx2 - 1);
    groups->append(info);
    return &groups->last();
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

    // Display the attributes of a file
    for (Imf::Header::ConstIterator it = file.header().begin();
            it != file.header().end(); ++it) {
        dbgFile << "Attribute: " << it.name() << " type: " << it.attribute().typeName();
    }

    // Constuct the list of LayerInfo

    QList<ExrPaintLayerInfo> infos;
    QList<ExrGroupLayerInfo> groups;

    ImageType imageType = IT_UNKNOWN;

    const Imf::ChannelList &channels = file.header().channels();
    std::set<std::string> layerNames;
    channels.layers(layerNames);

    // Test if it is a multilayer EXR or singlelayer
    if (layerNames.empty()) {
        dbgFile << "Single layer:";
        ExrPaintLayerInfo info;
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
            ExrPaintLayerInfo info;
            dbgFile << "layer name = " << i->c_str();
            info.name = i->c_str();
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

                if (list.size() > 1) {
                    info.name = list[list.size()-2];
                    info.parent = searchGroup(&groups, list, 0, list.size() - 3);
                }

                info.channelMap[layersuffix] = qname;
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
        ExrPaintLayerInfo& info = infos[i];
        QString modelId;
        if (info.channelMap.size() == 1) {
            modelId = GrayColorModelID.id();
            QString key = info.channelMap.begin().key();
            if (key != "G") {
                info.remappedChannels.push_back(ExrPaintLayerInfo::Remap(key, "G"));
                QString channel =  info.channelMap.begin().value();
                info.channelMap.clear();
                info.channelMap["G"] = channel;
            }
        } else if (info.channelMap.size() == 3 || info.channelMap.size() == 4) {
            if (info.channelMap.contains("R") && info.channelMap.contains("G") && info.channelMap.contains("B")) {
                modelId = RGBAColorModelID.id();
            } else if (info.channelMap.contains("X") && info.channelMap.contains("Y") && info.channelMap.contains("Z")) {
                modelId = XYZAColorModelID.id();
                QMap<QString, QString> newChannelMap;
                if (info.channelMap.contains("W")) {
                    newChannelMap["A"] = info.channelMap["W"];
                    info.remappedChannels.push_back(ExrPaintLayerInfo::Remap("W", "A"));
                    info.remappedChannels.push_back(ExrPaintLayerInfo::Remap("X", "X"));
                    info.remappedChannels.push_back(ExrPaintLayerInfo::Remap("Y", "Y"));
                    info.remappedChannels.push_back(ExrPaintLayerInfo::Remap("Z", "Z"));
                } else if (info.channelMap.contains("A")) {
                    newChannelMap["A"] = info.channelMap["A"];
                }
                // The decode function expect R, G, B in the channel map
                newChannelMap["B"] = info.channelMap["X"];
                newChannelMap["G"] = info.channelMap["Y"];
                newChannelMap["R"] = info.channelMap["Z"];
                info.channelMap = newChannelMap;
            } else {
                modelId = RGBAColorModelID.id();
                QMap<QString, QString> newChannelMap;
                QMap<QString, QString>::iterator it = info.channelMap.begin();
                newChannelMap["R"] = it.value();
                info.remappedChannels.push_back(ExrPaintLayerInfo::Remap(it.key(), "R"));
                ++it;
                newChannelMap["G"] = it.value();
                info.remappedChannels.push_back(ExrPaintLayerInfo::Remap(it.key(), "G"));
                ++it;
                newChannelMap["B"] = it.value();
                info.remappedChannels.push_back(ExrPaintLayerInfo::Remap(it.key(), "B"));
                if (info.channelMap.size() == 4) {
                    ++it;
                    newChannelMap["A"] = it.value();
                    info.remappedChannels.push_back(ExrPaintLayerInfo::Remap(it.key(), "A"));
                }
                info.channelMap = newChannelMap;
            }
        }
        if (!modelId.isEmpty()) {
            info.colorSpace = kisTypeToColorSpace(modelId, info.imageType);
        }
    }
    // Get colorspace
    dbgFile << "Image type = " << imageType;
    const KoColorSpace* colorSpace = kisTypeToColorSpace(RGBAColorModelID.id(), imageType);

    if (!colorSpace) return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    dbgFile << "Colorspace: " << colorSpace->name();

    // Set the colorspace on all groups
    for (int i = 0; i < groups.size(); ++i) {
        ExrGroupLayerInfo& info = groups[i];
        info.colorSpace = colorSpace;
    }

    // Create the image
    m_image = new KisImage(m_adapter, width, height, colorSpace, "");

    if (!m_image) {
        return KisImageBuilder_RESULT_FAILURE;
    }
    m_image->lock();

    // Create group layers
    for (int i = 0; i < groups.size(); ++i) {
        ExrGroupLayerInfo& info = groups[i];
        Q_ASSERT(info.parent == 0 || info.parent->groupLayer);
        KisGroupLayerSP groupLayerParent = (info.parent) ? info.parent->groupLayer : m_image->rootLayer();
        info.groupLayer = new KisGroupLayer(m_image, info.name, OPACITY_OPAQUE);
        m_image->addNode(info.groupLayer, groupLayerParent);
    }

    // Load the layers
    for (int i = 0; i < infos.size(); ++i) {
        ExrPaintLayerInfo& info = infos[i];
        if (info.colorSpace) {
            dbgFile << "Decoding " << info.name << " with " << info.channelMap.size() << " channels, and color space " << info.colorSpace->id();
            KisPaintLayerSP layer = new KisPaintLayer(m_image, info.name, OPACITY_OPAQUE, info.colorSpace);
            KisTransaction("", layer->paintDevice());

            layer->setCompositeOp(COMPOSITE_OVER);

            if (!layer) {
                return KisImageBuilder_RESULT_FAILURE;
            }

            switch (info.channelMap.size()) {
            case 1:
                // Decode the data
                switch (imageType) {
                case IT_FLOAT16:
                    decodeData1<half>(file, info, layer, width, dx, dy, height, Imf::HALF);
                    break;
                case IT_FLOAT32:
                    decodeData1<float>(file, info, layer, width, dx, dy, height, Imf::FLOAT);
                    break;
                case IT_UNKNOWN:
                case IT_UNSUPPORTED:
                    qFatal("Impossible error");
                }
                break;
            case 3:
            case 4:
                // Decode the data
                switch (imageType) {
                case IT_FLOAT16:
                    decodeData4<half>(file, info, layer, width, dx, dy, height, Imf::HALF);
                    break;
                case IT_FLOAT32:
                    decodeData4<float>(file, info, layer, width, dx, dy, height, Imf::FLOAT);
                    break;
                case IT_UNKNOWN:
                case IT_UNSUPPORTED:
                    qFatal("Impossible error");
                }
                break;
            default:
                qFatal("Invalid number of channels: %i", info.channelMap.size());
            }
            // Check if should set the channels
            if (!info.remappedChannels.isEmpty()) {
                QList<KisMetaData::Value> values;
                foreach(const ExrPaintLayerInfo::Remap& remap, info.remappedChannels) {
                    QMap<QString, KisMetaData::Value> map;
                    map["original"] = KisMetaData::Value(remap.original);
                    map["current"] = KisMetaData::Value(remap.current);
                    values.append(map);
                }
                layer->metaData()->addEntry(KisMetaData::Entry(KisMetaData::SchemaRegistry::instance()->create("http://krita.org/exrchannels/1.0/" , "exrchannels"), "channelsmap", values));
            }
            // Add the layer
            KisGroupLayerSP groupLayerParent = (info.parent) ? info.parent->groupLayer : m_image->rootLayer();
            m_image->addNode(layer, groupLayerParent);
            layer->setDirty();
        } else {
            dbgFile << "No decoding " << info.name << " with " << info.channelMap.size() << " channels, and lack of a color space";
        }
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

