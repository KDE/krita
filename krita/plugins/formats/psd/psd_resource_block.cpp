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
#include "psd_resource_block.h"

#include <QIODevice>
#include <QBuffer>
#include <QDataStream>

#include <kis_debug.h>

#include "psd_utils.h"
#include "psd_resource_section.h"

PSDResourceBlock::PSDResourceBlock()
    : identifier(PSDResourceSection::UNKNOWN)
    , resource(0)
{
}

bool PSDResourceBlock::read(QIODevice* io)
{
    dbgFile << "Reading resource block";
    if (io->atEnd()) {
        error = "Could not read resource block: no bytes left.";
        return false;
    }

    QByteArray b;
    b = io->read(4);
    if(b.size() != 4 || QString(b) != "8BIM") {
        error = QString("Could not read resource block signature. Got %1.")
                .arg(QString(b));
        return false;
    }

    if (!psdread(io, &identifier)) {
        error = "Could not read resource block identifier";
        return false;
    }

    dbgFile << "\tresource block identifier" << identifier;

    if (!psdread_pascalstring(io, name)) {
        error = "Could not read name of resource block";
        return false;
    }

    dbgFile << "\tresource block name" << name;

    if (!psdread(io, &dataSize)) {
        error = QString("Could not read datasize for resource block with name %1 of type %2").arg(name).arg(identifier);
        return false;
    }


    if ((dataSize & 0x01) != 0) {
        dataSize++;
    }

    dbgFile << "\tresource block size" << dataSize;

    data = io->read(dataSize);
    if (!data.size() == dataSize) {
        error = QString("Could not read data for resource block with name %1 of type %2").arg(name).arg(identifier);
        return false;
    }

    switch (identifier) {
    case PSDResourceSection::MAC_PRINT_INFO:
        resource = new MAC_PRINT_INFO_1001;
        break;
    case PSDResourceSection::RESN_INFO:
        resource = new RESN_INFO_1005;
        break;
    case PSDResourceSection::ALPHA_NAMES:
        resource = new ALPHA_NAMES_1006;
        break;
    case PSDResourceSection::DISPLAY_INFO:
        resource = new DISPLAY_INFO_1007;
        break;
    case PSDResourceSection::CAPTION:
        resource = new CAPTION_1008;
        break;
    case PSDResourceSection::BORDER_INFO:
        resource = new BORDER_INFO_1009;
        break;
    case PSDResourceSection::BACKGROUND_COL:
        resource = new BACKGROUND_COL_1010;
        break;
    case PSDResourceSection::PRINT_FLAGS:
        resource = new PRINT_FLAGS_1011;
        break;
    case PSDResourceSection::GREY_HALFTONE:
        resource = new GREY_HALFTONE_1012;
        break;
    case PSDResourceSection::COLOR_HALFTONE:
        resource = new COLOR_HALFTONE_1013;
        break;
    case PSDResourceSection::DUOTONE_HALFTONE:
        resource = new DUOTONE_HALFTONE_1014;
        break;
    case PSDResourceSection::GREY_XFER:
        resource = new GREY_XFER_1015;
        break;
    case PSDResourceSection::COLOR_XFER:
        resource = new COLOR_XFER_1016;
        break;
    case PSDResourceSection::DUOTONE_XFER:
        resource = new DUOTONE_XFER_1017;
        break;
    case PSDResourceSection::DUOTONE_INFO:
        resource = new DUOTONE_INFO_1018;
        break;
    case PSDResourceSection::EFFECTIVE_BW:
        resource = new EFFECTIVE_BW_1019;
        break;
    case PSDResourceSection::EPS_OPT:
        resource = new  EPS_OPT_1021;
        break;
    case PSDResourceSection::QUICK_MASK:
        resource = new QUICK_MASK_1022;
        break;
    case PSDResourceSection::LAYER_STATE:
        resource = new  LAYER_STATE_1024;
        break;
    case PSDResourceSection::WORKING_PATH:
        resource = new WORKING_PATH_1025;
        break;
    case PSDResourceSection::LAYER_GROUP:
        resource = new LAYER_GROUP_1026;
        break;
    case PSDResourceSection::IPTC_NAA_DATA:
        resource = new IPTC_NAA_DATA_1028;
        break;
    case PSDResourceSection::IMAGE_MODE_RAW:
        resource = new IMAGE_MODE_RAW_1029;
        break;
    case PSDResourceSection::JPEG_QUAL:
        resource = new JPEG_QUAL_1030;
        break;
    case PSDResourceSection::GRID_GUIDE:
        resource = new GRID_GUIDE_1032;
        break;
    case PSDResourceSection::THUMB_RES:
        resource = new THUMB_RES_1033;
        break;
    case PSDResourceSection::COPYRIGHT_FLG:
        resource = new COPYRIGHT_FLG_1034;
        break;
    case PSDResourceSection::URL:
        resource = new URL_1035;
        break;
    case PSDResourceSection::THUMB_RES2:
        resource = new THUMB_RES2_1036;
        break;
    case PSDResourceSection::GLOBAL_ANGLE:
        resource = new GLOBAL_ANGLE_1037;
        break;
    case PSDResourceSection::COLOR_SAMPLER:
        resource = new COLOR_SAMPLER_1038;
        break;
    case PSDResourceSection::ICC_PROFILE:
        resource = new ICC_PROFILE_1039;
        break;
    case PSDResourceSection::WATERMARK:
        resource = new WATERMARK_1040;
        break;
    case PSDResourceSection::ICC_UNTAGGED:
        resource = new ICC_UNTAGGED_1041;
        break;
    case PSDResourceSection::EFFECTS_VISIBLE:
        resource = new EFFECTS_VISIBLE_1042;
        break;
    case PSDResourceSection::SPOT_HALFTONE:
        resource = new SPOT_HALFTONE_1043;
        break;
    case PSDResourceSection::DOC_IDS:
        resource = new DOC_IDS_1044;
        break;
    case PSDResourceSection::ALPHA_NAMES_UNI:
        resource = new ALPHA_NAMES_UNI_1045;
        break;
    case PSDResourceSection::IDX_COL_TAB_CNT:
        resource = new IDX_COL_TAB_CNT_1046;
        break;
    case PSDResourceSection::IDX_TRANSPARENT:
        resource = new IDX_TRANSPARENT_1047;
        break;
    case PSDResourceSection::GLOBAL_ALT:
        resource = new GLOBAL_ALT_1049;
        break;
    case PSDResourceSection::SLICES:
        resource = new SLICES_1050;
        break;
    case PSDResourceSection::WORKFLOW_URL_UNI:
        resource = new WORKFLOW_URL_UNI_1051;
        break;
    case PSDResourceSection::JUMP_TO_XPEP:
        resource = new JUMP_TO_XPEP_1052;
        break;
    case PSDResourceSection::ALPHA_ID:
        resource = new ALPHA_ID_1053;
        break;
    case PSDResourceSection::URL_LIST_UNI:
        resource = new URL_LIST_UNI_1054;
        break;
    case PSDResourceSection::VERSION_INFO:
        resource = new VERSION_INFO_1057;
        break;
    case PSDResourceSection::EXIF_DATA:
        resource = new EXIF_DATA_1058;
        break;
    case PSDResourceSection::XMP_DATA:
        resource = new XMP_DATA_1060;
        break;
    case PSDResourceSection::PATH_INFO_FIRST:
        resource = new PATH_INFO_FIRST_2000;
        break;
    case PSDResourceSection::PATH_INFO_LAST:
        resource = new PATH_INFO_LAST_2998;
        break;
    case PSDResourceSection::CLIPPING_PATH:
        resource = new CLIPPING_PATH_2999;
        break;
    case PSDResourceSection::PRINT_FLAGS_2:
        resource = new PRINT_FLAGS_2_10000;
        break;
    default:
        ;
    }

    if (resource) {
        resource->interpretBlock(data);
    }

    return valid();
}

