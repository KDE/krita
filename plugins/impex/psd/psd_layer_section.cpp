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
#include "psd_layer_section.h"


#include <QIODevice>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_debug.h>
#include <kis_node.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_effect_mask.h>
#include <kis_image.h>


#include "kis_dom_utils.h"

#include "psd_header.h"
#include "psd_utils.h"

#include "compression.h"

#include <asl/kis_offset_on_exit_verifier.h>
#include <asl/kis_asl_reader_utils.h>
#include <kis_asl_layer_style_serializer.h>
#include <asl/kis_asl_writer_utils.h>


PSDLayerMaskSection::PSDLayerMaskSection(const PSDHeader& header)
    : globalInfoSection(header),
      m_header(header)
{
    hasTransparency = false;
    layerMaskBlockSize = 0;
    nLayers = 0;
}

PSDLayerMaskSection::~PSDLayerMaskSection()
{
    qDeleteAll(layers);
}

bool PSDLayerMaskSection::read(QIODevice* io)
{
    bool retval = true; // be optimistic! <:-)

    try {
        retval = readImpl(io);
    } catch (KisAslReaderUtils::ASLParseException &e) {
        warnKrita << "WARNING: PSD (emb. pattern):" << e.what();
        retval = false;
    }

    return retval;
}

bool PSDLayerMaskSection::readLayerInfoImpl(QIODevice* io)
{
    quint32 layerInfoSectionSize = 0;
    SAFE_READ_EX(io, layerInfoSectionSize);

    if (layerInfoSectionSize & 0x1) {
        warnKrita << "WARNING: layerInfoSectionSize is NOT even! Fixing...";
        layerInfoSectionSize++;
    }

    {
        SETUP_OFFSET_VERIFIER(layerInfoSectionTag, io, layerInfoSectionSize, 0);
        dbgFile << "Layer info block size" << layerInfoSectionSize;

        if (layerInfoSectionSize > 0 ) {

            if (!psdread(io, &nLayers) || nLayers == 0) {
                error = QString("Could not read read number of layers or no layers in image. %1").arg(nLayers);
                return false;
            }

            hasTransparency = nLayers < 0; // first alpha channel is the alpha channel of the projection.
            nLayers = qAbs(nLayers);

            dbgFile << "Number of layers:" << nLayers;
            dbgFile << "Has separate projection transparency:" << hasTransparency;

            for (int i = 0; i < nLayers; ++i) {

                dbgFile << "Going to read layer" << i << "pos" << io->pos();
                dbgFile << "== Enter PSDLayerRecord";
                QScopedPointer<PSDLayerRecord> layerRecord(new PSDLayerRecord(m_header));
                if (!layerRecord->read(io)) {
                    error = QString("Could not load layer %1: %2").arg(i).arg(layerRecord->error);
                    return false;
                }
                dbgFile << "== Leave PSDLayerRecord";
                dbgFile << "Finished reading layer" << i << layerRecord->layerName << "blending mode"
                        << layerRecord->blendModeKey << io->pos()
                        << "Number of channels:" <<  layerRecord->channelInfoRecords.size();
                layers << layerRecord.take();
            }
        }

        // get the positions for the channels belonging to each layer
        for (int i = 0; i < nLayers; ++i) {

            dbgFile << "Going to seek channel positions for layer" << i << "pos" << io->pos();
            if (i > layers.size()) {
                error = QString("Expected layer %1, but only have %2 layers").arg(i).arg(layers.size());
                return false;
            }

            PSDLayerRecord *layerRecord = layers.at(i);

            for (int j = 0; j < layerRecord->nChannels; ++j) {
                // save the current location so we can jump beyond this block later on.
                quint64 channelStartPos = io->pos();
                dbgFile << "\tReading channel image data for channel" << j << "from pos" << io->pos();

                KIS_ASSERT_RECOVER(j < layerRecord->channelInfoRecords.size()) { return false; }

                ChannelInfo* channelInfo = layerRecord->channelInfoRecords.at(j);

                quint16 compressionType;
                if (!psdread(io, &compressionType)) {
                    error = "Could not read compression type for channel";
                    return false;
                }
                channelInfo->compressionType = (Compression::CompressionType)compressionType;
                dbgFile << "\t\tChannel" << j << "has compression type" << compressionType;

                QRect channelRect = layerRecord->channelRect(channelInfo);

                // read the rle row lengths;
                if (channelInfo->compressionType == Compression::RLE) {
                    for(qint64 row = 0; row < channelRect.height(); ++row) {

                        //dbgFile << "Reading the RLE bytecount position of row" << row << "at pos" << io->pos();

                        quint32 byteCount;
                        if (m_header.version == 1) {
                            quint16 _byteCount;
                            if (!psdread(io, &_byteCount)) {
                                error = QString("Could not read byteCount for rle-encoded channel");
                                return 0;
                            }
                            byteCount = _byteCount;
                        }
                        else {
                            if (!psdread(io, &byteCount)) {
                                error = QString("Could not read byteCount for rle-encoded channel");
                                return 0;
                            }
                        }
                        ////dbgFile << "rle byte count" << byteCount;
                        channelInfo->rleRowLengths << byteCount;
                    }
                }

                // we're beyond all the length bytes, rle bytes and whatever, this is the
                // location of the real pixel data
                channelInfo->channelDataStart = io->pos();

                dbgFile << "\t\tstart" << channelStartPos
                        << "data start" << channelInfo->channelDataStart
                        << "data length" << channelInfo->channelDataLength
                        << "pos" << io->pos();

                // make sure we are at the start of the next channel data block
                io->seek(channelStartPos + channelInfo->channelDataLength);

                // this is the length of the actual channel data bytes
                channelInfo->channelDataLength = channelInfo->channelDataLength - (channelInfo->channelDataStart - channelStartPos);

                dbgFile << "\t\tchannel record" << j << "for layer" << i << "with id" << channelInfo->channelId
                        << "starting position" << channelInfo->channelDataStart
                        << "with length" << channelInfo->channelDataLength
                        << "and has compression type" << channelInfo->compressionType;

            }
        }
    }

    return true;
}

