/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_layer_section.h"

#include <QBuffer>
#include <QIODevice>

#include <KoColor.h>
#include <KoColorSpace.h>

#include <kis_debug.h>
#include <kis_effect_mask.h>
#include <kis_group_layer.h>
#include <kis_generator_layer.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_selection.h>
#include <kis_shape_selection.h>
#include <kis_transparency_mask.h>
#include <kis_shape_layer.h>
#include <KoSvgTextShape.h>
#include <KoSvgTextShapeMarkupConverter.h>
#include <KoShapeBackground.h>
#include <KoColorBackground.h>
#include <KoPatternBackground.h>
#include <KoGradientBackground.h>
#include <KoShapeStroke.h>

#include <KoPathShape.h>

#include "kis_dom_utils.h"

#include "psd.h"
#include "psd_header.h"
#include "psd_utils.h"

#include "compression.h"

#include <asl/kis_asl_reader_utils.h>
#include <asl/kis_asl_writer_utils.h>
#include <asl/kis_offset_on_exit_verifier.h>
#include <kis_asl_layer_style_serializer.h>
#include <cos/kis_txt2_utls.h>

PSDLayerMaskSection::PSDLayerMaskSection(const PSDHeader &header)
    : globalInfoSection(header)
    , m_header(header)
{
}

PSDLayerMaskSection::~PSDLayerMaskSection()
{
    qDeleteAll(layers);
}

bool PSDLayerMaskSection::read(QIODevice &io)
{
    bool retval = true; // be optimistic! <:-)

    try {
        if (m_header.tiffStyleLayerBlock) {
            switch (m_header.byteOrder) {
            case psd_byte_order::psdLittleEndian:
                retval = readTiffImpl<psd_byte_order::psdLittleEndian>(io);
                break;
            default:
                retval = readTiffImpl(io);
                break;
            }
        } else {
            retval = readPsdImpl(io);
        }
    } catch (KisAslReaderUtils::ASLParseException &e) {
        warnKrita << "WARNING: PSD (emb. pattern):" << e.what();
        retval = false;
    }

    return retval;
}

