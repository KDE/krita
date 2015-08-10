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
#include <kis_node.h>
#include "kis_iterator_ng.h"
#include <kis_paint_layer.h>

#include "psd_utils.h"
#include "psd_header.h"
#include "compression.h"

#include <KoColorSpace.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceTraits.h>

#include <asl/kis_offset_keeper.h>
#include <asl/kis_asl_writer_utils.h>


// Just for pretty debug messages
QString channelIdToChannelType(int channelId, psd_color_mode colormode)
{
    switch(channelId) {
    case -3:
        return "Real User Supplied Layer Mask (when both a user mask and a vector mask are present";
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
            return QString("alpha: %1").arg(channelId);
        case Lab:
        case Lab48:
            return QString("alpha: %1").arg(channelId);
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
    : top(0)
    , left(0)
    , bottom(0)
    , right(0)
    , nChannels(0)
    , opacity(0)
    , clipping(0)
    , transparencyProtected(false)
    , visible(true)
    , irrelevant(false)
    , layerName("UNINITIALIZED")
    , infoBlocks(header)
    , m_transparencyMaskSizeOffset(0)
    , m_header(header)
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

    Q_ASSERT(top <= bottom);
    Q_ASSERT(left <= right);
    Q_ASSERT(nChannels > 0);


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

        ChannelInfo* info = new ChannelInfo;

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

        dbgFile << "\tchannel" << i << "id"
                << channelIdToChannelType(info->channelId, m_header.colormode)
                << "length" << info->channelDataLength
                << "start" << info->channelDataStart
                << "offset" << info->channelOffset
                << "channelInfoPosition" << info->channelInfoPosition;

        channelInfoRecords << info;

    }

    if (!psd_read_blendmode(io, blendModeKey)) {
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

    visible = flags & 2 ? false : true;

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
                    !psdread(io, &layerMask.right) ||
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

        //dbgFile << "blending block data length" << blendingDataLength << ", pos" << io->pos();

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

        if (!infoBlocks.read(io)) {
            error = infoBlocks.error;
            return false;
        }

        if (infoBlocks.keys.contains("luni") && !infoBlocks.unicodeLayerName.isEmpty()) {
            layerName = infoBlocks.unicodeLayerName;
        }

    }

    return valid();
}

