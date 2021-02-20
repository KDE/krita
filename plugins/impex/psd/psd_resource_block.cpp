/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_resource_block.h"

#include <QIODevice>
#include <QBuffer>
#include <QDataStream>

#include <kis_debug.h>

#include "psd_utils.h"
#include "psd_resource_section.h"

PSDResourceBlock::PSDResourceBlock()
    : KisAnnotation("PSD Resource Block", "", QByteArray())
    , identifier(PSDImageResourceSection::UNKNOWN)
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

    dbgFile << "\tresource block identifier" << PSDImageResourceSection::idToString((PSDImageResourceSection::PSDResourceID)identifier) << identifier;

    m_type = QString("PSD Resource Block: %1").arg(identifier);


    if (!psdread_pascalstring(io, name, 2)) {
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

    m_description = PSDImageResourceSection::idToString((PSDImageResourceSection::PSDResourceID)identifier);

    data = io->read(dataSize);
    if (data.size() != (int)dataSize) {
        error = QString("Could not read data for resource block with name %1 of type %2").arg(name).arg(identifier);
        return false;
    }

    m_annotation = data;

    switch (identifier) {
//    case PSDImageResourceSection::MAC_PRINT_INFO:
//        resource = new MAC_PRINT_INFO_1001;
//        break;
    case PSDImageResourceSection::RESN_INFO:
        resource = new RESN_INFO_1005;
        break;
//    case PSDImageResourceSection::ALPHA_NAMES:
//        resource = new ALPHA_NAMES_1006;
//        break;
//    case PSDImageResourceSection::DISPLAY_INFO:
//        resource = new DISPLAY_INFO_1007;
//        break;
//    case PSDImageResourceSection::CAPTION:
//        resource = new CAPTION_1008;
//        break;
//    case PSDImageResourceSection::BORDER_INFO:
//        resource = new BORDER_INFO_1009;
//        break;
//    case PSDImageResourceSection::BACKGROUND_COL:
//        resource = new BACKGROUND_COL_1010;
//        break;
//    case PSDImageResourceSection::PRINT_FLAGS:
//        resource = new PRINT_FLAGS_1011;
//        break;
//    case PSDImageResourceSection::GREY_HALFTONE:
//        resource = new GREY_HALFTONE_1012;
//        break;
//    case PSDImageResourceSection::COLOR_HALFTONE:
//        resource = new COLOR_HALFTONE_1013;
//        break;
//    case PSDImageResourceSection::DUOTONE_HALFTONE:
//        resource = new DUOTONE_HALFTONE_1014;
//        break;
//    case PSDImageResourceSection::GREY_XFER:
//        resource = new GREY_XFER_1015;
//        break;
//    case PSDImageResourceSection::COLOR_XFER:
//        resource = new COLOR_XFER_1016;
//        break;
//    case PSDImageResourceSection::DUOTONE_XFER:
//        resource = new DUOTONE_XFER_1017;
//        break;
//    case PSDImageResourceSection::DUOTONE_INFO:
//        resource = new DUOTONE_INFO_1018;
//        break;
//    case PSDImageResourceSection::EFFECTIVE_BW:
//        resource = new EFFECTIVE_BW_1019;
//        break;
//    case PSDImageResourceSection::EPS_OPT:
//        resource = new  EPS_OPT_1021;
//        break;
//    case PSDImageResourceSection::QUICK_MASK:
//        resource = new QUICK_MASK_1022;
//        break;
//    case PSDImageResourceSection::LAYER_STATE:
//        resource = new  LAYER_STATE_1024;
//        break;
//    case PSDImageResourceSection::WORKING_PATH:
//        resource = new WORKING_PATH_1025;
//        break;
//    case PSDImageResourceSection::LAYER_GROUP:
//        resource = new LAYER_GROUP_1026;
//        break;
//    case PSDImageResourceSection::IPTC_NAA_DATA:
//        resource = new IPTC_NAA_DATA_1028;
//        break;
//    case PSDImageResourceSection::IMAGE_MODE_RAW:
//        resource = new IMAGE_MODE_RAW_1029;
//        break;
//    case PSDImageResourceSection::JPEG_QUAL:
//        resource = new JPEG_QUAL_1030;
//        break;
//    case PSDImageResourceSection::GRID_GUIDE:
//        resource = new GRID_GUIDE_1032;
//        break;
//    case PSDImageResourceSection::THUMB_RES:
//        resource = new THUMB_RES_1033;
//        break;
//    case PSDImageResourceSection::COPYRIGHT_FLG:
//        resource = new COPYRIGHT_FLG_1034;
//        break;
//    case PSDImageResourceSection::URL:
//        resource = new URL_1035;
//        break;
//    case PSDImageResourceSection::THUMB_RES2:
//        resource = new THUMB_RES2_1036;
//        break;
    case PSDImageResourceSection::GLOBAL_ANGLE:
        resource = new GLOBAL_ANGLE_1037;
        break;
//    case PSDImageResourceSection::COLOR_SAMPLER:
//        resource = new COLOR_SAMPLER_1038;
//        break;
    case PSDImageResourceSection::ICC_PROFILE:
        resource = new ICC_PROFILE_1039;
        break;
//    case PSDImageResourceSection::WATERMARK:
//        resource = new WATERMARK_1040;
//        break;
//    case PSDImageResourceSection::ICC_UNTAGGED:
//        resource = new ICC_UNTAGGED_1041;
//        break;
//    case PSDImageResourceSection::EFFECTS_VISIBLE:
//        resource = new EFFECTS_VISIBLE_1042;
//        break;
//    case PSDImageResourceSection::SPOT_HALFTONE:
//        resource = new SPOT_HALFTONE_1043;
//        break;
//    case PSDImageResourceSection::DOC_IDS:
//        resource = new DOC_IDS_1044;
//        break;
//    case PSDImageResourceSection::ALPHA_NAMES_UNI:
//        resource = new ALPHA_NAMES_UNI_1045;
//        break;
//    case PSDImageResourceSection::IDX_COL_TAB_CNT:
//        resource = new IDX_COL_TAB_CNT_1046;
//        break;
//    case PSDImageResourceSection::IDX_TRANSPARENT:
//        resource = new IDX_TRANSPARENT_1047;
//        break;
    case PSDImageResourceSection::GLOBAL_ALT:
        resource = new GLOBAL_ALT_1049;
        break;
//    case PSDImageResourceSection::SLICES:
//        resource = new SLICES_1050;
//        break;
//    case PSDImageResourceSection::WORKFLOW_URL_UNI:
//        resource = new WORKFLOW_URL_UNI_1051;
//        break;
//    case PSDImageResourceSection::JUMP_TO_XPEP:
//        resource = new JUMP_TO_XPEP_1052;
//        break;
//    case PSDImageResourceSection::ALPHA_ID:
//        resource = new ALPHA_ID_1053;
//        break;
//    case PSDImageResourceSection::URL_LIST_UNI:
//        resource = new URL_LIST_UNI_1054;
//        break;
//    case PSDImageResourceSection::VERSION_INFO:
//        resource = new VERSION_INFO_1057;
//        break;
//    case PSDImageResourceSection::EXIF_DATA:
//        resource = new EXIF_DATA_1058;
//        break;
//    case PSDImageResourceSection::XMP_DATA:
//        resource = new XMP_DATA_1060;
//        break;
//    case PSDImageResourceSection::PATH_INFO_FIRST:
//        resource = new PATH_INFO_FIRST_2000;
//        break;
//    case PSDImageResourceSection::PATH_INFO_LAST:
//        resource = new PATH_INFO_LAST_2998;
//        break;
//    case PSDImageResourceSection::CLIPPING_PATH:
//        resource = new CLIPPING_PATH_2999;
//        break;
//    case PSDImageResourceSection::PRINT_FLAGS_2:
//        resource = new PRINT_FLAGS_2_10000;
//        break;
    default:
        ;
    }

    if (resource) {
        resource->interpretBlock(data);
    }

    return valid();
}

