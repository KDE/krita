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
#include "psd_layer_record.h"

#include <netinet/in.h> // htonl

#include <QIODevice>
#include <QBuffer>
#include <QDataStream>
#include <QStringList>

#include <kis_debug.h>

#include "psd_utils.h"
#include "psd_header.h"
#include "compression.h"

QString channelIdToChannelType(int channelId, PSDColorMode colormode)
{
    switch(channelId) {
    case -2:
        return "User Supplied Layer Mask";
    case -1:
        return "Transparency mask";
    case  0:
        switch(colormode) {
        case Bitmap:
        case Indexed:
            return QString("bitmap or indexed: %1").arg(channelId);
        case Grayscale:
        case Gray16:
            return "gray";
        case RGB:
        case RGB48:
            return "red";
        case Lab:
        case Lab48:
            return "L";
        case CMYK:
        case CMYK64:
            return "cyan";
        case MultiChannel:
        case DeepMultichannel:
            return QString("multichannel channel %1").arg(channelId);
        case DuoTone:
        case Duotone16:
            return QString("duotone channel %1").arg(channelId);
        default:
            return QString("unknown: %1").arg(channelId);
        };
    case 1:
        switch(colormode) {
        case Bitmap:
        case Indexed:
            return QString("WARNING bitmap or indexed: %1").arg(channelId);
        case Grayscale:
        case Gray16:
            return QString("WARNING: %1").arg(channelId);
        case RGB:
        case RGB48:
            return "green";
        case Lab:
        case Lab48:
            return "a";
        case CMYK:
        case CMYK64:
            return "Magenta";
        case MultiChannel:
        case DeepMultichannel:
            return QString("multichannel channel %1").arg(channelId);
        case DuoTone:
        case Duotone16:
            return QString("duotone channel %1").arg(channelId);
        default:
            return QString("unknown: %1").arg(channelId);
        };
    case 2:
        switch(colormode) {
        case Bitmap:
        case Indexed:
            return QString("WARNING bitmap or indexed: %1").arg(channelId);
        case Grayscale:
        case Gray16:
            return QString("WARNING: %1").arg(channelId);
        case RGB:
        case RGB48:
            return "blue";
        case Lab:
        case Lab48:
            return "b";
        case CMYK:
        case CMYK64:
            return "yellow";
        case MultiChannel:
        case DeepMultichannel:
            return QString("multichannel channel %1").arg(channelId);
        case DuoTone:
        case Duotone16:
            return QString("duotone channel %1").arg(channelId);
        default:
            return QString("unknown: %1").arg(channelId);
        };
    case 3:
        switch(colormode) {
        case Bitmap:
        case Indexed:
            return QString("WARNING bitmap or indexed: %1").arg(channelId);
        case Grayscale:
        case Gray16:
            return QString("WARNING: %1").arg(channelId);
        case RGB:
        case RGB48:
            return QString("WARNING: %1").arg(channelId);
        case Lab:
        case Lab48:
            return QString("WARNING: %1").arg(channelId);
        case CMYK:
        case CMYK64:
            return "Key";
        case MultiChannel:
        case DeepMultichannel:
            return QString("multichannel channel %1").arg(channelId);
        case DuoTone:
        case Duotone16:
            return QString("duotone channel %1").arg(channelId);
        default:
            return QString("unknown: %1").arg(channelId);
        };
    default:
        return QString("unknown: %1").arg(channelId);
    };

}

PSDLayerRecord::PSDLayerRecord(const PSDHeader& header)
    : m_header(header)
{
}