void PSDLayerRecord::write(QIODevice* io,
                           KisPaintDeviceSP layerContentDevice,
                           KisNodeSP onlyTransparencyMask,
                           const QRect &maskRect,
                           psd_section_type sectionType,
                           const QDomDocument &stylesXmlDoc)
{
    dbgFile << "writing layer info record" << "at" << io->pos();

    m_layerContentDevice = layerContentDevice;
    m_onlyTransparencyMask = onlyTransparencyMask;
    m_onlyTransparencyMaskRect = maskRect;

    dbgFile << "saving layer record for " << layerName << "at pos" << io->pos();
    dbgFile << "\ttop" << top << "left" << left << "bottom" << bottom << "right" << right << "number of channels" << nChannels;
    Q_ASSERT(left <= right);
    Q_ASSERT(top <= bottom);
    Q_ASSERT(nChannels > 0);

    try {
        const QRect layerRect(left, top, right - left, bottom - top);
        KisAslWriterUtils::writeRect(layerRect, io);

        {
            quint16 realNumberOfChannels = nChannels + bool(m_onlyTransparencyMask);
            SAFE_WRITE_EX(io, realNumberOfChannels);
        }

        foreach(ChannelInfo *channel, channelInfoRecords) {
            SAFE_WRITE_EX(io, (quint16)channel->channelId);

            channel->channelInfoPosition = io->pos();

            // to be filled in when we know how big channel block is
            const quint32 fakeChannelSize = 0;
            SAFE_WRITE_EX(io, fakeChannelSize);
        }

        if (m_onlyTransparencyMask) {
            const quint16 userSuppliedMaskChannelId = -2;
            SAFE_WRITE_EX(io, userSuppliedMaskChannelId);

            m_transparencyMaskSizeOffset = io->pos();

            const quint32 fakeTransparencyMaskSize = 0;
            SAFE_WRITE_EX(io, fakeTransparencyMaskSize);
        }

        // blend mode
        dbgFile  << ppVar(blendModeKey) << ppVar(io->pos());

        KisAslWriterUtils::writeFixedString("8BIM", io);
        KisAslWriterUtils::writeFixedString(blendModeKey, io);

        SAFE_WRITE_EX(io, opacity);
        SAFE_WRITE_EX(io, clipping); // unused

        // visibility and protection
        quint8 flags = 0;
        if (transparencyProtected) flags |= 1;
        if (!visible) flags |= 2;
        if (irrelevant) {
            flags |= (1 << 3) | (1 << 4);
        }

        SAFE_WRITE_EX(io, flags);

        {
            quint8 padding = 0;
            SAFE_WRITE_EX(io, padding);
        }

        {
            // extra fields with their own length tag
            KisAslWriterUtils::OffsetStreamPusher<quint32> extraDataSizeTag(io);

            if (m_onlyTransparencyMask) {
                {
                    const quint32 layerMaskDataSize = 20; // support simple case only
                    SAFE_WRITE_EX(io, layerMaskDataSize);
                }

                KisAslWriterUtils::writeRect(m_onlyTransparencyMaskRect, io);

                {
                    KIS_ASSERT_RECOVER_NOOP(m_onlyTransparencyMask->paintDevice()->pixelSize() == 1);
                    const quint8 defaultPixel = *m_onlyTransparencyMask->paintDevice()->defaultPixel();
                    SAFE_WRITE_EX(io, defaultPixel);
                }

                {
                    const quint8 maskFlags = 0; // nothing serious
                    SAFE_WRITE_EX(io, maskFlags);

                    const quint16 padding = 0; // 2-byte padding
                    SAFE_WRITE_EX(io, padding);
                }
            } else {
                const quint32 nullLayerMaskDataSize = 0;
                SAFE_WRITE_EX(io, nullLayerMaskDataSize);
            }

            {
                // blending ranges are not implemented yet
                const quint32 nullBlendingRangesSize = 0;
                SAFE_WRITE_EX(io, nullBlendingRangesSize);
            }

            // layer name: Pascal string, padded to a multiple of 4 bytes.
            psdwrite_pascalstring(io, layerName, 4);


            PsdAdditionalLayerInfoBlock additionalInfoBlock(m_header);

            // write 'luni' data block
            additionalInfoBlock.writeLuniBlockEx(io, layerName);

            // write 'lsct' data block
            if (sectionType != psd_other) {
                additionalInfoBlock.writeLsctBlockEx(io, sectionType, isPassThrough, blendModeKey);
            }

            // write 'lfx2' data block
            if (!stylesXmlDoc.isNull()) {
                additionalInfoBlock.writeLfx2BlockEx(io, stylesXmlDoc);
            }
        }
    } catch (KisAslWriterUtils::ASLWriteException &e) {
        throw KisAslWriterUtils::ASLWriteException(PREPEND_METHOD(e.what()));
    }
}

void writeChannelDataRLE(QIODevice *io, const quint8 *plane, const int channelSize, const QRect &rc, const qint64 sizeFieldOffset)
{
    KisAslWriterUtils::OffsetStreamPusher<quint32> channelBlockSizeExternalTag(io, 0, sizeFieldOffset);

    SAFE_WRITE_EX(io, (quint16)Compression::RLE);

    // the start of RLE sizes block
    const qint64 channelRLESizePos = io->pos();

    // write zero's for the channel lengths block
    for(int i = 0; i < rc.height(); ++i) {
        // XXX: choose size for PSB!
        const quint16 fakeRLEBLockSize = 0;
        SAFE_WRITE_EX(io, fakeRLEBLockSize);
    }

    quint32 stride = channelSize * rc.width();
    for (qint32 row = 0; row < rc.height(); ++row) {

        QByteArray uncompressed = QByteArray::fromRawData((const char*)plane + row * stride, stride);
        QByteArray compressed = Compression::compress(uncompressed, Compression::RLE);

        KisAslWriterUtils::OffsetStreamPusher<quint16> rleExternalTag(io, 0, channelRLESizePos + row * sizeof(quint16));

        if (io->write(compressed) != compressed.size()) {
            throw KisAslWriterUtils::ASLWriteException("Failed to write image data");
        }
    }
}