template<psd_byte_order byteOrder>
bool PSDLayerMaskSection::readLayerInfoImpl(QIODevice &io)
{
    quint64 layerInfoSectionSize = 0;
    if (m_header.version == 1) {
        quint32 _layerInfoSectionSize = 0;
        SAFE_READ_EX(byteOrder, io, _layerInfoSectionSize);
        layerInfoSectionSize = _layerInfoSectionSize;
    } else if (m_header.version == 2) {
        SAFE_READ_EX(byteOrder, io, layerInfoSectionSize);
    }

    if (layerInfoSectionSize & 0x1) {
        warnKrita << "WARNING: layerInfoSectionSize is NOT even! Fixing...";
        layerInfoSectionSize++;
    }

    {
        SETUP_OFFSET_VERIFIER(layerInfoSectionTag, io, layerInfoSectionSize, 0);
        dbgFile << "Layer info block size" << layerInfoSectionSize;

        if (layerInfoSectionSize > 0) {
            if (!psdread<byteOrder>(io, nLayers) || nLayers == 0) {
                error = QString("Could not read number of layers or no layers in image. %1").arg(nLayers);
                return false;
            }

            hasTransparency = nLayers < 0; // first alpha channel is the alpha channel of the projection.
            nLayers = qAbs(nLayers);

            dbgFile << "Number of layers:" << nLayers;
            dbgFile << "Has separate projection transparency:" << hasTransparency;

            for (int i = 0; i < nLayers; ++i) {
                dbgFile << "Going to read layer" << i << "pos" << io.pos();
                dbgFile << "== Enter PSDLayerRecord";
                PSDHeader sanitizedHeader(m_header);
                sanitizedHeader.tiffStyleLayerBlock = false; // disable padding
                QScopedPointer<PSDLayerRecord> layerRecord(new PSDLayerRecord(sanitizedHeader));
                if (!layerRecord->read(io)) {
                    error = QString("Could not load layer %1: %2").arg(i).arg(layerRecord->error);
                    return false;
                }
                dbgFile << "== Leave PSDLayerRecord";
                dbgFile << "Finished reading layer" << i << layerRecord->layerName << "blending mode" << layerRecord->blendModeKey << io.pos()
                        << "Number of channels:" << layerRecord->channelInfoRecords.size();
                layers << layerRecord.take();
            }
        }

        // get the positions for the channels belonging to each layer
        for (int i = 0; i < nLayers; ++i) {
            dbgFile << "Going to seek channel positions for layer" << i << "pos" << io.pos();
            if (i > layers.size()) {
                error = QString("Expected layer %1, but only have %2 layers").arg(i).arg(layers.size());
                return false;
            }

            PSDLayerRecord *layerRecord = layers.at(i);

            for (int j = 0; j < layerRecord->nChannels; ++j) {
                // save the current location so we can jump beyond this block later on.
                quint64 channelStartPos = io.pos();
                dbgFile << "\tReading channel image data for channel" << j << "from pos" << io.pos();

                KIS_ASSERT_RECOVER(j < layerRecord->channelInfoRecords.size())
                {
                    return false;
                }

                ChannelInfo *channelInfo = layerRecord->channelInfoRecords.at(j);

                quint16 compressionType;
                if (!psdread<byteOrder>(io, compressionType)) {
                    error = "Could not read compression type for channel";
                    return false;
                }
                channelInfo->compressionType = static_cast<psd_compression_type>(compressionType);
                dbgFile << "\t\tChannel" << j << "has compression type" << compressionType;

                QRect channelRect = layerRecord->channelRect(channelInfo);

                // read the rle row lengths;
                if (channelInfo->compressionType == psd_compression_type::RLE) {
                    for (qint64 row = 0; row < channelRect.height(); ++row) {
                        // dbgFile << "Reading the RLE byte count position of row" << row << "at pos" << io.pos();

                        quint32 byteCount;
                        if (m_header.version == 1) {
                            quint16 _byteCount;
                            if (!psdread<byteOrder>(io, _byteCount)) {
                                error = QString("Could not read byteCount for rle-encoded channel");
                                return 0;
                            }
                            byteCount = _byteCount;
                        } else {
                            if (!psdread<byteOrder>(io, byteCount)) {
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
                channelInfo->channelDataStart = io.pos();

                dbgFile << "\t\tstart" << channelStartPos << "data start" << channelInfo->channelDataStart << "data length" << channelInfo->channelDataLength
                        << "pos" << io.pos();

                // make sure we are at the start of the next channel data block
                io.seek(channelStartPos + channelInfo->channelDataLength);

                // this is the length of the actual channel data bytes
                channelInfo->channelDataLength = channelInfo->channelDataLength - (channelInfo->channelDataStart - channelStartPos);

                dbgFile << "\t\tchannel record" << j << "for layer" << i << "with id" << channelInfo->channelId << "starting position"
                        << channelInfo->channelDataStart << "with length" << channelInfo->channelDataLength << "and has compression type"
                        << channelInfo->compressionType;
            }
        }
    }

    return true;
}

bool PSDLayerMaskSection::readPsdImpl(QIODevice &io)
{
    dbgFile << "(PSD) reading layer section. Pos:" << io.pos() << "bytes left:" << io.bytesAvailable();

    // https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_21849
    boost::optional<quint64> layerMaskBlockSize = 0;

    if (m_header.version == 1) {
        quint32 _layerMaskBlockSize = 0;
        SAFE_READ_EX(psd_byte_order::psdBigEndian, io, _layerMaskBlockSize);
        layerMaskBlockSize = _layerMaskBlockSize;
    } else if (m_header.version == 2) {
        SAFE_READ_EX(psd_byte_order::psdBigEndian, io, *layerMaskBlockSize);
    }

    qint64 start = io.pos();

    dbgFile << "layer block size" << *layerMaskBlockSize;

    if (*layerMaskBlockSize == 0) {
        dbgFile << "No layer info, so no PSD layers available";
        return true;
    }

    /**
     * PSD files created in some weird web applications may
     * have invalid layer-mask-block-size set. Just do a simple
     * sanity check to catch this case
     */
    if (static_cast<qint64>(*layerMaskBlockSize) > io.bytesAvailable()) {
        warnKrita << "WARNING: invalid layer block size. Got" << *layerMaskBlockSize << "Bytes left" << io.bytesAvailable() << "Triggering a workaround...";

        // just don't use this value for offset recovery at the end
        layerMaskBlockSize = boost::none;
    }

    if (!readLayerInfoImpl(io)) {
        return false;
    }

    dbgFile << "Leftover before additional blocks:" << io.pos() << io.bytesAvailable();

    quint32 globalMaskBlockLength;
    if (!psdread(io, globalMaskBlockLength)) {
        error = "Could not read global mask info block";
        return false;
    }

    dbgFile << "Global mask size:" << globalMaskBlockLength << "(" << io.pos() << io.bytesAvailable() << ")";

    if (globalMaskBlockLength > 0) {
        if (!psdread(io, globalLayerMaskInfo.overlayColorSpace)) {
            error = "Could not read global mask info overlay colorspace";
            return false;
        }

        for (int i = 0; i < 4; ++i) {
            if (!psdread(io, globalLayerMaskInfo.colorComponents[i])) {
                error = QString("Could not read mask info visualization color component %1").arg(i);
                return false;
            }
        }

        if (!psdread(io, globalLayerMaskInfo.opacity)) {
            error = "Could not read global mask info visualization opacity";
            return false;
        }

        if (!psdread(io, globalLayerMaskInfo.kind)) {
            error = "Could not read global mask info visualization type";
            return false;
        }

        // Global mask must measure at least 13 bytes
        // (excluding the 1 byte compiler enforced padding)
        if (globalMaskBlockLength >= 13) {
            dbgFile << "Padding for global mask block:"
                    << globalMaskBlockLength - 13 << "(" << io.pos() << ")";
            io.skip(static_cast<size_t>(globalMaskBlockLength) - 13);
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
    globalInfoSection.setExtraLayerInfoBlockHandler(
        std::bind(&PSDLayerMaskSection::readLayerInfoImpl<psd_byte_order::psdBigEndian>, this, std::placeholders::_1));

    dbgFile << "Position before starting global info section:" << io.pos();

    globalInfoSection.read(io);

    if (layerMaskBlockSize) {
        /* put us after this section so reading the next section will work even if we mess up */
        io.seek(start + static_cast<qint64>(*layerMaskBlockSize));
    }

    return true;
}

template<psd_byte_order byteOrder>
bool PSDLayerMaskSection::readGlobalMask(QIODevice &io)
{
    quint32 globalMaskBlockLength;
    if (!psdread<byteOrder>(io, globalMaskBlockLength)) {
        error = "Could not read global mask info block";
        return false;
    }

    dbgFile << "Global mask size:" << globalMaskBlockLength << "(" << io.pos() << io.bytesAvailable() << ")";

    if (globalMaskBlockLength > 0) {
        if (!psdread<byteOrder>(io, globalLayerMaskInfo.overlayColorSpace)) {
            error = "Could not read global mask info overlay colorspace";
            return false;
        }

        for (int i = 0; i < 4; ++i) {
            if (!psdread<byteOrder>(io, globalLayerMaskInfo.colorComponents[i])) {
                error = QString("Could not read mask info visualization color component %1").arg(i);
                return false;
            }
        }

        if (!psdread<byteOrder>(io, globalLayerMaskInfo.opacity)) {
            error = "Could not read global mask info visualization opacity";
            return false;
        }

        if (!psdread<byteOrder>(io, globalLayerMaskInfo.kind)) {
            error = "Could not read global mask info visualization type";
            return false;
        }

        dbgFile << "Global mask info: ";
        dbgFile << "\tOverlay:" << globalLayerMaskInfo.overlayColorSpace; // 0
        dbgFile << "\tColor components:" << globalLayerMaskInfo.colorComponents[0] // 65535
                << globalLayerMaskInfo.colorComponents[1] // 0
                << globalLayerMaskInfo.colorComponents[2] // 0
                << globalLayerMaskInfo.colorComponents[3]; // 0
        dbgFile << "\tOpacity:" << globalLayerMaskInfo.opacity; // 50
        dbgFile << "\tKind:" << globalLayerMaskInfo.kind; // 128

        if (globalMaskBlockLength >= 15) {
            io.skip(qMax(globalMaskBlockLength - 15, 0x0U));
        }
    }

    return true;
}

template<psd_byte_order byteOrder>
bool PSDLayerMaskSection::readTiffImpl(QIODevice &io)
{
    dbgFile << "(TIFF) reading layer section. Pos:" << io.pos() << "bytes left:" << io.bytesAvailable();

    // TIFF additional sections

    /**
     * Just like PSD, new versions of PSD have layers info block wrapped into
     * 'Lr16' or 'Lr32' additional section, while the main block is
     * absent.
     * Additionally, the global mask info is stored in a separate "LMsk" block.
     *
     * So, instead of having special handling, we just ship everything to the
     * additional layer info block handlers
     */

    globalInfoSection.setExtraLayerInfoBlockHandler(std::bind(&PSDLayerMaskSection::readLayerInfoImpl<byteOrder>, this, std::placeholders::_1));
    globalInfoSection.setUserMaskInfoBlockHandler(std::bind(&PSDLayerMaskSection::readGlobalMask<byteOrder>, this, std::placeholders::_1));

    if (!globalInfoSection.read(io)) {
        dbgFile << "Failed to read TIFF Photoshop blocks!";
        return false;
    }

    dbgFile << "Leftover data after parsing layer/extra blocks:" << io.pos() << io.bytesAvailable() << io.peek(io.bytesAvailable());

    return true;
}

struct FlattenedNode {
    FlattenedNode()
        : type(RASTER_LAYER)
    {
    }

    KisNodeSP node;

    enum Type { RASTER_LAYER, FOLDER_OPEN, FOLDER_CLOSED, SECTION_DIVIDER };

    Type type;
};

void addBackgroundIfNeeded(KisNodeSP root, QList<FlattenedNode> &nodes)
{
    KisGroupLayer *group = dynamic_cast<KisGroupLayer *>(root.data());
    if (!group)
        return;

    KoColor projectionColor = group->defaultProjectionColor();
    if (projectionColor.opacityU8() == OPACITY_TRANSPARENT_U8)
        return;

    KisPaintLayerSP layer = new KisPaintLayer(group->image(), i18nc("Automatically created layer name when saving into PSD", "Background"), OPACITY_OPAQUE_U8);

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
        const bool isLayer = child->inherits("KisLayer");
        const bool isGroupLayer = child->inherits("KisGroupLayer");

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
        } else if (isLayer) {
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
    if (type != FlattenedNode::FOLDER_OPEN && type != FlattenedNode::FOLDER_CLOSED && type != FlattenedNode::RASTER_LAYER) {
        return 0;
    }

    KisLayer *layer = qobject_cast<KisLayer *>(node.data());
    QList<KisEffectMaskSP> masks = layer->effectMasks();

    if (masks.size() != 1)
        return 0;

    KisEffectMaskSP onlyMask = masks.first();
    return onlyMask->inherits("KisTransparencyMask") ? onlyMask : 0;
}

QDomDocument fetchLayerStyleXmlData(KisNodeSP node)
{
    const KisLayer *layer = qobject_cast<KisLayer *>(node.data());
    KisPSDLayerStyleSP layerStyle = layer->layerStyle();

    if (!layerStyle)
        return QDomDocument();

    KisAslLayerStyleSerializer serializer;
    serializer.setStyles(QVector<KisPSDLayerStyleSP>() << layerStyle);
    return serializer.formPsdXmlDocument();
}

inline QDomNode findNodeByKey(const QString &key, QDomNode parent)
{
    return KisDomUtils::findElementByAttribute(parent, "node", "key", key);
}

void mergePatternsXMLSection(const QDomDocument &src, QDomDocument &dst)
{
    QDomNode srcPatternsNode = findNodeByKey(ResourceType::Patterns, src.documentElement());
    QDomNode dstPatternsNode = findNodeByKey(ResourceType::Patterns, dst.documentElement());

    if (srcPatternsNode.isNull())
        return;
    if (dstPatternsNode.isNull()) {
        dst = src;
        return;
    }

    KIS_ASSERT_RECOVER_RETURN(!srcPatternsNode.isNull());
    KIS_ASSERT_RECOVER_RETURN(!dstPatternsNode.isNull());

    QDomNode node = srcPatternsNode.firstChild();
    while (!node.isNull()) {
        QDomNode importedNode = dst.importNode(node, true);
        KIS_ASSERT_RECOVER_RETURN(!importedNode.isNull());

        dstPatternsNode.appendChild(importedNode);
        node = node.nextSibling();
    }
}

bool PSDLayerMaskSection::write(QIODevice &io, KisNodeSP rootLayer, psd_compression_type compressionType)
{
    bool retval = true;

    try {
        if (m_header.tiffStyleLayerBlock) {
            switch (m_header.byteOrder) {
            case psd_byte_order::psdLittleEndian:
                writeTiffImpl<psd_byte_order::psdLittleEndian>(io, rootLayer, compressionType);
                break;
            default:
                writeTiffImpl(io, rootLayer, compressionType);
                break;
            }
        } else {
            writePsdImpl(io, rootLayer, compressionType);
        }
    } catch (KisAslWriterUtils::ASLWriteException &e) {
        error = PREPEND_METHOD(e.what());
        retval = false;
    }

    return retval;
}

void PSDLayerMaskSection::writePsdImpl(QIODevice &io, KisNodeSP rootLayer, psd_compression_type compressionType)
{
    dbgFile << "Writing layer section";

    globalInfoSection.txt2Data = KisTxt2Utils::defaultTxt2();
    int textCount = 0;

    // Build the whole layer structure
    QList<FlattenedNode> nodes;
    addBackgroundIfNeeded(rootLayer, nodes);
    flattenNodes(rootLayer, nodes);

    if (nodes.isEmpty()) {
        throw KisAslWriterUtils::ASLWriteException("Could not find paint layers to save");
    }

    {
        KisAslWriterUtils::OffsetStreamPusher<quint32, psd_byte_order::psdBigEndian> layerAndMaskSectionSizeTag(io, 2);
        QDomDocument mergedPatternsXmlDoc;

        {
            KisAslWriterUtils::OffsetStreamPusher<quint32, psd_byte_order::psdBigEndian> layerInfoSizeTag(io, 2);

            {
                // number of layers (negative, because krita always has alpha)
                const qint16 layersSize = static_cast<qint16>(-nodes.size());
                SAFE_WRITE_EX(psd_byte_order::psdBigEndian, io, layersSize);

                dbgFile << "Number of layers" << layersSize << "at" << io.pos();
            }

            // Layer records section
            Q_FOREACH (const FlattenedNode &item, nodes) {
                KisNodeSP node = item.node;

                PSDLayerRecord *layerRecord = new PSDLayerRecord(m_header);
                layers.append(layerRecord);

                KisNodeSP onlyTransparencyMask = findOnlyTransparencyMask(node, item.type);
                QRect maskRect = onlyTransparencyMask ? onlyTransparencyMask->paintDevice()->exactBounds() : QRect();

                const bool nodeVisible = node->visible();
                const KoColorSpace *colorSpace = node->colorSpace();
                const quint8 nodeOpacity = node->opacity();
                const quint8 nodeClipping = 0;
                const int nodeLabelColor = node->colorLabelIndex();
                const KisPaintLayer *paintLayer = qobject_cast<KisPaintLayer *>(node.data());
                const bool alphaLocked = (paintLayer && paintLayer->alphaLocked());
                const QString nodeCompositeOp = node->compositeOpId();

                const KisGroupLayer *groupLayer = qobject_cast<KisGroupLayer *>(node.data());
                const bool nodeIsPassThrough = groupLayer && groupLayer->passThroughMode();

                const KisGeneratorLayer *fillLayer = qobject_cast<KisGeneratorLayer *>(node.data());
                QDomDocument fillConfig;
                psd_fill_type fillType = psd_fill_solid_color;
                if (fillLayer) {
                    QString generatorName = fillLayer->filter()->name();
                    if (generatorName == "color") {
                        psd_layer_solid_color fill;
                        if (fill.loadFromConfig(fillLayer->filter())) {
                            if (node->image()) {
                                fill.cs = node->image()->colorSpace();
                            } else {
                                fill.cs = node->colorSpace();
                            }
                            fillConfig = fill.getASLXML();
                            fillType = psd_fill_solid_color;
                        }
                    } else if (generatorName == "gradient") {
                        psd_layer_gradient_fill fill;
                        fill.imageWidth = node->image()->width();
                        fill.imageHeight = node->image()->height();
                        if (fill.loadFromConfig(fillLayer->filter())) {
                            fillConfig = fill.getASLXML();
                            fillType = psd_fill_gradient;
                        }
                    } else if (generatorName == "pattern") {

                        psd_layer_pattern_fill fill;
                        if (fill.loadFromConfig(fillLayer->filter())) {
                            if (fill.pattern) {
                                KisAslXmlWriter w;
                                w.enterList(ResourceType::Patterns);
                                QString uuid = w.writePattern("", fill.pattern);
                                w.leaveList();
                                mergedPatternsXmlDoc = w.document();
                                fill.patternID = uuid;
                                fillConfig = fill.getASLXML();
                                fillType = psd_fill_pattern;
                            }
                        }

                    }
                    // And if anything else, it cannot be stored as a PSD fill layer.
                }

                double vectorWidth = rootLayer->image()? rootLayer->image()->width() / rootLayer->image()->xRes(): 1;
                double vectorHeight = rootLayer->image()? rootLayer->image()->height() / rootLayer->image()->yRes(): 1;
                QTransform FlaketoPixels = QTransform::fromScale(rootLayer->image()->xRes(), rootLayer->image()->yRes());

                const KisShapeLayer *shapeLayer = qobject_cast<KisShapeLayer*>(node.data());
                psd_layer_type_shape textData;
                psd_vector_mask vectorMask;
                QDomDocument strokeData;

                if (shapeLayer && !shapeLayer->isFakeNode()) {
                    // only store the first shape.
                    if (shapeLayer->shapes().size() == 1) {
                        KoSvgTextShape * text = dynamic_cast<KoSvgTextShape*>(shapeLayer->shapes().first());
                        if (text) {
                            KoSvgTextShapeMarkupConverter convert(text);
                            QString svgtext;
                            QString styles;
                            convert.convertToSvg(&svgtext, &styles);
                            // unsure about the boundingBox, needs more research.
                            textData.boundingBox = text->boundingRect().normalized();
                            textData.bounds = text->outlineRect().normalized();

                            convert.convertToPSDTextEngineData(svgtext, textData.bounds, text->shapesInside(), globalInfoSection.txt2Data, textData.textIndex, textData.text, textData.isHorizontal, FlaketoPixels);
                            textData.engineData = KisTxt2Utils::tyShFromTxt2(globalInfoSection.txt2Data, FlaketoPixels.mapRect(textData.boundingBox), textData.textIndex);
                            textCount += 1;
                            if (!text->shapesInside().isEmpty()) {
                                textData.bounds = text->outlineRect().normalized();
                            }
                            if (!textData.bounds.isEmpty()) {
                                textData.boundingBox = FlaketoPixels.mapRect(textData.boundingBox);
                                textData.bounds = FlaketoPixels.mapRect(textData.bounds);
                            } else {
                                textData.boundingBox = QRectF();
                            }
                            textData.transform = FlaketoPixels.inverted() * text->absoluteTransformation() * FlaketoPixels;
                        } else {
                            KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shapeLayer->shapes().first());
                            if (pathShape){
                                layerRecord->addPathShapeToPSDPath(vectorMask.path, pathShape, vectorWidth, vectorHeight);
                                KoColorBackground *b = dynamic_cast<KoColorBackground *>(pathShape->background().data());
                                KoGradientBackground *g = dynamic_cast<KoGradientBackground *>(pathShape->background().data());
                                KoPatternBackground *p = dynamic_cast<KoPatternBackground *>(pathShape->background().data());
                                if (b) {
                                    psd_layer_solid_color fill;

                                    if (node->image()) {
                                        fill.cs = node->image()->colorSpace();
                                    } else {
                                        fill.cs = node->colorSpace();
                                    }
                                    fill.setColor(KoColor(b->color(), fill.cs));
                                    fillConfig = fill.getASLXML();
                                    fillType = psd_fill_solid_color;
                                } else if (g) {
                                    psd_layer_gradient_fill fill;
                                    fill.setFromQGradient(g->gradient());
                                    fillConfig = fill.getASLXML();
                                    fillType = psd_fill_gradient;
                                } else if (p) {
                                    psd_layer_pattern_fill fill;
                                    fillConfig = fill.getASLXML();
                                    fillType = psd_fill_pattern;
                                } else if (!pathShape->background()) {
                                    psd_layer_solid_color fill;

                                    if (node->image()) {
                                        fill.cs = node->image()->colorSpace();
                                    } else {
                                        fill.cs = node->colorSpace();
                                    }
                                    fill.setColor(KoColor(Qt::transparent, fill.cs));
                                    fillConfig = fill.getASLXML();
                                    fillType = psd_fill_solid_color;
                                }
                                KoShapeStrokeSP shapeStroke = qSharedPointerDynamicCast<KoShapeStroke>(pathShape->stroke());
                                if (shapeStroke) {
                                    psd_vector_stroke_data strokeDataStruct;
                                    strokeDataStruct.loadFromShapeStroke(shapeStroke);
                                    strokeDataStruct.strokeEnabled = true;
                                    strokeDataStruct.fillEnabled = pathShape->background()? true: false;
                                    strokeDataStruct.resolution = node->image()->xRes()*72.0;
                                    strokeData = strokeDataStruct.getASLXML();
                                }
                            }
                        }
                    }
                }

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

                    /**
                     * For fill layers we save their internal selection as a separate transparency mask
                     */
                    if (fillLayer) {
                        bool transparency = KisPainter::checkDeviceHasTransparency(node->paintDevice());
                        bool semiOpacity = node->paintDevice()->defaultPixel().opacityU8() < OPACITY_OPAQUE_U8;
                        if (transparency || semiOpacity) {
                            KisSelectionSP selection = fillLayer->internalSelection();
                            if(selection) {
                                if(selection->hasNonEmptyShapeSelection()) {
                                    KisShapeSelection* shapeSelection = dynamic_cast<KisShapeSelection*>(selection->shapeSelection());
                                    if (shapeSelection) {
                                        Q_FOREACH(KoShape *shape, shapeSelection->shapes()) {
                                            KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape);
                                            if (pathShape){
                                                layerRecord->addPathShapeToPSDPath(vectorMask.path, pathShape, vectorWidth, vectorHeight);

                                            }
                                        }
                                    }
                                }
                            }
                            layerContentDevice = node->original();
                            onlyTransparencyMask = node;
                            maskRect = onlyTransparencyMask->paintDevice()->exactBounds();
                        }
                    } else {
                        KisTransparencyMask *mask = qobject_cast<KisTransparencyMask*>(onlyTransparencyMask.data());
                        if (mask) {
                        KisSelectionSP selection = mask->selection();
                        if(selection) {
                            if(selection->hasNonEmptyShapeSelection()) {
                                KisShapeSelection* shapeSelection = dynamic_cast<KisShapeSelection*>(selection->shapeSelection());
                                if (shapeSelection) {
                                    Q_FOREACH(KoShape *shape, shapeSelection->shapes()) {
                                        KoPathShape *pathShape = dynamic_cast<KoPathShape*>(shape);
                                        if (pathShape){
                                            layerRecord->addPathShapeToPSDPath(vectorMask.path, pathShape, vectorWidth, vectorHeight);
                                        }
                                    }
                                }
                            }
                        }
                        }
                    }
                    sectionType = psd_other;
                } else {
                    nodeIrrelevant = true;
                    nodeName = item.type == FlattenedNode::SECTION_DIVIDER ? QString("</Layer group>") : node->name();
                    layerContentDevice = 0;
                    sectionType = item.type == FlattenedNode::SECTION_DIVIDER ? psd_bounding_divider
                        : item.type == FlattenedNode::FOLDER_OPEN             ? psd_open_folder
                                                                              : psd_closed_folder;
                }

                // === no access to node anymore

                QRect layerRect;

                if (layerContentDevice) {
                    QRect rc = layerContentDevice->exactBounds();
                    rc = rc.normalized();

                    // keep to the max of photoshop's capabilities
                    if (rc.width() > 30000)
                        rc.setWidth(30000);
                    if (rc.height() > 30000)
                        rc.setHeight(30000);

                    layerRect = rc;
                }

                layerRecord->top = layerRect.y();
                layerRecord->left = layerRect.x();
                layerRecord->bottom = layerRect.y() + layerRect.height();
                layerRecord->right = layerRect.x() + layerRect.width();

                // colors + alpha channel
                // note: transparency mask not included
                layerRecord->nChannels = static_cast<quint16>(colorSpace->colorChannelCount() + 1);

                ChannelInfo *info = new ChannelInfo;
                info->channelId = -1; // For the alpha channel, which we always have in Krita, and should be saved first in
                layerRecord->channelInfoRecords << info;

                // the rest is in display order: rgb, cmyk, lab...
                for (qint16 i = 0; i < (int)colorSpace->colorChannelCount(); ++i) {
                    info = new ChannelInfo;
                    info->channelId = i; // 0 for red, 1 = green, etc
                    layerRecord->channelInfoRecords << info;
                }

                layerRecord->blendModeKey = composite_op_to_psd_blendmode(nodeCompositeOp);
                layerRecord->isPassThrough = nodeIsPassThrough;
                layerRecord->opacity = nodeOpacity;
                layerRecord->clipping = nodeClipping;

                layerRecord->labelColor = nodeLabelColor;

                layerRecord->transparencyProtected = alphaLocked;
                layerRecord->visible = nodeVisible;
                layerRecord->irrelevant = nodeIrrelevant;

                layerRecord->layerName = nodeName.isEmpty() ? i18n("Unnamed Layer") : nodeName;

                layerRecord->fillType = fillType;
                layerRecord->fillConfig = fillConfig;

                layerRecord->vectorMask = vectorMask;
                layerRecord->vectorStroke = strokeData;

                layerRecord->textShape = textData;

                layerRecord->write(io, layerContentDevice, onlyTransparencyMask, maskRect, sectionType, stylesXmlDoc, node->inherits("KisGroupLayer"));
            }

            dbgFile << "start writing layer pixel data" << io.pos();

            // Now save the pixel data
            for (PSDLayerRecord *layerRecord : layers) {
                layerRecord->writePixelData(io, compressionType);
            }
        }

        {
            // write the global layer mask info -- which is empty
            const quint32 globalMaskSize = 0;
            SAFE_WRITE_EX(psd_byte_order::psdBigEndian, io, globalMaskSize);
        }

        globalInfoSection.writePattBlockEx(io, mergedPatternsXmlDoc);

#if 0
        /**
         * We're currently not writing the Txt2 data itself as it doesn't
         * result in correct PSDs. There's three possible culprits for this:
         *
         * 1. PSD perhaps requires the data to be stored in a specific order.
         *    The 'uncompressKeys' function in kis_txt2_utls gives an indication of this order.
         * 2. PSD requires the Strikes for each text object to be written.
         *    This is the most likely cause. The strikes object however consists of data for
         *    every line, segment, and character, with positioning, bounding boxes and even
         *    precise font glyph indices for each character. This is more or less a cached
         *    version of the layout data of the text shape, and we don't have that kind of access
         *    of the text shape data right now.
         * 3. Something else. The Txt2 data is huge and therefore it is hard to figure out
         *    where things might be going wrong.
         *
         * In practice, this means Krita won't be able to store OpenType feature data as well
         * as path shapes for either text-in-shape or text-on-path.
         */
        if (textCount > 0) {
            globalInfoSection.writeTxt2BlockEx(io, globalInfoSection.txt2Data);
        }
#endif
    }
}

template<psd_byte_order byteOrder>
void PSDLayerMaskSection::writeTiffImpl(QIODevice &io, KisNodeSP rootLayer, psd_compression_type compressionType)
{
    dbgFile << "(TIFF) Writing layer section";

    // Build the whole layer structure
    QList<FlattenedNode> nodes;
    addBackgroundIfNeeded(rootLayer, nodes);
    flattenNodes(rootLayer, nodes);

    if (nodes.isEmpty()) {
        throw KisAslWriterUtils::ASLWriteException("Could not find paint layers to save");
    }

    {
        QDomDocument mergedPatternsXmlDoc;

        {
            KisAslWriterUtils::writeFixedString<byteOrder>("8BIM", io);
            KisAslWriterUtils::writeFixedString<byteOrder>("Layr", io);

            KisAslWriterUtils::OffsetStreamPusher<quint32, byteOrder> layerAndMaskSectionSizeTag(io, 4);
            // number of layers (negative, because krita always has alpha)
            const qint16 layersSize = nodes.size();
            SAFE_WRITE_EX(byteOrder, io, layersSize);

            dbgFile << "Number of layers" << layersSize << "at" << io.pos();

            // Layer records section
            for (const FlattenedNode &item : nodes) {
                KisNodeSP node = item.node;

                PSDLayerRecord *layerRecord = new PSDLayerRecord(m_header);
                layers.append(layerRecord);

                const bool nodeVisible = node->visible();
                const KoColorSpace *colorSpace = node->colorSpace();
                const quint8 nodeOpacity = node->opacity();
                const quint8 nodeClipping = 0;
                const int nodeLabelColor = node->colorLabelIndex();
                const KisPaintLayer *paintLayer = qobject_cast<KisPaintLayer *>(node.data());
                const bool alphaLocked = (paintLayer && paintLayer->alphaLocked());
                const QString nodeCompositeOp = node->compositeOpId();

                const KisGroupLayer *groupLayer = qobject_cast<KisGroupLayer *>(node.data());
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
                    layerContentDevice = node->projection();
                    sectionType = psd_other;
                } else {
                    nodeIrrelevant = true;
                    nodeName = item.type == FlattenedNode::SECTION_DIVIDER ? QString("</Layer group>") : node->name();
                    layerContentDevice = 0;
                    sectionType = item.type == FlattenedNode::SECTION_DIVIDER ? psd_bounding_divider
                        : item.type == FlattenedNode::FOLDER_OPEN             ? psd_open_folder
                                                                              : psd_closed_folder;
                }

                // === no access to node anymore

                QRect layerRect;

                if (layerContentDevice) {
                    QRect rc = layerContentDevice->exactBounds();
                    rc = rc.normalized();

                    // keep to the max of photoshop's capabilities
                    // XXX: update this to PSB
                    if (rc.width() > 30000)
                        rc.setWidth(30000);
                    if (rc.height() > 30000)
                        rc.setHeight(30000);

                    layerRect = rc;
                }

                layerRecord->top = layerRect.y();
                layerRecord->left = layerRect.x();
                layerRecord->bottom = layerRect.y() + layerRect.height();
                layerRecord->right = layerRect.x() + layerRect.width();

                // colors + alpha channel
                // note: transparency mask not included
                layerRecord->nChannels = static_cast<quint16>(colorSpace->colorChannelCount() + 1);

                ChannelInfo *info = new ChannelInfo;
                info->channelId = -1; // For the alpha channel, which we always have in Krita, and should be saved first in
                layerRecord->channelInfoRecords << info;

                // the rest is in display order: rgb, cmyk, lab...
                for (quint32 i = 0; i < colorSpace->colorChannelCount(); ++i) {
                    info = new ChannelInfo;
                    info->channelId = static_cast<qint16>(i); // 0 for red, 1 = green, etc
                    layerRecord->channelInfoRecords << info;
                }

                layerRecord->blendModeKey = composite_op_to_psd_blendmode(nodeCompositeOp);
                layerRecord->isPassThrough = nodeIsPassThrough;
                layerRecord->opacity = nodeOpacity;
                layerRecord->clipping = nodeClipping;

                layerRecord->transparencyProtected = alphaLocked;
                layerRecord->visible = nodeVisible;
                layerRecord->irrelevant = nodeIrrelevant;
                layerRecord->labelColor = nodeLabelColor;

                layerRecord->layerName = nodeName.isEmpty() ? i18n("Unnamed Layer") : nodeName;

                layerRecord->write(io, layerContentDevice, nullptr, QRect(), sectionType, stylesXmlDoc, node->inherits("KisGroupLayer"));
            }

            dbgFile << "start writing layer pixel data" << io.pos();

            // Now save the pixel data
            for (PSDLayerRecord *layerRecord : layers) {
                layerRecord->writePixelData(io, compressionType);
            }
        }

        // {
        //     // write the global layer mask info -- which is NOT empty but fixed
        //     KisAslWriterUtils::writeFixedString<byteOrder>("8BIM", io);
        //     KisAslWriterUtils::writeFixedString<byteOrder>("LMsk", io);

        //     KisAslWriterUtils::OffsetStreamPusher<quint32, byteOrder> layerAndMaskSectionSizeTag(io, 4);
        //     // https://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577411_22664
        //     psdwrite<byteOrder>(io, quint16(0));     // CS: RGB
        //     psdwrite<byteOrder>(io, quint16(65535)); // Pure red verification
        //     psdwrite<byteOrder>(io, quint16(0));
        //     psdwrite<byteOrder>(io, quint16(0));
        //     psdwrite<byteOrder>(io, quint16(0));
        //     psdwrite<byteOrder>(io, quint16(50)); // opacity
        //     psdwrite<byteOrder>(io, quint16(128)); // kind
        // }

        globalInfoSection.writePattBlockEx(io, mergedPatternsXmlDoc);
    }
}