bool PSDResourceBlock::write(QIODevice* io) const
{

    dbgFile << "Writing Resource Block" << PSDImageResourceSection::idToString((PSDImageResourceSection::PSDResourceID)identifier) << identifier;

    if (resource && !resource->valid()) {
        error = QString("Cannot write an invalid Resource Block");
        return false;
    }

    if (identifier == PSDImageResourceSection::LAYER_STATE ||
        identifier == PSDImageResourceSection::LAYER_GROUP ||
        identifier == PSDImageResourceSection::LAYER_COMPS ||
        identifier == PSDImageResourceSection::LAYER_GROUP_ENABLED_ID ||
        identifier == PSDImageResourceSection::LAYER_SELECTION_ID) {

        /**
         * We can actually handle LAYER_SELECTION_ID. It consists
         * of a number of layers and a list of IDs to select, which
         * are retrieved from 'lyid' additional layer block.
         */
        dbgFile << "Skip writing resource block" << identifier << displayText();
        return true;
    }

    QByteArray ba;

    // createBlock returns true by default but does not change the data.
    if (resource && !resource->createBlock(ba)) {
        error = resource->error;
        return false;
    }
    else if (!resource) {
        // reconstruct from the data
        QBuffer buf(&ba);
        buf.open(QBuffer::WriteOnly);
        buf.write("8BIM", 4);
        psdwrite(&buf, identifier);
        psdwrite_pascalstring(&buf, name);
        psdwrite(&buf, dataSize);
        buf.write(data);
        buf.close();
    }
    if (io->write(ba.constData(), ba.size()) != ba.size()) {
        error = QString("Could not write complete resource");
        return false;
    }

    return true;
}