void PSDLayerRecord::writeTransparencyMaskPixelData(QIODevice *io)
{
    if (m_onlyTransparencyMask) {
        KisPaintDeviceSP device = m_onlyTransparencyMask->paintDevice();
        KIS_ASSERT_RECOVER_NOOP(device->pixelSize() == 1);

        QByteArray buffer(m_onlyTransparencyMaskRect.width() * m_onlyTransparencyMaskRect.height(), 0);
        device->readBytes((quint8*)buffer.data(), m_onlyTransparencyMaskRect);

        writeChannelDataRLE(io, (quint8*)buffer.data(), 1, m_onlyTransparencyMaskRect, m_transparencyMaskSizeOffset);
    }
}

void PSDLayerRecord::writePixelData(QIODevice *io)
{
    dbgFile << "writing pixel data for layer" << layerName << "at" << io->pos();

    KisPaintDeviceSP dev = m_layerContentDevice;
    const QRect rc(left, top, right - left, bottom - top);

    if (rc.isEmpty()) {
        try {
            dbgFile << "Layer is empty! Writing placeholder information.";

            for (int i = 0; i < nChannels; i++) {
                const ChannelInfo *channelInfo = channelInfoRecords[i];
                KisAslWriterUtils::OffsetStreamPusher<quint32> channelBlockSizeExternalTag(io, 0, channelInfo->channelInfoPosition);
                SAFE_WRITE_EX(io, (quint16)Compression::Uncompressed);
            }

            writeTransparencyMaskPixelData(io);

        } catch (KisAslWriterUtils::ASLWriteException &e) {
            throw KisAslWriterUtils::ASLWriteException(PREPEND_METHOD(e.what()));
        }

        return;
    }

    // now write all the channels in display order
    dbgFile << "layer" << layerName;
    QVector<quint8* > tmp = dev->readPlanarBytes(rc.x() - dev->x(), rc.y() - dev->y(), rc.width(), rc.height());

    // then reorder the planes to fit the psd model -- alpha first, then display order
    QVector<quint8*> planes;
    QList<KoChannelInfo*> origChannels = dev->colorSpace()->channels();

    foreach(KoChannelInfo *ch, KoChannelInfo::displayOrderSorted(origChannels)) {
        int channelIndex = KoChannelInfo::displayPositionToChannelIndex(ch->displayPosition(), origChannels);

        quint8 *holder = tmp[channelIndex];
        tmp[channelIndex] = 0;

        if (ch->channelType() == KoChannelInfo::ALPHA) {
            planes.insert(0, holder);
        } else {
            planes.append(holder);
        }
    }

    // now planes are holding pointers to quint8 arrays
    tmp.clear();

    try {
        // here's where we save the total size of the channel data
        for (int channelInfoIndex = 0; channelInfoIndex  < nChannels; ++channelInfoIndex) {

            const ChannelInfo *channelInfo = channelInfoRecords[channelInfoIndex];

            dbgFile << "\tWriting channel" << channelInfoIndex << "psd channel id" << channelInfo->channelId;

            // if the bitdepth > 8, place the bytes in the right order
            // if cmyk, invert the pixel value
            if (m_header.channelDepth == 8) {
                if (channelInfo->channelId >= 0 && (m_header.colormode == CMYK || m_header.colormode == CMYK64)) {
                    for (int i = 0; i < rc.width() * rc.height(); ++i) {
                        planes[channelInfoIndex][i] = 255 - planes[channelInfoIndex][i];
                    }
                }
            }
            else if (m_header.channelDepth == 16) {
                quint16 val;
                for (int i = 0; i < rc.width() * rc.height(); ++i) {
                    val = reinterpret_cast<quint16*>(planes[channelInfoIndex])[i];
                    val = ntohs(val);
                    if (channelInfo->channelId >= 0 && (m_header.colormode == CMYK || m_header.colormode == CMYK64)) {
                        val = quint16_MAX - val;
                    }
                    reinterpret_cast<quint16*>(planes[channelInfoIndex])[i] = val;
                }
            }
            else if (m_header.channelDepth == 32) {
                quint32 val;
                for (int i = 0; i < rc.width() * rc.height(); ++i) {
                    val = reinterpret_cast<quint32*>(planes[channelInfoIndex])[i];
                    val = ntohl(val);
                    if (channelInfo->channelId >= 0 && (m_header.colormode == CMYK || m_header.colormode == CMYK64)) {
                        val = quint16_MAX - val;
                    }
                    reinterpret_cast<quint32*>(planes[channelInfoIndex])[i] = val;
                }
            }

            dbgFile << "\t\tchannel start" << ppVar(io->pos());

            writeChannelDataRLE(io, planes[channelInfoIndex], m_header.channelDepth / 8, rc, channelInfo->channelInfoPosition);
        }

        writeTransparencyMaskPixelData(io);

    } catch (KisAslWriterUtils::ASLWriteException &e) {
        qDeleteAll(planes);
        planes.clear();

        throw KisAslWriterUtils::ASLWriteException(PREPEND_METHOD(e.what()));
    }

    qDeleteAll(planes);
    planes.clear();
}

