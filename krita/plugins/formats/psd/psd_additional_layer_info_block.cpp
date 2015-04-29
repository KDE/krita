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

#include <QBuffer>

#include "psd_utils.h"


PsdAdditionalLayerInfoBlock::PsdAdditionalLayerInfoBlock()
{
}

bool PsdAdditionalLayerInfoBlock::read(QIODevice *io)
{
    QStringList longBlocks;
    if (m_header.version > 1) {
        longBlocks << "LMsk" << "Lr16" << "Layr" << "Mt16" << "Mtrn" << "Alph";
    }

    while (!io->atEnd()) {

        // read all the additional layer info 8BIM blocks
        QByteArray b;
        b = io->peek(4);
        if (b.size() != 4 || QString(b) != "8BIM") {
            error = "No 8BIM marker for additional layer info block";
            return true;
        }
        else {
            io->seek(io->pos() + 4); // skip the 8BIM header we peeked ahead for
        }

        QString key(io->read(4));
        if (key.size() != 4) {
            error = "Could not read key for additional layer info block";
            return false;
        }
        dbgFile << "found info block with key" << key;
        if (keys.contains(key)) {
            error = "Found duplicate entry for key ";
            continue;
        }
        keys << key;

        quint64 size;
        if (longBlocks.contains(key)) {
            psdread(io, &size);
        }
        else {
            quint32 _size;
            psdread(io, &_size);
            size = _size;
        }

        QByteArray data = io->read(size);
        if (data.size() != (qint64)size) {
            error = QString("Could not read full info block for key %1.").arg(key);
            return false;
        }

        dbgFile << "\tRead layer info block" << key << "for size" << data.size();
        QBuffer buf(&data);
        buf.open(QBuffer::ReadOnly);

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
//            if (!psdread(&buf, &layerEffects.version) || layerEffects.version != 0) {
//                dbgFile << "Layer Effect version is not zero" << layerEffects.version;
//                continue;
//            }
//            if (!psdread(&buf, &layerEffects.effects_count)
//                    || layerEffects.effects_count < 6
//                    || layerEffects.effects_count > 7) {
//                dbgFile << "Wrong number of Layer Effects " << layerEffects.effects_count;
//                continue;
//            }
//            for (int i = 0; i < layerEffects.effects_count; ++i) {
//                QByteArray b;
//                b = buf.peek(4);
//                if (b.size() != 4 || QString(b) != "8BIM") {
//                    error = "No 8BIM marker for lrFX block";
//                    return false;
//                }
//                else {
//                    buf.seek(buf.pos() + 4); // skip the 8BIM header we peeked ahead for
//                }
//                QString tag = QString(io->read(4));
//                if (tag.size() != 4) {
//                    error = "Could not read layer effect type tag";
//                    return false;
//                }

//            }

        }
        else if (key == "tySh") {
        }
        else if (key == "luni") {
            // get the unicode layer name
            psdread_unicodestring(&buf, unicodeLayerName);
            dbgFile << "unicodeLayerName" << unicodeLayerName;
        }
        else if (key == "lyid") {

        }
        else if (key == "lfx2") {

        }
        else if (key == "Patt" || key == "Pat2" || key == "Pat3") {

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
            quint32 type;
            if (!psdread(&buf, &type)) {
                error = "Could not read group type";
                return false;
            }
            if (size >= 12) {
                if (!psd_read_blendmode(io, sectionDividerBlendMode)) {
                    error = QString("Could not read blend mode key. Got: %1").arg(sectionDividerBlendMode);
                    return false;
                }
            }
            if (size >= 16) {
                // Don't care: animation scene group
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
    return true;


}

bool PsdAdditionalLayerInfoBlock::write(QIODevice */*io*/, KisNodeSP /*node*/)
{
    return true;
}


bool PsdAdditionalLayerInfoBlock::valid()
{

    return true;
}
