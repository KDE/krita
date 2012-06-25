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
#ifndef PSD_RESOURCE_BLOCK_H
#define PSD_RESOURCE_BLOCK_H

class QIODevice;

#include <QString>
#include "psd.h"
#include "psd_resource_section.h"

/**
 * @brief The PSDResourceInterpreter class interprets the data in a psd resource block
 */
class PSDInterpretedResource
{
public:

    virtual ~PSDInterpretedResource() {};

    virtual bool interpretBlock(QByteArray /*data*/) { return true; }
    virtual bool valid() { return true; }
};

/**
 * Contains the unparsed contents of the image resource blocks
 *
 * XXX: make KisAnnotations out of the resource blocks so we can load/save them. Need to
 *      expand KisAnnotation to make that work.
 */
class PSDResourceBlock //: public KisAnnotation
{

public:
    PSDResourceBlock();
    ~PSDResourceBlock()
    {
        delete resource;
    }

    bool read(QIODevice* io);
    bool write(QIODevice* io);
    bool valid();

    quint16     identifier;
    QString     name;
    quint32     dataSize;
    QByteArray  data;

    PSDInterpretedResource *resource;

    QString error;
};


/* 0x03e9 - Optional - Mac print manager print info record */
class MAC_PRINT_INFO_1001 : public PSDInterpretedResource
{

};

/* 0x03ed - ResolutionInfo structure */
class RESN_INFO_1005 : public PSDInterpretedResource
{
    virtual bool interpretBlock(QByteArray data);
    bool valid();

    Fixed   hRes;
    quint16 hResUnit;
    quint16 widthUnit;
    Fixed   vRes;
    quint16 vResUnit;
    quint16 heightUnit;

};


/* 0x03ee - Alpha channel names */
class ALPHA_NAMES_1006 : public PSDInterpretedResource
{

};


/* 0x03ef - DisplayInfo structure */
class DISPLAY_INFO_1007 : public PSDInterpretedResource
{

};


/* 0x03f0 - Optional - Caption string */
class CAPTION_1008 : public PSDInterpretedResource
{

};


/* 0x03f1 - Border info */
class BORDER_INFO_1009 : public PSDInterpretedResource
{

};


/* 0x03f2 - Background colour */
class BACKGROUND_COL_1010 : public PSDInterpretedResource
{

};


/* 0x03f3 - Print flags */
class PRINT_FLAGS_1011 : public PSDInterpretedResource
{

};


/* 0x03f4 - Greyscale and multichannel halftoning info */
class GREY_HALFTONE_1012 : public PSDInterpretedResource
{

};


/* 0x03f5 - Colour halftoning info */
class COLOR_HALFTONE_1013 : public PSDInterpretedResource
{

};


/* 0x03f6 - Duotone halftoning info */
class DUOTONE_HALFTONE_1014 : public PSDInterpretedResource
{

};


/* 0x03f7 - Greyscale and multichannel transfer functions */
class GREY_XFER_1015 : public PSDInterpretedResource
{

};


/* 0x03f8 - Colour transfer functions */
class COLOR_XFER_1016 : public PSDInterpretedResource
{

};


/* 0x03f9 - Duotone transfer functions */
class DUOTONE_XFER_1017 : public PSDInterpretedResource
{

};


/* 0x03fa - Duotone image information */
class DUOTONE_INFO_1018 : public PSDInterpretedResource
{

};


/* 0x03fb - Effective black & white values for dot range */
class EFFECTIVE_BW_1019 : public PSDInterpretedResource
{

};


/* 0x03fd - EPS options */
class EPS_OPT_1021 : public PSDInterpretedResource
{

};


/* 0x03fe - Quick mask info */
class QUICK_MASK_1022 : public PSDInterpretedResource
{

};


/* 0x0400 - Layer state info */
class LAYER_STATE_1024 : public PSDInterpretedResource
{

};


/* 0x0401 - Working path (not saved) */
class WORKING_PATH_1025 : public PSDInterpretedResource
{

};


/* 0x0402 - Layers group info */
class LAYER_GROUP_1026 : public PSDInterpretedResource
{

};