bool PSDLayerRecord::valid()
{
    // XXX: check validity!
    return true;
}

bool PSDLayerRecord::readPixelData(QIODevice *io, KisPaintDeviceSP device)
{
    dbgFile << "Reading pixel data for layer" << layerName << "pos" << io->pos();
    switch (m_header.colormode) {
    case Bitmap:
        error = "Unsupported color mode: bitmap";
        return false; // Not supported;
    case Indexed:
        error = "Unsupported color mode: indexed";
        return false; // Not supported;
    case MultiChannel:
        error = "Unsupported color mode: indexed";
        return false; // Not supported
    case DuoTone:
        error = "Unsupported color mode: Duotone";
        return false; // Not supported
    case Grayscale:
        return readGray(device, io);
    case RGB:
        return readRGB(device, io);
    case CMYK:
        return readCMYK(device, io);
    case Lab:
        return readLAB(device, io);
    case UNKNOWN:
    default:
        return false;
    }

    return false;
}

QRect PSDLayerRecord::channelRect(ChannelInfo *channel) const
{
    QRect result;

    if (channel->channelId < -1) {
        result = QRect(layerMask.left,
                       layerMask.top,
                       layerMask.right - layerMask.left,
                       layerMask.bottom - layerMask.top);
    } else {
        result = QRect(left,
                       top,
                       right - left,
                       bottom - top);
    }

    return result;
}

bool PSDLayerRecord::readMask(QIODevice *io, KisPaintDeviceSP dev, ChannelInfo *channelInfo)
{
    KisOffsetKeeper keeper(io);

    KIS_ASSERT_RECOVER(channelInfo->channelId < -1) { return false; }

    dbgFile << "Going to read" << channelIdToChannelType(channelInfo->channelId, m_header.colormode) << "mask";

    QRect maskRect = channelRect(channelInfo);
    if (maskRect.isEmpty()) {
        dbgFile << "Empty Channel";
        return true;
    }

    // the device must be a pixel selection
    KIS_ASSERT_RECOVER(dev->pixelSize() == 1) { return false; }

    dev->setDefaultPixel(&layerMask.defaultColor);


    int uncompressedLength = maskRect.width();

    if (channelInfo->compressionType == Compression::ZIP ||
            channelInfo->compressionType == Compression::ZIPWithPrediction) {

        error = "Unsupported Compression mode: zip";
        return false;
    }

    KisHLineIteratorSP it = dev->createHLineIteratorNG(maskRect.left(), maskRect.top(), maskRect.width());
    for (int row = maskRect.top(); row <= maskRect.bottom(); row++)
    {
        QByteArray channelBytes;

        io->seek(channelInfo->channelDataStart + channelInfo->channelOffset);

        if (channelInfo->compressionType == Compression::Uncompressed) {
            channelBytes = io->read(uncompressedLength);
            channelInfo->channelOffset += uncompressedLength;
        } else if (channelInfo->compressionType == Compression::RLE) {
            int rleLength = channelInfo->rleRowLengths[row - maskRect.top()];
            QByteArray compressedBytes = io->read(rleLength);
            channelBytes = Compression::uncompress(uncompressedLength, compressedBytes, channelInfo->compressionType);
            channelInfo->channelOffset += rleLength;
        } else {
            error = "Unsupported Compression mode: " + channelInfo->compressionType;
            return false;
        }

        for (int col = 0; col < maskRect.width(); col++){
            *it->rawData() = channelBytes[col];
            it->nextPixel();
        }

        it->nextRow();
    }

    // the position of the io device will be restored by
    // KisOffsetKeeper automagically

    return true;
}

