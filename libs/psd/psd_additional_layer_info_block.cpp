/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "psd_additional_layer_info_block.h"
#include "psd.h"

#include <QDomDocument>

#include <asl/kis_offset_on_exit_verifier.h>

#include <asl/kis_asl_patterns_writer.h>
#include <asl/kis_asl_reader.h>
#include <asl/kis_asl_reader_utils.h>
#include <asl/kis_asl_writer.h>
#include <asl/kis_asl_writer_utils.h>

PsdAdditionalLayerInfoBlock::PsdAdditionalLayerInfoBlock(const PSDHeader &header)
    : m_header(header)
{
}

void PsdAdditionalLayerInfoBlock::setExtraLayerInfoBlockHandler(ExtraLayerInfoBlockHandler handler)
{
    m_layerInfoBlockHandler = handler;
}

void PsdAdditionalLayerInfoBlock::setUserMaskInfoBlockHandler(UserMaskInfoBlockHandler handler)
{
    m_userMaskBlockHandler = handler;
}

bool PsdAdditionalLayerInfoBlock::read(QIODevice &io)
{
    bool result = true;

    try {
        switch (m_header.byteOrder) {
        case psd_byte_order::psdLittleEndian:
            readImpl<psd_byte_order::psdLittleEndian>(io);
            break;
        default:
            readImpl(io);
            break;
        }
    } catch (KisAslReaderUtils::ASLParseException &e) {
        error = e.what();
        result = false;
    }

    return result;
}