bool PSDResourceBlock::valid()
{
    if (identifier == PSDImageResourceSection::UNKNOWN) {
        error = QString("Unknown ID: %1").arg(identifier);
        return false;
    }
    if (data.size() != (int)dataSize) {
        error = QString("Needed %1 bytes, got %2 bytes of data").arg(dataSize).arg(data.length());
        return false;
    }
    return true;
}

bool RESN_INFO_1005::interpretBlock(QByteArray data)
{
    dbgFile << "Reading RESN_INFO_1005";

    // the resolution we set on the image should be dpi; we can also set the unit on the KisDocument.
    QDataStream ds(data);
    ds.setByteOrder(QDataStream::BigEndian);

    ds >> hRes >> hResUnit >> widthUnit >> vRes >> vResUnit >> heightUnit;

    /* Resolution always recorded as pixels / inch in a fixed point implied
       decimal int32 with 16 bits before point and 16 after (i.e. cast as
       double and divide resolution by 2^16 */
    dbgFile << "hres" << hRes << "vres" << vRes;

    hRes = hRes / 65536.0;
    vRes = vRes / 65536.0;

    dbgFile << hRes << hResUnit << widthUnit << vRes << vResUnit << heightUnit;

    return ds.atEnd();
}

bool RESN_INFO_1005::createBlock(QByteArray & data)
{
    dbgFile << "Writing RESN_INFO_1005";
    QBuffer buf(&data);

    startBlock(buf, PSDImageResourceSection::RESN_INFO, 16);

    // Convert to 16.16 fixed point
    Fixed h = hRes * 65536.0 + 0.5;
    dbgFile << "h" << h << "hRes" << hRes;
    psdwrite(&buf, (quint32)h);
    psdwrite(&buf, hResUnit);
    psdwrite(&buf, widthUnit);

    // Convert to 16.16 fixed point
    Fixed v = vRes * 65536.0 + 0.5;
    dbgFile << "v" << v << "vRes" << vRes;
    psdwrite(&buf, (quint32)v);
    psdwrite(&buf, vResUnit);
    psdwrite(&buf, heightUnit);

    buf.close();

    return true;
}

bool ICC_PROFILE_1039::interpretBlock(QByteArray data)
{
    dbgFile << "Reading ICC_PROFILE_1039";

    icc = data;

    return true;

}

bool ICC_PROFILE_1039::createBlock(QByteArray &data)
{
    dbgFile << "Writing ICC_PROFILE_1039";
    if (icc.size() == 0) {
        error = "ICC_PROFILE_1039: Trying to save an empty profile";
        return false;
    }
    QBuffer buf(&data);
    startBlock(buf, PSDImageResourceSection::ICC_PROFILE, icc.size());
    buf.write(icc.constData(), icc.size());
    buf.close();


    return true;
}