template <class Traits>
typename Traits::channels_type convertByteOrder(typename Traits::channels_type value);
// default implementation is undefined for every color space should be added manually

template <>
inline quint8 convertByteOrder<KoGrayU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoGrayU16Traits>(quint16 value) {
    return ntohs(value);
}

template <>
inline quint32 convertByteOrder<KoGrayU32Traits>(quint32 value) {
    return ntohs(value);
}


template <class Traits>
void readGrayPixel(const QMap<quint16, QByteArray> &channelBytes,
                  int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    quint32 opacity = KoColorSpaceMathsTraits<channels_type>::unitValue;
    if (channelBytes.contains(-1)) {
        opacity = channelBytes[-1].constData()[col];
    }

    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    channels_type gray = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[0].constData())[col]);

    pixelPtr->gray = gray;
    pixelPtr->alpha = opacity;
}

bool PSDLayerRecord::readGray(KisPaintDeviceSP dev, QIODevice *io)
{
    KisOffsetKeeper keeper(io);

    qint64 width = right - left;

    if (width <= 0) {
        dbgFile << "Empty layer";
        return true;
    }

    int channelSize = m_header.channelDepth / 8;
    int uncompressedLength = width * channelSize;

    if (channelInfoRecords.first()->compressionType == Compression::ZIP
            || channelInfoRecords.first()->compressionType == Compression::ZIPWithPrediction) {

        error = "Unsupported Compression mode: zip";
        return false;
    }

    KisHLineIteratorSP it = dev->createHLineIteratorNG(left, top, width);
    for (int row = top ; row < bottom; row++)
    {
        QMap<quint16, QByteArray> channelBytes;

        foreach(ChannelInfo *channelInfo, channelInfoRecords) {
            // user supplied masks are ignored here
            if (channelInfo->channelId < -1) continue;

            io->seek(channelInfo->channelDataStart + channelInfo->channelOffset);

            if (channelInfo->compressionType == Compression::Uncompressed) {
                channelBytes[channelInfo->channelId] = io->read(uncompressedLength);
                channelInfo->channelOffset += uncompressedLength;
            }
            else if (channelInfo->compressionType == Compression::RLE) {
                int rleLength = channelInfo->rleRowLengths[row - top];
                QByteArray compressedBytes = io->read(rleLength);
                QByteArray uncompressedBytes = Compression::uncompress(uncompressedLength, compressedBytes, channelInfo->compressionType);
                channelBytes.insert(channelInfo->channelId, uncompressedBytes);
                channelInfo->channelOffset += rleLength;

            }
            else {
                error = "Unsupported Compression mode: " + channelInfo->compressionType;
                return false;
            }
        }

        for (qint64 col = 0; col < width; col++){

            if (channelSize == 1) {
                readGrayPixel<KoGrayU8Traits>(channelBytes, col, it->rawData());
            }

            else if (channelSize == 2) {
                readGrayPixel<KoGrayU16Traits>(channelBytes, col, it->rawData());
            }
            else if (channelSize == 4) {
                readGrayPixel<KoGrayU32Traits>(channelBytes, col, it->rawData());
            }
            it->nextPixel();
        }
        it->nextRow();
    }

    return true;
}