template<psd_byte_order byteOrder>
void PsdAdditionalLayerInfoBlock::readImpl(QIODevice &io)
{
    using namespace KisAslReaderUtils;

    QStringList longBlocks;
    if (m_header.version > 1) {
        longBlocks << "LMsk"
                   << "Lr16"
                   << "Layr"
                   << "Mt16"
                   << "Mtrn"
                   << "Alph";
    }

    while (!io.atEnd()) {
        {
            const std::array<quint8, 4> refSignature1 = {'8', 'B', 'I', 'M'}; // '8BIM' in big-endian
            const std::array<quint8, 4> refSignature2 = {'8', 'B', '6', '4'}; // '8B64' in big-endian

            if (!TRY_READ_SIGNATURE_2OPS_EX<byteOrder>(io, refSignature1, refSignature2)) {
                break;
            }
        }

        QString key = readFixedString<byteOrder>(io);
        dbgFile << "found info block with key" << key << "(" << io.pos() << ")";

        quint64 blockSize = GARBAGE_VALUE_MARK;
        if (longBlocks.contains(key)) {
            SAFE_READ_EX(byteOrder, io, blockSize);
        } else {
            quint32 size32;
            SAFE_READ_EX(byteOrder, io, size32);
            blockSize = size32;
        }

        // Since TIFF headers are padded to multiples of 4,
        // staving them off here is way easier.
        if (m_header.tiffStyleLayerBlock) {
            if (blockSize % 4U) {
                dbgFile << "(TIFF) WARNING: current block size is NOT a multiple of 4! Fixing...";
                blockSize += (4U - blockSize % 4U);
            }
        }

        dbgFile << "info block size" << blockSize << "(" << io.pos() << ")";

        // offset verifier will correct the position on the exit from
        // current namespace, including 'continue', 'return' and
        // exceptions.
        SETUP_OFFSET_VERIFIER(infoBlockEndVerifier, io, blockSize, 0);

        if (keys.contains(key)) {
            error = "Found duplicate entry for key ";
            continue;
        }
        keys << key;

        // TODO: Loading of 32 bit files is not supported yet
        if (key == "Lr16" /* || key == "Lr32"*/) {
            if (m_layerInfoBlockHandler) {
                int offset = m_header.version > 1 ? 8 : 4;
                dbgFile << "Offset for block handler: " << io.pos() << offset;
                io.seek(io.pos() - offset);
                m_layerInfoBlockHandler(io);
            }
        } else if (key == "Layr") {
            if (m_header.tiffStyleLayerBlock && m_layerInfoBlockHandler) {
                int offset = m_header.version > 1 ? 8 : 4;
                dbgFile << "(TIFF) Offset for block handler: " << io.pos() << offset;
                io.seek(io.pos() - offset);
                m_layerInfoBlockHandler(io);
            }
        } else if (key == "SoCo") {
        } else if (key == "GdFl") {
        } else if (key == "PtFl") {
        } else if (key == "brit") {
        } else if (key == "levl") {
        } else if (key == "curv") {
        } else if (key == "expA") {
        } else if (key == "vibA") {
        } else if (key == "hue") {
        } else if (key == "hue2") {
        } else if (key == "blnc") {
        } else if (key == "blwh") {
        } else if (key == "phfl") {
        } else if (key == "mixr") {
        } else if (key == "clrL") {
        } else if (key == "nvrt") {
        } else if (key == "post") {
        } else if (key == "thrs") {
        } else if (key == "selc") {
        } else if (key == "lrFX") {
            // deprecated! use lfx2 instead!
        } else if (key == "tySh") {
        } else if (key == "luni") {
            // get the unicode layer name
            unicodeLayerName = readUnicodeString<byteOrder>(io);
            dbgFile << "unicodeLayerName" << unicodeLayerName;
        } else if (key == "lyid") {
            quint32 id;
            psdread<byteOrder>(io, id);
            dbgFile << "layer ID:" << id;
        } else if (key == "lfx2" || key == "lfxs") {
            // lfxs is a special variant of layer styles for group layers
            layerStyleXml = KisAslReader::readLfx2PsdSection(io, byteOrder);
        } else if (key == "Patt" || key == "Pat2" || key == "Pat3") {
            QDomDocument pattern = KisAslReader::readPsdSectionPattern(io, blockSize, byteOrder);
            embeddedPatterns << pattern;
        } else if (key == "Anno") {
        } else if (key == "clbl") {
        } else if (key == "infx") {
        } else if (key == "knko") {
        } else if (key == "spf") {
        } else if (key == "lclr") {
        } else if (key == "fxrp") {
        } else if (key == "grdm") {
        } else if (key == "lsct") {
            quint32 dividerType = GARBAGE_VALUE_MARK;
            SAFE_READ_EX(byteOrder, io, dividerType);
            this->sectionDividerType = (psd_section_type)dividerType;

            dbgFile << "Reading \"lsct\" block:";
            dbgFile << ppVar(blockSize);
            dbgFile << ppVar(dividerType);

            if (blockSize >= 12) {
                quint32 lsctSignature = GARBAGE_VALUE_MARK;
                const quint32 refSignature1 = 0x3842494D; // '8BIM' in little-endian
                SAFE_READ_SIGNATURE_EX(byteOrder, io, lsctSignature, refSignature1);

                this->sectionDividerBlendMode = readFixedString<byteOrder>(io);

                dbgFile << ppVar(this->sectionDividerBlendMode);
            }

            // Animation
            if (blockSize >= 14) {
                /**
                 * "I don't care
                 *  I don't care, no... !" (c)
                 */
            }

        } else if (key == "brst") {
        } else if (key == "vmsk" || key == "vsms") { // If key is "vsms" then we are writing for (Photoshop CS6) and the document will have a "vscg" key

        } else if (key == "TySh") {
        } else if (key == "ffxi") {
        } else if (key == "lnsr") {
        } else if (key == "shpa") {
        } else if (key == "shmd") {
        } else if (key == "lyvr") {
        } else if (key == "tsly") {
        } else if (key == "lmgm") {
        } else if (key == "vmgm") {
        } else if (key == "plLd") { // Replaced by SoLd in CS3

        } else if (key == "linkD" || key == "lnk2" || key == "lnk3") {
        } else if (key == "CgEd") {
        } else if (key == "Txt2") {
        } else if (key == "pths") {
        } else if (key == "anFX") {
        } else if (key == "FMsk") {
        } else if (key == "SoLd") {
        } else if (key == "vstk") {
        } else if (key == "vsCg") {
        } else if (key == "sn2P") {
        } else if (key == "vogk") {
        } else if (key == "Mtrn" || key == "Mt16" || key == "Mt32") { // There is no data associated with these keys.

        } else if (key == "LMsk") {
            // TIFFs store the global mask here.
            if (m_header.tiffStyleLayerBlock) {
                int offset = m_header.version > 1 ? 8 : 4;
                dbgFile << "(TIFF) Offset for block handler: " << io.pos() << offset;
                io.seek(io.pos() - offset);
                m_userMaskBlockHandler(io);
            }
        } else if (key == "FXid") {
        } else if (key == "FEid") {
        }
    }
}

bool PsdAdditionalLayerInfoBlock::write(QIODevice & /*io*/, KisNodeSP /*node*/)
{
    return true;
}

bool PsdAdditionalLayerInfoBlock::valid()
{
    return true;
}

void PsdAdditionalLayerInfoBlock::writeLuniBlockEx(QIODevice &io, const QString &layerName)
{
    switch (m_header.byteOrder) {
    case psd_byte_order::psdLittleEndian:
        writeLuniBlockExImpl<psd_byte_order::psdLittleEndian>(io, layerName);
        break;
    default:
        writeLuniBlockExImpl(io, layerName);
        break;
    }
}