bool PSDLayerRecord::read(QIODevice* io)
{
    dbgFile << "Going to read layer record. Pos:" << io->pos();

    if (!psdread(io, &top)  ||
        !psdread(io, &left) ||
        !psdread(io, &bottom) ||
        !psdread(io, &right) ||
        !psdread(io, &nChannels)) {

        error = "could not read layer record";
        return false;
    }

    dbgFile << "\ttop" << top << "left" << left << "bottom" << bottom << "right" << right << "number of channels" << nChannels;

    switch(m_header.colormode) {
    case(Bitmap):
    case(Indexed):
    case(DuoTone):
    case(Grayscale):
    case(MultiChannel):
        if (nChannels < 1) {
            error = QString("Not enough channels. Got: %1").arg(nChannels);
            return false;
        }
        break;
    case(RGB):
    case(CMYK):
    case(Lab):
    default:
        if (nChannels < 3) {
            error = QString("Not enough channels. Got: %1").arg(nChannels);
            return false;
        }
        break;
    };

    if (nChannels > MAX_CHANNELS) {
        error = QString("Too many channels. Got: %1").arg(nChannels);
        return false;
    }

    for (int i = 0; i < nChannels; ++i) {

        if (io->atEnd()) {
            error = "Could not read enough data for channels";
            return false;
        }

        ChannelInfo* info = new ChannelInfo();
        info->compressionType = Compression::Unknown;
        info->channelId = -1;
        info->channelDataLength = 0;
        info->channelDataStart = 0;

        if (!psdread(io, &info->channelId)) {
            error = "could not read channel id";
	    delete info;
            return false;
        }
        bool r;
        if (m_header.version == 1) {
            quint32 channelDataLength;
            r = psdread(io, &channelDataLength);
            info->channelDataLength = (quint64)channelDataLength;
        }
        else {
            r = psdread(io, &info->channelDataLength);
        }
        if (!r) {
            error = "Could not read length for channel data";
	    delete info;
            return false;
        }

        dbgFile << "\tchannel" << i << "id" << channelIdToChannelType(info->channelId, m_header.colormode) << "length" << info->channelDataLength;
        channelInfoRecords << info;
    }

    QByteArray b;
    b = io->read(4);
    if(b.size() != 4 || QString(b) != "8BIM") {
        error = QString("Could not read blend mode signature for layer. Got %1.")
                .arg(QString(b));
        return false;
    }

    blendModeKey = QString(io->read(4));
    if (blendModeKey.size() != 4) {
        error = QString("Could not read blend mode key. Got: %1").arg(blendModeKey);
        return false;
    }

    dbgFile << "\tBlend mode" << blendModeKey << "pos" << io->pos();

    if (!psdread(io, &opacity)) {
        error = "Could not read opacity";
        return false;
    }

    dbgFile << "\tOpacity" << opacity << io->pos();

    if (!psdread(io, &clipping)) {
        error = "Could not read clipping";
        return false;
    }

    dbgFile << "\tclipping" << clipping << io->pos();

    quint8 flags;
    if (!psdread(io, &flags)) {
        error = "Could not read flags";
        return false;
    }
    dbgFile << "\tflags" << flags << io->pos();

    transparencyProtected = flags & 1 ? true : false;

    dbgFile << "\ttransparency protected" << transparencyProtected;

    visible = flags & 2 ? true : false;

    dbgFile << "\tvisible" << visible;

    if (flags & 8) {
        irrelevant = flags & 16 ? true : false;
    }
    else {
        irrelevant = false;
    }

    dbgFile << "\tirrelevant" << irrelevant;

    dbgFile << "\tfiller at " << io->pos();

    quint8 filler;
    if (!psdread(io, &filler) || filler != 0) {
        error = "Could not read padding";
        return false;
    }

    dbgFile << "\tGoing to read extra data length" << io->pos();

    quint32 extraDataLength;
    if (!psdread(io, &extraDataLength) || io->bytesAvailable() < extraDataLength) {
        error = QString("Could not read extra layer data: %1 at pos %2").arg(extraDataLength).arg(io->pos());
        return false;
    }

    dbgFile << "\tExtra data length" << extraDataLength;

    if (extraDataLength > 0) {

        dbgFile << "Going to read extra data field. Bytes available: "
                << io->bytesAvailable()
                << "pos" << io->pos();

        quint32 layerMaskLength = 1; // invalid...
        if (!psdread(io, &layerMaskLength) ||
            io->bytesAvailable() < layerMaskLength ||
            !(layerMaskLength == 0 || layerMaskLength == 20 || layerMaskLength == 36)) {
            error = QString("Could not read layer mask length: %1").arg(layerMaskLength);
            return false;
        }

        memset(&layerMask, 0, sizeof(LayerMaskData));

        if (layerMaskLength == 20 || layerMaskLength == 36) {
            if (!psdread(io, &layerMask.top)  ||
                !psdread(io, &layerMask.left) ||
                !psdread(io, &layerMask.bottom) ||
                !psdread(io, &layerMask.top) ||
                !psdread(io, &layerMask.defaultColor) ||
                !psdread(io, &flags)) {

                error = "could not read mask record";
                return false;
            }
        }
        if (layerMaskLength == 20) {
            quint16 padding;
            if (!psdread(io, &padding)) {
                error = "Could not read layer mask padding";
                return false;
            }
        }


        if (layerMaskLength == 36 ) {
            if (!psdread(io, &flags) ||
                !psdread(io, &layerMask.defaultColor) ||
                !psdread(io, &layerMask.top)  ||
                !psdread(io, &layerMask.left) ||
                !psdread(io, &layerMask.bottom) ||
                !psdread(io, &layerMask.top)) {

                error = "could not read 'real' mask record";
                return false;
            }
        }

        layerMask.positionedRelativeToLayer = flags & 1 ? true : false;
        layerMask.disabled = flags & 2 ? true : false;
        layerMask.invertLayerMaskWhenBlending = flags & 4 ? true : false;

        dbgFile << "\tRead layer mask/adjustment layer data. Length of block:"
                << layerMaskLength << "pos" << io->pos();

        // layer blending thingies
        quint32 blendingDataLength;
        if (!psdread(io, &blendingDataLength) || io->bytesAvailable() < blendingDataLength) {
            error = "Could not read extra blending data.";
            return false;
        }

        qDebug() << "blending block data length" << blendingDataLength << ", pos" << io->pos();

        blendingRanges.data = io->read(blendingDataLength);
        if ((quint32)blendingRanges.data.size() != blendingDataLength) {
            error = QString("Got %1 bytes for the blending range block, needed %2").arg(blendingRanges.data.size(), blendingDataLength);
        }
        /*
    // XXX: reading this block correctly failed, I have more channel ranges than I'd expected.

    if (!psdread(io, &blendingRanges.blackValues[0]) ||
        !psdread(io, &blendingRanges.blackValues[1]) ||
        !psdread(io, &blendingRanges.whiteValues[0]) ||
        !psdread(io, &blendingRanges.whiteValues[1]) ||
        !psdread(io, &blendingRanges.compositeGrayBlendDestinationRange)) {

        error = "Could not read blending black/white values";
        return false;
    }

    for (int i = 0; i < nChannels; ++i) {
        quint32 src;
        quint32 dst;
        if (!psdread(io, &src) || !psdread(io, &dst)) {
            error = QString("could not read src/dst range for channel %1").arg(i);
            return false;
        }
        dbgFile << "\tread range " << src << "to" << dst << "for channel" << i;
        blendingRanges.sourceDestinationRanges << QPair<quint32, quint32>(src, dst);
    }
    */
        dbgFile << "\tGoing to read layer name at" << io->pos();
        quint8 layerNameLength;
        if (!psdread(io, &layerNameLength)) {
            error = "Could not read layer name length";
            return false;
        }

        dbgFile << "\tlayer name length unpadded" << layerNameLength << "pos" << io->pos();

        layerNameLength = ((layerNameLength + 1 + 3) & ~0x03) - 1;

        dbgFile << "\tlayer name length padded" << layerNameLength << "pos" << io->pos();

        layerName = io->read(layerNameLength);

        dbgFile << "\tlayer name" << layerName << io->pos();

        QStringList longBlocks;
        if (m_header.version > 1) {
            longBlocks << "LMsk" << "Lr16" << "Layr" << "Mt16" << "Mtrn" << "Alph";
        }

        while(!io->atEnd()) {

            // read all the additional layer info 8BIM blocks
            QByteArray b;
            b = io->peek(4);
            if(b.size() != 4 || QString(b) != "8BIM") {
                break;
            }
            else {
                io->seek(io->pos() + 4); // skip the 8BIM header we peeked ahead for
            }

            QString key(io->read(4));
            if (key.size() != 4) {
                error = "Could not read key for additional layer info block";
                return false;
            }

            if (infoBlocks.contains(key)) {
                error = QString("Duplicate layer info block with key %1").arg(key);
                return false;
            }

            quint64 size;
            if (longBlocks.contains(key)) {
                psdread(io, &size);
            }
            else {
                quint32 _size;
                psdread(io, &_size);
                size = _size;
            }

            LayerInfoBlock* infoBlock = new LayerInfoBlock();
            infoBlock->data = io->read(size);
            if (infoBlock->data.size() != (qint64)size) {
                error = QString("Could not read full info block for key %1 for layer %2").arg(key).arg(layerName);
                return false;
            }

            dbgFile << "\tRead layer info block" << infoBlock->data.size();

            infoBlocks[key] = infoBlock;
        }
    }

    return valid();
}