template <>
inline quint8 convertByteOrder<KoBgrU8Traits>(quint8 value) {
    return value;
}

template <>
inline quint16 convertByteOrder<KoBgrU16Traits>(quint16 value) {
    return ntohs(value);
}

template <>
inline quint32 convertByteOrder<KoBgrU32Traits>(quint32 value) {
    return ntohs(value);
}


template <class Traits>
void readRGBPixel(const QMap<quint16, QByteArray> &channelBytes,
                  int col, quint8 *dstPtr)
{
    typedef typename Traits::Pixel Pixel;
    typedef typename Traits::channels_type channels_type;

    quint32 opacity = KoColorSpaceMathsTraits<channels_type>::unitValue;
    if (channelBytes.contains(-1)) {
        opacity = channelBytes[-1].constData()[col];
    }

    Pixel *pixelPtr = reinterpret_cast<Pixel*>(dstPtr);

    channels_type blue = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[2].constData())[col]);
    channels_type green = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[1].constData())[col]);
    channels_type red = convertByteOrder<Traits>(reinterpret_cast<const channels_type *>(channelBytes[0].constData())[col]);

    pixelPtr->blue = blue;
    pixelPtr->green = green;
    pixelPtr->red = red;
    pixelPtr->alpha = opacity;
}

bool PSDLayerRecord::readRGB(KisPaintDeviceSP dev, QIODevice *io)
{
    KisOffsetKeeper keeper(io);

    qint64 width = right - left;

    if (width <= 0) {
        dbgFile << "Empty layer";
        return true;
    }

    int channelSize = m_header.channelDepth / 8;
    int uncompressedLength = width * channelSize;

    if (channelInfoRecords.first()->compressionType == Compression::ZIP
            || channelInfoRecords.first()->compressionType == Compression::ZIPWithPrediction) {

        error = "Unsupported Compression mode: zip";
        return false;
    }

    KisHLineIteratorSP it = dev->createHLineIteratorNG(left, top, width);
    for (int row = top ; row < bottom; row++)
    {
        QMap<quint16, QByteArray> channelBytes;

        foreach(ChannelInfo *channelInfo, channelInfoRecords) {
            // user supplied masks are ignored here
            if (channelInfo->channelId < -1) continue;

            io->seek(channelInfo->channelDataStart + channelInfo->channelOffset);

            if (channelInfo->compressionType == Compression::Uncompressed) {
                channelBytes[channelInfo->channelId] = io->read(uncompressedLength);
                channelInfo->channelOffset += uncompressedLength;
            }
            else if (channelInfo->compressionType == Compression::RLE) {
                int rleLength = channelInfo->rleRowLengths[row - top];
                QByteArray compressedBytes = io->read(rleLength);
                QByteArray uncompressedBytes = Compression::uncompress(uncompressedLength, compressedBytes, channelInfo->compressionType);
                channelBytes.insert(channelInfo->channelId, uncompressedBytes);
                channelInfo->channelOffset += rleLength;

            }
            else {
                error = "Unsupported Compression mode: " + channelInfo->compressionType;
                return false;
            }
        }

        for (qint64 col = 0; col < width; col++){

            if (channelSize == 1) {
                readRGBPixel<KoBgrU8Traits>(channelBytes, col, it->rawData());
            }
            else if (channelSize == 2) {
                readRGBPixel<KoBgrU16Traits>(channelBytes, col, it->rawData());
            }
            else if (channelSize == 4) {
                readRGBPixel<KoBgrU16Traits>(channelBytes, col, it->rawData());
            }
            it->nextPixel();
        }
        it->nextRow();
    }

    return true;
}