bool PSDLayerMaskSection::readImpl(QIODevice* io)
{
    dbgFile << "reading layer section. Pos:" << io->pos() <<  "bytes left:" << io->bytesAvailable();

    layerMaskBlockSize = 0;
    if (m_header.version == 1) {
        quint32 _layerMaskBlockSize = 0;
        if (!psdread(io, &_layerMaskBlockSize) || _layerMaskBlockSize > (quint64)io->bytesAvailable()) {
            error = QString("Could not read layer + mask block size. Got %1. Bytes left %2")
                .arg(_layerMaskBlockSize).arg(io->bytesAvailable());
            return false;
        }
        layerMaskBlockSize = _layerMaskBlockSize;
    }
    else if (m_header.version == 2) {
        if (!psdread(io, &layerMaskBlockSize) || layerMaskBlockSize > (quint64)io->bytesAvailable()) {
            error = QString("Could not read layer + mask block size. Got %1. Bytes left %2")
                .arg(layerMaskBlockSize).arg(io->bytesAvailable());
            return false;
        }
    }

    quint64 start = io->pos();

    dbgFile << "layer + mask section size" << layerMaskBlockSize;

    if (layerMaskBlockSize == 0) {
        dbgFile << "No layer + mask info, so no layers, only a background layer";
        return true;
    }

    if (!readLayerInfoImpl(io)) {
        return false;
    }

    quint32 globalMaskBlockLength;
    if (!psdread(io, &globalMaskBlockLength)) {
        error = "Could not read global mask info block";
        return false;
    }

    if (globalMaskBlockLength > 0) {

        if (!psdread(io, &globalLayerMaskInfo.overlayColorSpace)) {
            error = "Could not read global mask info overlay colorspace";
            return false;
        }

        for (int i = 0; i < 4; ++i) {
            if (!psdread(io, &globalLayerMaskInfo.colorComponents[i])) {
                error = QString("Could not read mask info visualizaion color component %1").arg(i);
                return false;
            }
        }

        if (!psdread(io, &globalLayerMaskInfo.opacity)) {
            error = "Could not read global mask info visualization opacity";
            return false;
        }

        if (!psdread(io, &globalLayerMaskInfo.kind)) {
            error = "Could not read global mask info visualization type";
            return false;
        }
    }

    // global additional sections

    /**
     * Newer versions of PSD have layers info block wrapped into
     * 'Lr16' or 'Lr32' additional section, while the main block is
     * absent.
     *
     * Here we pass the callback which should be used when such
     * additional section is recognized.
     */
    globalInfoSection.setExtraLayerInfoBlockHandler(std::bind(&PSDLayerMaskSection::readLayerInfoImpl, this, std::placeholders::_1));

    globalInfoSection.read(io);

    /* put us after this section so reading the next section will work even if we mess up */
    io->seek(start + layerMaskBlockSize);

    return true;
}