bool PSDResourceBlock::write(QIODevice* io)
{
    Q_UNUSED(io);
    Q_ASSERT(valid());
    if (!valid()) {
        error = QString("Cannot write an invalid Resource Block");
        return false;
    }
    qFatal("TODO: implement writing the resource block");
    return false;
}

bool PSDResourceBlock::valid()
{
    if (identifier == PSDResourceSection::UNKNOWN) {
        error = QString("Unknown ID: %1").arg(identifier);
        return false;
    }
    if (!data.size() == dataSize) {
        error = QString("Needed %1 bytes, got %2 bytes of data").arg(dataSize).arg(data.length());
        return false;
    }
    return true;
}

bool RESN_INFO_1005::interpretBlock(QByteArray data)
{
    // the resolution we set on the image should be dpi; we can also set the unit on the KoDocument.
    QDataStream ds(data);
    ds.setByteOrder(QDataStream::BigEndian);

    ds >> hRes >> hResUnit >> widthUnit >> vRes >> vResUnit >> heightUnit;

    /* Resolution always recorded as pixels / inch in a fixed point implied
       decimal int32 with 16 bits before point and 16 after (i.e. cast as
       double and divide resolution by 2^16 */
    qDebug() << "hres" << hRes / 65536.0 << "vres" << vRes / 65536.0;

    hRes = hRes / 65536.0;
    vRes = vRes / 65536.0;

    return ds.atEnd();
}

bool RESN_INFO_1005::valid()
{
    return true;
}