template<psd_byte_order byteOrder>
void PsdAdditionalLayerInfoBlock::writeLuniBlockExImpl(QIODevice &io, const QString &layerName)
{
    KisAslWriterUtils::writeFixedString<byteOrder>("8BIM", io);
    KisAslWriterUtils::writeFixedString<byteOrder>("luni", io);
    KisAslWriterUtils::OffsetStreamPusher<quint32, byteOrder> layerNameSizeTag(io, 2);
    KisAslWriterUtils::writeUnicodeString<byteOrder>(layerName, io);
}

void PsdAdditionalLayerInfoBlock::writeLsctBlockEx(QIODevice &io, psd_section_type sectionType, bool isPassThrough, const QString &blendModeKey)
{
    switch (m_header.byteOrder) {
    case psd_byte_order::psdLittleEndian:
        writeLsctBlockExImpl<psd_byte_order::psdLittleEndian>(io, sectionType, isPassThrough, blendModeKey);
        break;
    default:
        writeLsctBlockExImpl(io, sectionType, isPassThrough, blendModeKey);
        break;
    }
}

template<psd_byte_order byteOrder>
void PsdAdditionalLayerInfoBlock::writeLsctBlockExImpl(QIODevice &io, psd_section_type sectionType, bool isPassThrough, const QString &blendModeKey)
{
    KisAslWriterUtils::writeFixedString<byteOrder>("8BIM", io);
    KisAslWriterUtils::writeFixedString<byteOrder>("lsct", io);
    KisAslWriterUtils::OffsetStreamPusher<quint32, byteOrder> sectionTypeSizeTag(io, 2);
    SAFE_WRITE_EX(byteOrder, io, (quint32)sectionType);

    QString realBlendModeKey = isPassThrough ? QString("pass") : blendModeKey;

    KisAslWriterUtils::writeFixedString<byteOrder>("8BIM", io);
    KisAslWriterUtils::writeFixedString<byteOrder>(realBlendModeKey, io);
}

void PsdAdditionalLayerInfoBlock::writeLfx2BlockEx(QIODevice &io, const QDomDocument &stylesXmlDoc, bool useLfxsLayerStyleFormat)
{
    switch (m_header.byteOrder) {
    case psd_byte_order::psdLittleEndian:
        writeLfx2BlockExImpl<psd_byte_order::psdLittleEndian>(io, stylesXmlDoc, useLfxsLayerStyleFormat);
        break;
    default:
        writeLfx2BlockExImpl(io, stylesXmlDoc, useLfxsLayerStyleFormat);
        break;
    }
}

template<psd_byte_order byteOrder>
void PsdAdditionalLayerInfoBlock::writeLfx2BlockExImpl(QIODevice &io, const QDomDocument &stylesXmlDoc, bool useLfxsLayerStyleFormat)
{
    KisAslWriterUtils::writeFixedString<byteOrder>("8BIM", io);
    // 'lfxs' format is used for Group layers in PS
    KisAslWriterUtils::writeFixedString<byteOrder>(!useLfxsLayerStyleFormat ? "lfx2" : "lfxs", io);
    KisAslWriterUtils::OffsetStreamPusher<quint32, byteOrder> lfx2SizeTag(io, 2);

    try {
        KisAslWriter writer(byteOrder);
        writer.writePsdLfx2SectionEx(io, stylesXmlDoc);

    } catch (KisAslWriterUtils::ASLWriteException &e) {
        warnKrita << "WARNING: Couldn't save layer style lfx2 block:" << PREPEND_METHOD(e.what());

        // TODO: make this error recoverable!
        throw e;
    }
}

void PsdAdditionalLayerInfoBlock::writePattBlockEx(QIODevice &io, const QDomDocument &patternsXmlDoc)
{
    switch (m_header.byteOrder) {
    case psd_byte_order::psdLittleEndian:
        writePattBlockExImpl<psd_byte_order::psdLittleEndian>(io, patternsXmlDoc);
        break;
    default:
        writePattBlockExImpl(io, patternsXmlDoc);
        break;
    }
}

template<psd_byte_order byteOrder>
void PsdAdditionalLayerInfoBlock::writePattBlockExImpl(QIODevice &io, const QDomDocument &patternsXmlDoc)
{
    KisAslWriterUtils::writeFixedString<byteOrder>("8BIM", io);
    KisAslWriterUtils::writeFixedString<byteOrder>("Patt", io);
    const quint32 padding = m_header.tiffStyleLayerBlock ? 4 : 2;
    KisAslWriterUtils::OffsetStreamPusher<quint32, byteOrder> pattSizeTag(io, padding);

    try {
        KisAslPatternsWriter writer(patternsXmlDoc, io, byteOrder);
        writer.writePatterns();

    } catch (KisAslWriterUtils::ASLWriteException &e) {
        warnKrita << "WARNING: Couldn't save layer style patterns block:" << PREPEND_METHOD(e.what());

        // TODO: make this error recoverable!
        throw e;
    }
}
