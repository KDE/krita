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
#ifndef PSD_RESOURCE_SECTION_H
#define PSD_RESOURCE_SECTION_H

#include <QMap>
#include <QString>

class QIODevice;
class PSDResourceBlock;

/**
 * loads all the resource sections
 */
class PSDResourceSection
{

    public:

    enum PSDResourceID {
        UNKNOWN           = 0,
        PS2_IMAGE_INFO    = 1000,         /* 0x03e8 - Obsolete - ps 2.0 image info */
        MAC_PRINT_INFO    = 1001,         /* 0x03e9 - Optional - Mac print manager print info record */
        PS2_COLOR_TAB     = 1003,         /* 0x03eb - Obsolete - ps 2.0 indexed colour table */
        RESN_INFO         = 1005,         /* 0x03ed - ResolutionInfo structure */
        ALPHA_NAMES       = 1006,         /* 0x03ee - Alpha channel names */
        DISPLAY_INFO      = 1007,         /* 0x03ef - DisplayInfo structure */
        CAPTION           = 1008,         /* 0x03f0 - Optional - Caption string */
        BORDER_INFO       = 1009,         /* 0x03f1 - Border info */
        BACKGROUND_COL    = 1010,         /* 0x03f2 - Background colour */
        PRINT_FLAGS       = 1011,         /* 0x03f3 - Print flags */
        GREY_HALFTONE     = 1012,         /* 0x03f4 - Greyscale and multichannel halftoning info */
        COLOR_HALFTONE    = 1013,         /* 0x03f5 - Colour halftoning info */
        DUOTONE_HALFTONE  = 1014,         /* 0x03f6 - Duotone halftoning info */
        GREY_XFER         = 1015,         /* 0x03f7 - Greyscale and multichannel transfer functions */
        COLOR_XFER        = 1016,         /* 0x03f8 - Colour transfer functions */
        DUOTONE_XFER      = 1017,         /* 0x03f9 - Duotone transfer functions */
        DUOTONE_INFO      = 1018,         /* 0x03fa - Duotone image information */
        EFFECTIVE_BW      = 1019,         /* 0x03fb - Effective black & white values for dot range */
        OBSOLETE_01       = 1020,         /* 0x03fc - Obsolete */
        EPS_OPT           = 1021,         /* 0x03fd - EPS options */
        QUICK_MASK        = 1022,         /* 0x03fe - Quick mask info */
        OBSOLETE_02       = 1023,         /* 0x03ff - Obsolete */
        LAYER_STATE       = 1024,         /* 0x0400 - Layer state info */
        WORKING_PATH      = 1025,         /* 0x0401 - Working path (not saved) */
        LAYER_GROUP       = 1026,         /* 0x0402 - Layers group info */
        OBSOLETE_03       = 1027,         /* 0x0403 - Obsolete */
        IPTC_NAA_DATA     = 1028,         /* 0x0404 - IPTC-NAA record (IMV4.pdf) */
        IMAGE_MODE_RAW    = 1029,         /* 0x0405 - Image mode for raw format files */
        JPEG_QUAL         = 1030,         /* 0x0406 - JPEG quality */
        GRID_GUIDE        = 1032,         /* 0x0408 - Grid & guide info */
        THUMB_RES         = 1033,         /* 0x0409 - Thumbnail resource */
        COPYRIGHT_FLG     = 1034,         /* 0x040a - Copyright flag */
        URL               = 1035,         /* 0x040b - URL string */
        THUMB_RES2        = 1036,         /* 0x040c - Thumbnail resource */
        GLOBAL_ANGLE      = 1037,         /* 0x040d - Global angle */
        COLOR_SAMPLER     = 1038,         /* 0x040e - Colour samplers resource */
        ICC_PROFILE       = 1039,         /* 0x040f - ICC Profile */
        WATERMARK         = 1040,         /* 0x0410 - Watermark */
        ICC_UNTAGGED      = 1041,         /* 0x0411 - Do not use ICC profile flag */
        EFFECTS_VISIBLE   = 1042,         /* 0x0412 - Show / hide all effects layers */
        SPOT_HALFTONE     = 1043,         /* 0x0413 - Spot halftone */
        DOC_IDS           = 1044,         /* 0x0414 - Document specific IDs */
        ALPHA_NAMES_UNI   = 1045,         /* 0x0415 - Unicode alpha names */
        IDX_COL_TAB_CNT   = 1046,         /* 0x0416 - Indexed colour table count */
        IDX_TRANSPARENT   = 1047,         /* 0x0417 - Index of transparent colour (if any) */
        GLOBAL_ALT        = 1049,         /* 0x0419 - Global altitude */
        SLICES            = 1050,         /* 0x041a - Slices */
        WORKFLOW_URL_UNI  = 1051,         /* 0x041b - Workflow URL - Unicode string */
        JUMP_TO_XPEP      = 1052,         /* 0x041c - Jump to XPEP (?) */
        ALPHA_ID          = 1053,         /* 0x041d - Alpha IDs */
        URL_LIST_UNI      = 1054,         /* 0x041e - URL list - unicode */
        VERSION_INFO      = 1057,         /* 0x0421 - Version info */
        EXIF_DATA         = 1058,         /* 0x0422 - Exif data block */
        XMP_DATA          = 1060,         /* 0x0424 - XMP data block */
        PATH_INFO_FIRST   = 2000,         /* 0x07d0 - First path info block */
        PATH_INFO_LAST    = 2998,         /* 0x0bb6 - Last path info block */
        CLIPPING_PATH     = 2999,         /* 0x0bb7 - Name of clipping path */
        PRINT_FLAGS_2     = 10000         /* 0x2710 - Print flags */
     };


    PSDResourceSection();
    ~PSDResourceSection();

    bool read(QIODevice* io);
    bool write(QIODevice* io);
    bool valid();

    QMap<PSDResourceID, PSDResourceBlock*> m_resources;

    QString error;
};

#endif // PSD_RESOURCE_SECTION_H
