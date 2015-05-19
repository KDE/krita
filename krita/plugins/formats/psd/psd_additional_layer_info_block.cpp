/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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

#include "psd_additional_layer_info_block.h"

#include <QDomDocument>

#include <asl/kis_offset_on_exit_verifier.h>

#include <asl/kis_asl_reader_utils.h>
#include <asl/kis_asl_reader.h>


PsdAdditionalLayerInfoBlock::PsdAdditionalLayerInfoBlock(const PSDHeader& header)
    : m_header(header)
{
}

bool PsdAdditionalLayerInfoBlock::read(QIODevice *io)
{
    bool result = true;

    try {
        readImpl(io);
    } catch (KisAslReaderUtils::ASLParseException &e) {
        error = e.what();
        result = false;
    }

    return result;
}

void PsdAdditionalLayerInfoBlock::readImpl(QIODevice* io)
{
    using namespace KisAslReaderUtils;

    QStringList longBlocks;
    if (m_header.version > 1) {
        longBlocks << "LMsk" << "Lr16" << "Layr" << "Mt16" << "Mtrn" << "Alph";
    }

    while (!io->atEnd()) {

        {
            const quint32 refSignature1 = 0x3842494D; // '8BIM' in little-endian
            const quint32 refSignature2 = 0x38423634; // '8B64' in little-endian
            if (!TRY_READ_SIGNATURE_2OPS_EX(io, refSignature1, refSignature2)) {
                break;
            }
        }

        QString key = readFixedString(io);
        dbgFile << "found info block with key" << key;

        quint64 blockSize = GARBAGE_VALUE_MARK;
        if (longBlocks.contains(key)) {
            SAFE_READ_EX(io, blockSize);
        } else {
            quint32 size32;
            SAFE_READ_EX(io, size32);
            blockSize = size32;
        }

        // offset verifier will correct the position on the exit from
        // current namespace, including 'continue', 'return' and
        // exceptions.
        SETUP_OFFSET_VERIFIER(infoBlockEndVerifier, io, blockSize, 0);

        if (keys.contains(key)) {
            error = "Found duplicate entry for key ";
            continue;
        }
        keys << key;

        if (key == "SoCo") {

        }
        else if (key == "GdFl") {

        }
        else if (key == "PtFl") {

        }
        else if (key == "brit") {

        }
        else if (key == "levl") {

        }
        else if (key == "curv") {

        }
        else if (key == "expA") {

        }
        else if (key == "vibA") {

        }
        else if (key == "hue") {

        }
        else if (key == "hue2") {

        }
        else if (key == "blnc") {

        }
        else if (key == "blwh") {

        }
        else if (key == "phfl") {

        }
        else if (key == "mixr") {

        }
        else if (key == "clrL") {

        }
        else if (key == "nvrt") {

        }
        else if (key == "post") {

        }
        else if (key == "thrs") {

        }
        else if (key == "grdm") {

        }
        else if (key == "selc") {

        }
        else if (key == "lrFX") {
            // deprecated! use lfx2 instead!
        }
        else if (key == "tySh") {
        }
        else if (key == "luni") {
            // get the unicode layer name
            unicodeLayerName = readUnicodeString(io);
            dbgFile << "unicodeLayerName" << unicodeLayerName;
        }
        else if (key == "lyid") {

        }
        else if (key == "lfx2") {
            KisAslReader reader;
            layerStyleXml = reader.readLfx2PsdSection(io);
        }
        else if (key == "Patt" || key == "Pat2" || key == "Pat3") {
            KisAslReader reader;
            QDomDocument pattern = reader.readPsdSectionPattern(io, blockSize);

            embeddedPatterns << pattern;
        }
        else if (key == "Anno") {

        }
        else if (key == "clbl") {

        }
        else if (key == "infx") {

        }
        else if (key == "knko") {

        }
        else if (key == "spf") {

        }
        else if (key == "lclr") {

        }
        else if (key == "fxrp") {

        }
        else if (key == "grdm") {

        }
        else if (key == "lsct") {
            quint32 dividerType = GARBAGE_VALUE_MARK;
            SAFE_READ_EX(io, dividerType);
            this->sectionDividerType = (psd_section_type)dividerType;

            dbgFile << "Reading \"lsct\" block:";
            dbgFile << ppVar(blockSize);
            dbgFile << ppVar(dividerType);

            if (blockSize >= 12) {
                quint32 lsctSignature = GARBAGE_VALUE_MARK;
                const quint32 refSignature1 = 0x3842494D; // '8BIM' in little-endian
                SAFE_READ_SIGNATURE_EX(io, lsctSignature, refSignature1);

                this->sectionDividerBlendMode = readFixedString(io);

                dbgFile << ppVar(this->sectionDividerBlendMode);
            }

            // Animation
            if (blockSize >= 14) {
                /**
                 * "I don't care
                 *  I don't care, no... !" (c)
                 */
            }

        }
        else if (key == "brst") {

        }
        else if (key == "SoCo") {

        }
        else if (key == "PtFl") {

        }
        else if (key == "GdFl") {

        }
        else if (key == "vmsk" || key == "vsms") { // If key is "vsms" then we are writing for (Photoshop CS6) and the document will have a "vscg" key

        }
        else if (key == "TySh") {

        }
        else if (key == "ffxi") {

        }
        else if (key == "lnsr") {

        }
        else if (key == "shpa") {

        }
        else if (key == "shmd") {

        }
        else if (key == "lyvr") {

        }
        else if (key == "tsly") {

        }
        else if (key == "lmgm") {

        }
        else if (key == "vmgm") {

        }
        else if (key == "plLd") { // Replaced by SoLd in CS3

        }
        else if (key == "linkD" || key == "lnk2" || key == "lnk3") {

        }
        else if (key == "phfl") {

        }
        else if (key == "blwh") {

        }
        else if (key == "CgEd") {

        }
        else if (key == "Txt2") {

        }
        else if (key == "vibA") {

        }
        else if (key == "pths") {

        }
        else if (key == "anFX") {

        }
        else if (key == "FMsk") {

        }
        else if (key == "SoLd") {

        }
        else if (key == "vstk") {

        }
        else if (key == "vsCg") {

        }
        else if (key == "sn2P") {

        }
        else if (key == "vogk") {

        }
        else if (key == "Mtrn" || key == "Mt16" || key == "Mt32") { // There is no data associated with these keys.

        }
        else if (key == "LMsk") {

        }
        else if (key == "expA") {

        }
        else if (key == "FXid") {

        }
        else if (key == "FEid") {

        }
    }
}

bool PsdAdditionalLayerInfoBlock::write(QIODevice */*io*/, KisNodeSP /*node*/)
{
    return true;
}


bool PsdAdditionalLayerInfoBlock::valid()
{

    return true;
}