struct FlattenedNode {
    FlattenedNode() : type(RASTER_LAYER) {}

    KisNodeSP node;

    enum Type {
        RASTER_LAYER,
        FOLDER_OPEN,
        FOLDER_CLOSED,
        SECTION_DIVIDER
    };

    Type type;
};

void addBackgroundIfNeeded(KisNodeSP root, QList<FlattenedNode> &nodes)
{
    KisGroupLayer *group = dynamic_cast<KisGroupLayer*>(root.data());
    if (!group) return;

    KoColor projectionColor = group->defaultProjectionColor();
    if (projectionColor.opacityU8() == OPACITY_TRANSPARENT_U8) return;

    KisPaintLayerSP layer =
        new KisPaintLayer(group->image(),
                          i18nc("Automatically created layer name when saving into PSD", "Background"),
                          OPACITY_OPAQUE_U8);

    layer->paintDevice()->setDefaultPixel(projectionColor);

    {
        FlattenedNode item;
        item.node = layer;
        item.type = FlattenedNode::RASTER_LAYER;
        nodes << item;
    }
}

void flattenNodes(KisNodeSP node, QList<FlattenedNode> &nodes)
{
    KisNodeSP child = node->firstChild();
    while (child) {

        bool isGroupLayer = child->inherits("KisGroupLayer");
        bool isRasterLayer = child->inherits("KisPaintLayer") || child->inherits("KisShapeLayer");

        if (isGroupLayer) {
            {
                FlattenedNode item;
                item.node = child;
                item.type = FlattenedNode::SECTION_DIVIDER;
                nodes << item;
            }

            flattenNodes(child, nodes);

            {
                FlattenedNode item;
                item.node = child;
                item.type = FlattenedNode::FOLDER_OPEN;
                nodes << item;
            }
        } else if (isRasterLayer) {
            FlattenedNode item;
            item.node = child;
            item.type = FlattenedNode::RASTER_LAYER;
            nodes << item;
        }

        child = child->nextSibling();
    }
}

KisNodeSP findOnlyTransparencyMask(KisNodeSP node, FlattenedNode::Type type)
{
    if (type != FlattenedNode::FOLDER_OPEN &&
        type != FlattenedNode::FOLDER_CLOSED &&
        type != FlattenedNode::RASTER_LAYER) {

        return 0;
    }

    KisLayer *layer = qobject_cast<KisLayer*>(node.data());
    QList<KisEffectMaskSP> masks = layer->effectMasks();

    if (masks.size() != 1) return 0;

    KisEffectMaskSP onlyMask = masks.first();
    return onlyMask->inherits("KisTransparencyMask") ? onlyMask : 0;
}