bool PSDLayerRecord::write(QIODevice* io)
{
    Q_UNUSED(io);
    Q_ASSERT(valid());
    if (!valid()) {
        error = "Cannot write an invalid Layer Record object";
        return false;
    }
    qFatal("TODO: implement writing the layer record");
    return false;
}

bool PSDLayerRecord::valid()
{
    // XXX: check validity!
    return true;
}

QByteArray PSDLayerRecord::readChannelData(QIODevice* io, ChannelInfo *channelInfo)
{
    dbgFile << "Going to read channel data for channel " << channelInfo
            << "from io with current pos" << io->pos();

    QByteArray unCompressedBytes;
    io->seek(channelInfo->channelDataStart);
    switch(channelInfo->compressionType) {
    case Compression::Uncompressed:
        {
            unCompressedBytes = io->read(channelInfo->channelDataLength);
        }
        break;
    case Compression::RLE:
        {
            QByteArray compressedBytes;
            QBuffer buf(&unCompressedBytes);
            int uncompressedLength = (right - left) * (m_header.channelDepth / 8);
            foreach(int rleRowLength, channelInfo->rleRowLengths) {
                compressedBytes = io->read(rleRowLength);
                if (compressedBytes.length() == 0) {
                    error = QString("Could not read enough RLE bytes");
                    return QByteArray();
                }
                buf.write(Compression::uncompress(uncompressedLength, compressedBytes, channelInfo->compressionType));
            }
        }
        break;
    case Compression::ZIP:
    case Compression::ZIPWithPrediction:
        {
            io->seek(channelInfo->channelDataStart);
            int unCompressedLength = (right - left) * (bottom - top) * (m_header.channelDepth / 8);
            QByteArray compressedBytes = io->read(channelInfo->channelDataLength);
            unCompressedBytes = Compression::uncompress(unCompressedLength, compressedBytes, channelInfo->compressionType);
        }
        break;
    default:
        error = QString("Unknown compression type: %1").arg(channelInfo->compressionType);
        return QByteArray();
    }
    return unCompressedBytes;
}