bool PSDLayerRecord::readCMYK(KisPaintDeviceSP dev, QIODevice *io)
{
    dbgFile << "doCMYK for" << layerName << "channels:" << channelInfoRecords.size() << "compression" << channelInfoRecords.first()->compressionType;
    dbgFile << "top" << top << "bottom" << bottom << "left" << left << "right" << right;

    KisOffsetKeeper keeper(io);

    quint64 width = right - left;
    int channelSize = m_header.channelDepth / 8;
    int uncompressedLength = width * channelSize;


    if (channelInfoRecords.first()->compressionType == Compression::ZIP
            || channelInfoRecords.first()->compressionType == Compression::ZIPWithPrediction) {
        dbgFile << "zippedy-do-da!";
        // Zip needs to be implemented here.
        return false;
    }

    KisHLineIteratorSP it = dev->createHLineIteratorNG(left, top, width);
    for (int row = top ; row < bottom; row++)
    {

        QMap<quint16, QByteArray> channelBytes;

        foreach(ChannelInfo *channelInfo, channelInfoRecords) {

            io->seek(channelInfo->channelDataStart + channelInfo->channelOffset);

            if (channelInfo->compressionType == Compression::Uncompressed) {
                channelBytes[channelInfo->channelId] = io->read(uncompressedLength);
                channelInfo->channelOffset += uncompressedLength;
            }
            else if (channelInfo->compressionType == Compression::RLE) {
                int rleLength = channelInfo->rleRowLengths[row - top];
                QByteArray compressedBytes = io->read(rleLength);
                QByteArray uncompressedBytes = Compression::uncompress(uncompressedLength, compressedBytes, channelInfo->compressionType);
                channelBytes.insert(channelInfo->channelId, uncompressedBytes);
                channelInfo->channelOffset += rleLength;

            }
        }

        for (quint64 col = 0; col < width; col++){

            if (channelSize == 1) {

                quint8 opacity = OPACITY_OPAQUE_U8;
                if (channelBytes.contains(-1)) {
                    opacity = channelBytes[-1].constData()[col];

                }
                quint8 *pixel = new quint8[5];
                memset(pixel, 0, 5);
                dev->colorSpace()->setOpacity(pixel, opacity, 1);

                memset(pixel, 255 - channelBytes[0].constData()[col], 1);
                memset(pixel + 1, 255 - channelBytes[1].constData()[col], 1);
                memset(pixel + 2, 255 - channelBytes[2].constData()[col], 1);
                memset(pixel + 3, 255 - channelBytes[3].constData()[col], 1);
                //dbgFile << "C" << pixel[0] << "M" << pixel[1] << "Y" << pixel[2] << "K" << pixel[3] << "A" << pixel[4];
                memcpy(it->rawData(), pixel, 5);

                delete[] pixel;
            }
            else if (channelSize == 2) {

                quint16 opacity = quint16_MAX;
                if (channelBytes.contains(-1)) {
                    opacity = channelBytes[-1].constData()[col];
                }

                // We don't have a convenient setOpacity function :-(
                memcpy(it->rawData() + KoCmykTraits<quint16>::alpha_pos, &opacity, sizeof(quint16));

                quint16 C = ntohs(reinterpret_cast<const quint16 *>(channelBytes[0].constData())[col]);
                KoCmykTraits<quint16>::setC(it->rawData(),C);

                quint16 M = ntohs(reinterpret_cast<const quint16 *>(channelBytes[1].constData())[col]);
                KoCmykTraits<quint16>::setM(it->rawData(),M);

                quint16 Y = ntohs(reinterpret_cast<const quint16 *>(channelBytes[2].constData())[col]);
                KoCmykTraits<quint16>::setY(it->rawData(),Y);

                quint16 K = ntohs(reinterpret_cast<const quint16 *>(channelBytes[3].constData())[col]);
                KoCmykTraits<quint16>::setK(it->rawData(),K);

            }
            else if (channelSize == 4) {

                quint32 C = ntohs(reinterpret_cast<const quint32 *>(channelBytes[0].constData())[col]);
                KoCmykTraits<quint32>::setC(it->rawData(),C);

                quint32 M = ntohs(reinterpret_cast<const quint32 *>(channelBytes[1].constData())[col]);
                KoCmykTraits<quint32>::setM(it->rawData(),M);

                quint32 Y = ntohs(reinterpret_cast<const quint32 *>(channelBytes[2].constData())[col]);
                KoCmykTraits<quint32>::setY(it->rawData(),Y);

                quint32 K = ntohs(reinterpret_cast<const quint32 *>(channelBytes[3].constData())[col]);
                KoCmykTraits<quint32>::setK(it->rawData(),K);
            }

            else {
                // Unsupported channel sizes for now
                return false;
            }
            it->nextPixel();
        }
        it->nextRow();
    }

    return true;
}