QDomDocument fetchLayerStyleXmlData(KisNodeSP node)
{
    const KisLayer *layer = qobject_cast<KisLayer*>(node.data());
    KisPSDLayerStyleSP layerStyle = layer->layerStyle();

    if (!layerStyle) return QDomDocument();

    KisAslLayerStyleSerializer serializer;
    serializer.setStyles(QVector<KisPSDLayerStyleSP>() << layerStyle);
    return serializer.formPsdXmlDocument();
}

inline QDomNode findNodeByKey(const QString &key, QDomNode parent) {
    return KisDomUtils::findElementByAttibute(parent, "node", "key", key);
}

void mergePatternsXMLSection(const QDomDocument &src, QDomDocument &dst)
{
    QDomNode srcPatternsNode = findNodeByKey("Patterns", src.documentElement());
    QDomNode dstPatternsNode = findNodeByKey("Patterns", dst.documentElement());

    if (srcPatternsNode.isNull()) return;
    if (dstPatternsNode.isNull()) {
        dst = src;
        return;
    }

    KIS_ASSERT_RECOVER_RETURN(!srcPatternsNode.isNull());
    KIS_ASSERT_RECOVER_RETURN(!dstPatternsNode.isNull());

    QDomNode node = srcPatternsNode.firstChild();
    while(!node.isNull()) {
        QDomNode importedNode = dst.importNode(node, true);
        KIS_ASSERT_RECOVER_RETURN(!importedNode.isNull());

        dstPatternsNode.appendChild(importedNode);
        node = node.nextSibling();
    }

}

bool PSDLayerMaskSection::write(QIODevice* io, KisNodeSP rootLayer)
{
    bool retval = true;

    try {
        writeImpl(io, rootLayer);
    } catch (KisAslWriterUtils::ASLWriteException &e) {
        error = PREPEND_METHOD(e.what());
        retval = false;
    }

    return retval;
}

