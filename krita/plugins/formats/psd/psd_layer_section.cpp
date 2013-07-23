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

#include <kis_debug.h>
#include <kis_node.h>
#include <kis_paint_layer.h>

#include "psd_header.h"
#include "psd_utils.h"

#include "compression.h"

PSDLayerSection::PSDLayerSection(const PSDHeader& header)
    : m_header(header)
{
    hasTransparency = false;
    layerMaskBlockSize = 0;
    layerInfoSize = 0;
    nLayers = 0;
}

PSDLayerSection::~PSDLayerSection()
{
    qDeleteAll(layers);
}

bool PSDLayerSection::read(QIODevice* io)
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

    dbgFile << "reading layer info block. Bytes left" << io->bytesAvailable() << "position" << io->pos();
    if (m_header.version == 1) {
        quint32 _layerInfoSize;
        if (!psdread(io, &_layerInfoSize) || _layerInfoSize > (quint64)io->bytesAvailable()) {
            error = "Could not read layer section size";
            return false;
        }
        layerInfoSize = _layerInfoSize;
    }
    else if (m_header.version == 2) {
        if (!psdread(io, &layerInfoSize) || layerInfoSize > (quint64)io->bytesAvailable()) {
            error = "Could not read layer section size";
            return false;
        }
    }
    dbgFile << "Layer info block size" << layerInfoSize;


    if (layerInfoSize > 0 ) {

        // rounded to a multiple of 2
        if ((layerInfoSize & 0x01) != 0) {
            layerInfoSize++;
        }
        dbgFile << "Layer info block size after rounding" << layerInfoSize;

        if (!psdread(io, &nLayers) || nLayers == 0) {
            error = QString("Could not read read number of layers or no layers in image. %1").arg(nLayers);
            return false;
        }

        if (nLayers < 0) {
            hasTransparency = true; // first alpha channel is the alpha channel of the projection.
            nLayers = -nLayers;
        }
        else {
            hasTransparency = false;
        }
        dbgFile << "transparency" << hasTransparency;

        dbgFile << "Number of layers" << nLayers << "transparency" << hasTransparency;

        for (int i = 0; i < nLayers; ++i) {

            dbgFile << "Going to read layer " << i << "pos" << io->pos();
            PSDLayerRecord *layerRecord = new PSDLayerRecord(m_header);
            if (!layerRecord->read(io)) {
                error = QString("Could not load layer %1: %2").arg(i).arg(layerRecord->error);
                return false;
            }
            dbgFile << "Read layer" << i << layerRecord->layerName << "blending mode"
                    << layerRecord->blendModeKey << io->pos()
                    << "Number of channels:" <<  layerRecord->channelInfoRecords.size();
            layers << layerRecord;
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

            Q_ASSERT(j < layerRecord->channelInfoRecords.size());
            if (j > layerRecord->channelInfoRecords.size()) {
                error = QString("Expected channel %1, but only have %2 channels for layer %3")
                        .arg(j)
                        .arg(layerRecord->channelInfoRecords.size())
                        .arg(i);
                return false;
            }

            ChannelInfo* channelInfo = layerRecord->channelInfoRecords.at(j);

            quint16 compressionType;
            if (!psdread(io, &compressionType)) {
                error = "Could not read compression type for channel";
                return false;
            }
            channelInfo->compressionType = (Compression::CompressionType)compressionType;
            dbgFile << "\t\tChannel" << j << "has compression type" << compressionType;

            // read the rle row lengths;
            if (channelInfo->compressionType == Compression::RLE) {
                for(qint64 row = 0; row < (layerRecord->bottom - layerRecord->top); ++row) {

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
                    << "starting postion" << channelInfo->channelDataStart
                    << "with length" << channelInfo->channelDataLength
                    << "and has compression type" << channelInfo->compressionType;

        }
    }

    quint32 globalMaskBlockLength;
    if (!psdread(io, &globalMaskBlockLength)) {
        error = "Could not read global mask info block";
        return false;
    }

    if (globalMaskBlockLength > 0) {

        if (!psdread(io, &overlayColorSpace)) {
            error = "Could not read global mask info overlay colorspace";
            return false;
        }

        for (int i = 0; i < 4; ++i) {
            if (!psdread(io, &colorComponents[i])) {
                error = QString("Could not read mask info visualizaion color component %1").arg(i);
                return false;
            }
        }

        if (!psdread(io, &opacity)) {
            error = "Could not read global mask info visualization opacity";
            return false;
        }

        if (!psdread(io, &kind)) {
            error = "Could not read global mask info visualization type";
            return false;
        }
    }

    /* put us after this section so reading the next section will work even if we mess up */
    io->seek(start + layerMaskBlockSize);

    return valid();
}

void flattenLayers(KisNodeSP node, QList<KisNodeSP> &layers)
{
    for (uint i = 0; i < node->childCount(); ++i) {
        KisNodeSP child = node->at(i);
        if (child->inherits("KisPaintLayer")) {
            layers << child;
        }
        if (child->childCount() > 0) {
            flattenLayers(child, layers);
        }
    }
    dbgFile << layers.size();
}

bool PSDLayerSection::write(QIODevice* io, KisNodeSP rootLayer)
{
    dbgFile << "Writing layer layer section";

    // Build the whole layer structure
    QList<KisNodeSP> nodes;
    flattenLayers(rootLayer, nodes);

    if (nodes.isEmpty()) {
        error = "Could not find paint layers to save";
        return false;
    }

    quint64 layerMaskPos = io->pos();
    // length of the layer info and mask information section
    dbgFile << "Length of layer info and mask info section at" << layerMaskPos;
    psdwrite(io, (quint32)0);

    quint64 layerInfoPos = io->pos();
    dbgFile << "length of the layer info section, rounded up to a multiple of two, at" << layerInfoPos;
    psdwrite(io, (quint32)0);

    // number of layers (negative, because krita always has alpha)
    dbgFile << "number of layers" << -nodes.size() << "at" << io->pos();
    psdwrite(io, (qint16)-nodes.size());

    // Layer records section
    foreach(KisNodeSP node, nodes) {
        PSDLayerRecord *layerRecord = new PSDLayerRecord(m_header);
        layers.append(layerRecord);

        QRect rc = node->projection()->extent();
        // empty layers
        if (rc.width() == 0 || rc.height() == 0) {
            rc = QRect(0, 0, 64, 64);
        }

        rc = rc.normalized();

        Q_ASSERT(rc.top() <= rc.bottom());
        Q_ASSERT(rc.left() <= rc.right());
        // keep to the max of photoshop's capabilities
        if (rc.width() > 30000) rc.setWidth(30000);
        if (rc.height() > 30000) rc.setHeight(30000);
        layerRecord->top = rc.y();
        layerRecord->left = rc.x();
        layerRecord->bottom = rc.y() + rc.height();
        layerRecord->right = rc.x() + rc.width();
        layerRecord->nChannels = node->projection()->colorSpace()->colorChannelCount();

        // XXX: masks should be saved as channels as well, with id -2
        ChannelInfo *info = new ChannelInfo;
        info->channelId = -1; // For the alpha channel, which we always have in Krita, and should be saved first in
        layerRecord->channelInfoRecords << info;

        // the rest is in display order: rgb, cmyk, lab...
        for (int i = 0; i < layerRecord->nChannels; ++i) {
            info = new ChannelInfo;
            info->channelId = i; // 0 for red, 1 = green, etc
            layerRecord->channelInfoRecords << info;
        }
        layerRecord->nChannels++; // to compensate for the alpha channel at the start

        layerRecord->blendModeKey = composite_op_to_psd_blendmode(node->compositeOpId());
        layerRecord->opacity = node->opacity();
        layerRecord->clipping = 0;

        KisPaintLayer *paintLayer = qobject_cast<KisPaintLayer*>(node.data());
        layerRecord->transparencyProtected = (paintLayer && paintLayer->alphaLocked());
        layerRecord->visible = node->visible();

        layerRecord->layerName = node->name();

        if (!layerRecord->write(io, node)) {
            error = layerRecord->error;
            return false;
        }
    }

    // Now save the pixel data
    dbgFile << "start writing layer pixel data" << io->pos();
    foreach(PSDLayerRecord *layerRecord, layers) {
        if (!layerRecord->writePixelData(io)) {
            error = layerRecord->error;
            return false;
        }
    }

    // Write the final size of the block
    dbgFile << "Final io pos after writing layer pixel data" << io->pos();
    quint64 eof_pos = io->pos();

    io->seek(layerInfoPos);

    // length of the layer info information section
    quint32 layerInfoSize = eof_pos - layerInfoPos - sizeof(qint32);
    dbgFile << "Layer Info Section length" << layerInfoSize << "at"  << io->pos();
    psdwrite(io, layerInfoSize);

    io->seek(eof_pos);

    // Write the global layer mask info -- which is empty
    psdwrite(io, (quint32)0);

    // Write the final size of the block
    dbgFile << "Final io pos after writing layer pixel data" << io->pos();
    eof_pos = io->pos();

    io->seek(layerInfoPos);

    // length of the layer and mask info section, rounded up to a multiple of two
    io->seek(layerMaskPos);
    quint32 layerMaskSize = eof_pos - layerMaskPos - sizeof(qint32);
    dbgFile << "Layer and Mask information length" << layerMaskSize << "at" << io->pos();
    psdwrite(io, layerMaskSize);

    io->seek(eof_pos);

    return true;
}

bool PSDLayerSection::valid()
{
    if (layerInfoSize > 0) {
        if (nLayers <= 0) return false;
        if (nLayers != layers.size()) return false;
        foreach(PSDLayerRecord* layer, layers) {
            if (!layer->valid()) {
                return false;
            }
        }
    }
    if (!error.isNull()) {
        return false;
    }
    return true;
}