bool PSDLayerRecord::readLAB(KisPaintDeviceSP dev, QIODevice *io)
{
    KisOffsetKeeper keeper(io);

    quint64 width = right - left;
    int channelSize = m_header.channelDepth / 8;
    int uncompressedLength = width * channelSize;

    if (channelInfoRecords.first()->compressionType == Compression::ZIP
            || channelInfoRecords.first()->compressionType == Compression::ZIPWithPrediction) {

        // Zip needs to be implemented here.
        return false;
    }

    KisHLineIteratorSP it = dev->createHLineIteratorNG(left, top, width);
    for (int row = top ; row < bottom; row++)
    {

        QMap<quint16, QByteArray> channelBytes;

        foreach(ChannelInfo *channelInfo, channelInfoRecords) {

            io->seek(channelInfo->channelDataStart + channelInfo->channelOffset);

            if (channelInfo->compressionType == Compression::Uncompressed) {
                channelBytes[channelInfo->channelId] = io->read(uncompressedLength);
                channelInfo->channelOffset += uncompressedLength;
            }
            else if (channelInfo->compressionType == Compression::RLE) {
                int rleLength = channelInfo->rleRowLengths[row - top];
                QByteArray compressedBytes = io->read(rleLength);
                QByteArray uncompressedBytes = Compression::uncompress(uncompressedLength, compressedBytes, channelInfo->compressionType);
                channelBytes.insert(channelInfo->channelId, uncompressedBytes);
                channelInfo->channelOffset += rleLength;

            }
        }

        for (quint64 col = 0; col < width; col++){

            if (channelSize == 1) {
                quint8 opacity = OPACITY_OPAQUE_U8;
                if (channelBytes.contains(-1)) {
                    opacity = channelBytes[-1].constData()[col];
                }
                KoLabTraits<quint8>::setOpacity(it->rawData(), opacity, 1);

                quint8 L = ntohs(reinterpret_cast<const quint8 *>(channelBytes[0].constData())[col]);
                KoLabTraits<quint8>::setL(it->rawData(),L);

                quint8 A = ntohs(reinterpret_cast<const quint8 *>(channelBytes[1].constData())[col]);
                KoLabTraits<quint8>::setA(it->rawData(),A);

                quint8 B = ntohs(reinterpret_cast<const quint8 *>(channelBytes[2].constData())[col]);
                KoLabTraits<quint8>::setB(it->rawData(),B);


            }

            else if (channelSize == 2) {

                quint16 opacity = quint16_MAX;
                if (channelBytes.contains(-1)) {
                    opacity = channelBytes[-1].constData()[col];
                }
                // We don't have a convenient setOpacity function :-(
                memcpy(it->rawData() + KoLabU16Traits::alpha_pos, &opacity, sizeof(quint16));
                // KoLabTraits<quint16>::setOpacity(it->rawData(), opacity, 1);

                quint16 L = ntohs(reinterpret_cast<const quint16 *>(channelBytes[0].constData())[col]);
                KoLabTraits<quint16>::setL(it->rawData(),L);

                quint16 A = ntohs(reinterpret_cast<const quint16 *>(channelBytes[1].constData())[col]);
                KoLabTraits<quint16>::setA(it->rawData(),A);

                quint16 B = ntohs(reinterpret_cast<const quint16 *>(channelBytes[2].constData())[col]);
                KoLabTraits<quint16>::setB(it->rawData(),B);
            }
            else {
                // Unsupported channel sizes for now
                return false;
            }

            it->nextPixel();
        }
        it->nextRow();
    }

    return true;
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