void PSDLayerMaskSection::writeImpl(QIODevice* io, KisNodeSP rootLayer)
{
    dbgFile << "Writing layer layer section";

    // Build the whole layer structure
    QList<FlattenedNode> nodes;
    addBackgroundIfNeeded(rootLayer, nodes);
    flattenNodes(rootLayer, nodes);

    if (nodes.isEmpty()) {
        throw KisAslWriterUtils::ASLWriteException("Could not find paint layers to save");
    }

    {
        KisAslWriterUtils::OffsetStreamPusher<quint32> layerAndMaskSectionSizeTag(io, 2);
        QDomDocument mergedPatternsXmlDoc;

        {
            KisAslWriterUtils::OffsetStreamPusher<quint32> layerInfoSizeTag(io, 4);

            {
                // number of layers (negative, because krita always has alpha)
                const qint16 layersSize = -nodes.size();
                SAFE_WRITE_EX(io, layersSize);

                dbgFile << "Number of layers" << layersSize << "at" << io->pos();
            }

            // Layer records section
            Q_FOREACH (const FlattenedNode &item, nodes) {
                KisNodeSP node = item.node;

                PSDLayerRecord *layerRecord = new PSDLayerRecord(m_header);
                layers.append(layerRecord);

                KisNodeSP onlyTransparencyMask = findOnlyTransparencyMask(node, item.type);
                const QRect maskRect = onlyTransparencyMask ? onlyTransparencyMask->paintDevice()->exactBounds() : QRect();

                const bool nodeVisible = node->visible();
                const KoColorSpace *colorSpace = node->colorSpace();
                const quint8 nodeOpacity = node->opacity();
                const quint8 nodeClipping = 0;
                const KisPaintLayer *paintLayer = qobject_cast<KisPaintLayer*>(node.data());
                const bool alphaLocked = (paintLayer && paintLayer->alphaLocked());
                const QString nodeCompositeOp = node->compositeOpId();

                const KisGroupLayer *groupLayer = qobject_cast<KisGroupLayer*>(node.data());
                const bool nodeIsPassThrough = groupLayer && groupLayer->passThroughMode();

                QDomDocument stylesXmlDoc = fetchLayerStyleXmlData(node);

                if (mergedPatternsXmlDoc.isNull() && !stylesXmlDoc.isNull()) {
                    mergedPatternsXmlDoc = stylesXmlDoc;
                } else if (!mergedPatternsXmlDoc.isNull() && !stylesXmlDoc.isNull()) {
                    mergePatternsXMLSection(stylesXmlDoc, mergedPatternsXmlDoc);
                }

                bool nodeIrrelevant = false;
                QString nodeName;
                KisPaintDeviceSP layerContentDevice;
                psd_section_type sectionType;

                if (item.type == FlattenedNode::RASTER_LAYER) {
                    nodeIrrelevant = false;
                    nodeName = node->name();
                    layerContentDevice = onlyTransparencyMask ? node->original() : node->projection();
                    sectionType = psd_other;
                } else {
                    nodeIrrelevant = true;
                    nodeName = item.type == FlattenedNode::SECTION_DIVIDER ?
                        QString("</Layer group>") :
                        node->name();
                    layerContentDevice = 0;
                    sectionType =
                        item.type == FlattenedNode::SECTION_DIVIDER ? psd_bounding_divider :
                        item.type == FlattenedNode::FOLDER_OPEN ? psd_open_folder :
                        psd_closed_folder;
                }


                // === no access to node anymore

                QRect layerRect;

                if (layerContentDevice) {
                    QRect rc = layerContentDevice->exactBounds();
                    rc = rc.normalized();

                    // keep to the max of photoshop's capabilities
                    if (rc.width() > 30000) rc.setWidth(30000);
                    if (rc.height() > 30000) rc.setHeight(30000);

                    layerRect = rc;
                }

                layerRecord->top = layerRect.y();
                layerRecord->left = layerRect.x();
                layerRecord->bottom = layerRect.y() + layerRect.height();
                layerRecord->right = layerRect.x() + layerRect.width();

                // colors + alpha channel
                // note: transparency mask not included
                layerRecord->nChannels = colorSpace->colorChannelCount() + 1;

                ChannelInfo *info = new ChannelInfo;
                info->channelId = -1; // For the alpha channel, which we always have in Krita, and should be saved first in
                layerRecord->channelInfoRecords << info;

                // the rest is in display order: rgb, cmyk, lab...
                for (int i = 0; i < (int)colorSpace->colorChannelCount(); ++i) {
                    info = new ChannelInfo;
                    info->channelId = i; // 0 for red, 1 = green, etc
                    layerRecord->channelInfoRecords << info;
                }

                layerRecord->blendModeKey = composite_op_to_psd_blendmode(nodeCompositeOp);
                layerRecord->isPassThrough = nodeIsPassThrough;
                layerRecord->opacity = nodeOpacity;
                layerRecord->clipping = nodeClipping;

                layerRecord->transparencyProtected = alphaLocked;
                layerRecord->visible = nodeVisible;
                layerRecord->irrelevant = nodeIrrelevant;

                layerRecord->layerName = nodeName.isEmpty() ? i18n("Unnamed Layer") : nodeName;

                layerRecord->write(io,
                                   layerContentDevice,
                                   onlyTransparencyMask,
                                   maskRect,
                                   sectionType,
                                   stylesXmlDoc,
                                   node->inherits("KisGroupLayer"));
            }

            dbgFile << "start writing layer pixel data" << io->pos();

            // Now save the pixel data
            Q_FOREACH (PSDLayerRecord *layerRecord, layers) {
                layerRecord->writePixelData(io);
            }

        }

        {
            // write the global layer mask info -- which is empty
            const quint32 globalMaskSize = 0;
            SAFE_WRITE_EX(io, globalMaskSize);
        }

        globalInfoSection.writePattBlockEx(io, mergedPatternsXmlDoc);
    }
}