QDebug operator<<(QDebug dbg, const PSDLayerRecord &layer)
{
#ifndef NODEBUG
    dbg.nospace() << "valid: " << const_cast<PSDLayerRecord*>(&layer)->valid();
    dbg.nospace() << ", name: " << layer.layerName;
    dbg.nospace() << ", top: " << layer.top;
    dbg.nospace() << ", left:" << layer.left;
    dbg.nospace() << ", bottom: " << layer.bottom;
    dbg.nospace() << ", right: " << layer.right;
    dbg.nospace() << ", number of channels: " << layer.nChannels;
    dbg.nospace() << ", blendModeKey: " << layer.blendModeKey;
    dbg.nospace() << ", opacity: " << layer.opacity;
    dbg.nospace() << ", clipping: " << layer.clipping;
    dbg.nospace() << ", transparency protected: " << layer.transparencyProtected;
    dbg.nospace() << ", visible: " << layer.visible;
    dbg.nospace() << ", irrelevant: " << layer.irrelevant << "\n";
    foreach(const ChannelInfo* channel, layer.channelInfoRecords) {
        dbg.space() << channel;
    }
#endif
    return dbg.nospace();
}


QDebug operator<<(QDebug dbg, const ChannelInfo &channel)
{
#ifndef NODEBUG
    dbg.nospace() << "\tChannel type" << channel.channelId
            << "size: " << channel.channelDataLength
            << "compression type" << channel.compressionType << "\n";
#endif
    return dbg.nospace();
}