/* 0x0404 - IPTC-NAA record (IMV4.pdf) */
class IPTC_NAA_DATA_1028 : public PSDInterpretedResource
{

};


/* 0x0405 - Image mode for raw format files */
class IMAGE_MODE_RAW_1029 : public PSDInterpretedResource
{

};


/* 0x0406 - JPEG quality */
class JPEG_QUAL_1030 : public PSDInterpretedResource
{

};


/* 0x0408 - Grid & guide info */
class GRID_GUIDE_1032 : public PSDInterpretedResource
{

};


/* 0x0409 - Thumbnail resource */
class THUMB_RES_1033 : public PSDInterpretedResource
{

};


/* 0x040a - Copyright flag */
class COPYRIGHT_FLG_1034 : public PSDInterpretedResource
{

};


/* 0x040b - URL string */
class URL_1035 : public PSDInterpretedResource
{

};


/* 0x040c - Thumbnail resource */
class THUMB_RES2_1036 : public PSDInterpretedResource
{

};


/* 0x040d - Global angle */
class GLOBAL_ANGLE_1037 : public PSDInterpretedResource
{

};


/* 0x040e - Colour samplers resource */
class COLOR_SAMPLER_1038 : public PSDInterpretedResource
{

};


/* 0x040f - ICC Profile */
class ICC_PROFILE_1039 : public PSDInterpretedResource
{

};


/* 0x0410 - Watermark */
class WATERMARK_1040 : public PSDInterpretedResource
{

};


/* 0x0411 - Do not use ICC profile flag */
class ICC_UNTAGGED_1041 : public PSDInterpretedResource
{

};


/* 0x0412 - Show / hide all effects layers */
class EFFECTS_VISIBLE_1042 : public PSDInterpretedResource
{

};


/* 0x0413 - Spot halftone */
class SPOT_HALFTONE_1043 : public PSDInterpretedResource
{

};


/* 0x0414 - Document specific IDs */
class DOC_IDS_1044 : public PSDInterpretedResource
{

};


/* 0x0415 - Unicode alpha names */
class ALPHA_NAMES_UNI_1045 : public PSDInterpretedResource
{

};


/* 0x0416 - Indexed colour table count */
class IDX_COL_TAB_CNT_1046 : public PSDInterpretedResource
{

};


/* 0x0417 - Index of transparent colour (if any) */
class IDX_TRANSPARENT_1047 : public PSDInterpretedResource
{

};


/* 0x0419 - Global altitude */
class GLOBAL_ALT_1049 : public PSDInterpretedResource
{

};


/* 0x041a - Slices */
class SLICES_1050 : public PSDInterpretedResource
{

};


/* 0x041b - Workflow URL - Unicode string */
class WORKFLOW_URL_UNI_1051 : public PSDInterpretedResource
{

};


/* 0x041c - Jump to XPEP (?) */
class JUMP_TO_XPEP_1052 : public PSDInterpretedResource
{

};


/* 0x041d - Alpha IDs */
class ALPHA_ID_1053 : public PSDInterpretedResource
{

};

/* 0x041e - URL list - unicode */
class URL_LIST_UNI_1054 : public PSDInterpretedResource
{

};

/* 0x0421 - Version info */
class VERSION_INFO_1057 : public PSDInterpretedResource
{

};


/* 0x0422 - Exif data block */
class EXIF_DATA_1058 : public PSDInterpretedResource
{

};


/* 0x0424 - XMP data block */
class XMP_DATA_1060 : public PSDInterpretedResource
{

};


/* 0x07d0 - First path info block */
class PATH_INFO_FIRST_2000 : public PSDInterpretedResource
{

};


/* 0x0bb6 - Last path info block */
class PATH_INFO_LAST_2998 : public PSDInterpretedResource
{

};


/* 0x0bb7 - Name of clipping path */
class CLIPPING_PATH_2999 : public PSDInterpretedResource
{

};


/* 0x2710 - Print flags */
class PRINT_FLAGS_2_10000 : public PSDInterpretedResource
{

};


        #endif // PSD_RESOURCE_BLOCK_H
