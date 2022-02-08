/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef PSD_RESOURCE_SECTION_H
#define PSD_RESOURCE_SECTION_H

#include "kritapsd_export.h"

#include <QMap>
#include <QString>

class QIODevice;
class PSDResourceBlock;

/**
 * loads all the resource sections
 */
class KRITAPSD_EXPORT PSDImageResourceSection
{
public:
    enum PSDResourceID {
        UNKNOWN = 0,

        PS2_IMAGE_INFO = 1000, /* 0x03e8 - Obsolete - ps 2.0 image info */
        MAC_PRINT_INFO = 1001, /* 0x03e9 - Optional - Mac print manager print info record */
        PS2_COLOR_TAB = 1003, /* 0x03eb - Obsolete - ps 2.0 indexed colour table */
        RESN_INFO = 1005, /* 0x03ed - ResolutionInfo structure */
        ALPHA_NAMES = 1006, /* 0x03ee - Alpha channel names */
        DISPLAY_INFO = 1007, /* 0x03ef - DisplayInfo structure */
        CAPTION = 1008, /* 0x03f0 - Optional - Caption string */
        BORDER_INFO = 1009, /* 0x03f1 - Border info */

        BACKGROUND_COL = 1010, /* 0x03f2 - Background colour */
        PRINT_FLAGS = 1011, /* 0x03f3 - Print flags */
        GREY_HALFTONE = 1012, /* 0x03f4 - Greyscale and multichannel halftoning info */
        COLOR_HALFTONE = 1013, /* 0x03f5 - Colour halftoning info */
        DUOTONE_HALFTONE = 1014, /* 0x03f6 - Duotone halftoning info */
        GREY_XFER = 1015, /* 0x03f7 - Greyscale and multichannel transfer functions */
        COLOR_XFER = 1016, /* 0x03f8 - Colour transfer functions */
        DUOTONE_XFER = 1017, /* 0x03f9 - Duotone transfer functions */
        DUOTONE_INFO = 1018, /* 0x03fa - Duotone image information */
        EFFECTIVE_BW = 1019, /* 0x03fb - Effective black & white values for dot range */

        OBSOLETE_01 = 1020, /* 0x03fc - Obsolete */
        EPS_OPT = 1021, /* 0x03fd - EPS options */
        QUICK_MASK = 1022, /* 0x03fe - Quick mask info */
        OBSOLETE_02 = 1023, /* 0x03ff - Obsolete */
        LAYER_STATE = 1024, /* 0x0400 - Layer state info */
        WORKING_PATH = 1025, /* 0x0401 - Working path (not saved) */
        LAYER_GROUP = 1026, /* 0x0402 - Layers group info */
        OBSOLETE_03 = 1027, /* 0x0403 - Obsolete */
        IPTC_NAA_DATA = 1028, /* 0x0404 - IPTC-NAA record (IMV4.pdf) */
        IMAGE_MODE_RAW = 1029, /* 0x0405 - Image mode for raw format files */

        JPEG_QUAL = 1030, /* 0x0406 - JPEG quality */
        GRID_GUIDE = 1032, /* 0x0408 - Grid & guide info */
        THUMB_RES = 1033, /* 0x0409 - Thumbnail resource */
        COPYRIGHT_FLG = 1034, /* 0x040a - Copyright flag */
        URL = 1035, /* 0x040b - URL string */
        THUMB_RES2 = 1036, /* 0x040c - Thumbnail resource */
        GLOBAL_ANGLE = 1037, /* 0x040d - Global angle */
        COLOR_SAMPLER = 1038, /* 0x040e - Colour samplers resource */
        ICC_PROFILE = 1039, /* 0x040f - ICC Profile */

        WATERMARK = 1040, /* 0x0410 - Watermark */
        ICC_UNTAGGED = 1041, /* 0x0411 - Do not use ICC profile flag */
        EFFECTS_VISIBLE = 1042, /* 0x0412 - Show / hide all effects layers */
        SPOT_HALFTONE = 1043, /* 0x0413 - Spot halftone */
        DOC_IDS = 1044, /* 0x0414 - Document specific IDs */
        ALPHA_NAMES_UNI = 1045, /* 0x0415 - Unicode alpha names */
        IDX_COL_TAB_CNT = 1046, /* 0x0416 - Indexed colour table count */
        IDX_TRANSPARENT = 1047, /* 0x0417 - Index of transparent colour (if any) */
        GLOBAL_ALT = 1049, /* 0x0419 - Global altitude */

        SLICES = 1050, /* 0x041a - Slices */
        WORKFLOW_URL_UNI = 1051, /* 0x041b - Workflow URL - Unicode string */
        JUMP_TO_XPEP = 1052, /* 0x041c - Jump to XPEP (?) */
        ALPHA_ID = 1053, /* 0x041d - Alpha IDs */
        URL_LIST_UNI = 1054, /* 0x041e - URL list - unicode */
        VERSION_INFO = 1057, /* 0x0421 - Version info */
        EXIF_DATA = 1058, /* 0x0422 - (Photoshop 7.0) EXIF data 1. See http://www.kodak.com/global/plugins/acrobat/en/service/digCam/exifStandard2.pdf */
        EXIF_DATA_3 = 1059, /* 0x0423 - (Photoshop 7.0) EXIF data 3. See http://www.kodak.com/global/plugins/acrobat/en/service/digCam/exifStandard2.pdf */

        XMP_DATA = 1060, /* 0x0424 - XMP data block */
        CAPTION_DIGEST = 1061, /* 0x0425 - (Photoshop 7.0) Caption digest. 16 bytes: RSA Data Security, MD5 message-digest algorithm */
        PRINT_SCALE = 1062, /* 0x0426 - (Photoshop 7.0) Print scale. 2 bytes style (0 = centered, 1 = size to fit, 2 = user defined). 4 bytes x location
                               (floating point). 4 bytes y location (floating point). 4 bytes scale (floating point) */
        PIXEL_ASPECT_RATION = 1064, /* 0x0428 - (Photoshop CS) Pixel Aspect Ratio. 4 bytes (version = 1 or 2), 8 bytes double, x / y of a pixel. Version 2,
                                       attempting to correct values for NTSC and PAL, previously off by a factor of approx. 5%. */
        LAYER_COMPS = 1065, /* 0x0429 - (Photoshop CS) Layer Comps. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) */
        ALTERNATE_DUOTONE = 1066, /* 0x042A - (Photoshop CS) Alternate Duotone Colors. 2 bytes (version = 1), 2 bytes count, following is repeated for each
                                     count: [ Color: 2 bytes for space followed by 4 * 2 byte color component ], following this is another 2 byte count, usually
                                     256, followed by Lab colors one byte each for L, a, b. This resource is not read or used by Photoshop. */
        ALTERNATE_SPOT =
            1067, /* 0x042B - (Photoshop CS)Alternate Spot Colors. 2 bytes (version = 1), 2 bytes channel count, following is repeated for each count: 4 bytes
                     channel ID, Color: 2 bytes for space followed by 4 * 2 byte color component. This resource is not read or used by Photoshop. */
        LAYER_SELECTION_ID = 1069, /* 0x042D - (Photoshop CS2) Layer Selection ID(s). 2 bytes count, following is repeated for each count: 4 bytes layer ID */

        HDR_TONING = 1070, /* 0x042E - (Photoshop CS2) HDR Toning information */
        CS2_PRINT_INFO = 1071, /* 0x042F - (Photoshop CS2) Print info */
        LAYER_GROUP_ENABLED_ID = 1072, /* 0x0430 - (Photoshop CS2) Layer Group(s) Enabled ID. 1 byte for each layer in the document, repeated by length of the
                                          resource. NOTE: Layer groups have start and end markers */
        COLOR_SAMPLERS = 1073, /* 0x0431 - (Photoshop CS3) Color samplers resource. Also see ID 1038 for old format. See See Color samplers resource format. */
        MEASUREMENT_SCALE = 1074, /* 0x0432 - (Photoshop CS3) Measurement Scale. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) */
        TIMELINE_INFO = 1075, /* 0x0433 - (Photoshop CS3) Timeline Information. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) */
        SHEET_DISCLOSURE = 1076, /* 0x0434 - (Photoshop CS3) Sheet Disclosure. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) */
        CS3_DISPLAY_INFO = 1077, /* 0x0435 - (Photoshop CS3) DisplayInfo structure to support floating point clors. Also see ID 1007. See Appendix A in
                                    Photoshop API Guide.pdf . */
        ONION_SKINS = 1078, /* 0x0436 - (Photoshop CS3) Onion Skins. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) */

        COUNT_INFO = 1080, /* 0x0438 - (Photoshop CS4) Count Information. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure)
                              Information about the count in the document. See the Count Tool. */
        CS5_PRINT_INFO = 1082, /* 0x043A - (Photoshop CS5) Print Information. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure)
                                  Information about the current print settings in the document. The color management options. */
        CS5_PRINT_STYLE = 1083, /* 0x043B - (Photoshop CS5) Print Style. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure)
                                   Information about the current print style in the document. The printing marks, labels, ornaments, etc. */
        CS5_NSPrintInfo = 1084, /* 0x043C - (Photoshop CS5) Macintosh NSPrintInfo. Variable OS specific info for Macintosh. NSPrintInfo. It is recommended that
                                   you do not interpret or use this data. */
        CS5_WIN_DEVMODE = 1085, /* 0x043D - (Photoshop CS5) Windows DEVMODE. Variable OS specific info for Windows. DEVMODE. It is recommended that you do not
                                   interpret or use this data. */
        CS6_AUTOSAVE_FILE_PATH =
            1086, /* 0x043E - (Photoshop CS6) Auto Save File Path. Unicode string. It is recommended that you do not interpret or use this data. */
        CS6_AUTOSAVE_FORMAT =
            1087, /* 0x043F - (Photoshop CS6) Auto Save Format. Unicode string. It is recommended that you do not interpret or use this data. */
        CC_PATH_SELECTION_SATE = 1088, /* 0x0440 - (Photoshop CC) Path Selection State. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor
                                          structure) Information about the current path selection state. */

        PATH_INFO_FIRST = 2000, /* 0x07d0 - First path info block */
        PATH_INFO_LAST = 2998, /* 0x0bb6 - Last path info block */
        CLIPPING_PATH = 2999, /* 0x0bb7 - Name of clipping path */

        CC_ORIGIN_PATH_INFO = 3000, /* 0x0BB8 (Photoshop CC) Origin Path Info. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure)
                                       Information about the origin path data. */

        PLUGIN_RESOURCE_START = 4000, /* 0x0FA0-0x1387 Plug-In resource(s). Resources added by a plug-in. See the plug-in API found in the SDK documentation  */
        PLUGIN_RESOURCE_END = 4999, /* Last plug-in resource */

        IMAGE_READY_VARS = 7000, /* 0x1B58 Image Ready variables. XML representation of variables definition */
        IMAGE_READY_DATA_SETS = 7001, /* 0x1B59 Image Ready data sets */

        LIGHTROOM_WORKFLOW = 8000, /* 0x1F40 (Photoshop CS3) Lightroom workflow, if present the document is in the middle of a Lightroom workflow. */

        PRINT_FLAGS_2 = 10000 /* 0x2710 - Print flags */
    };

    PSDImageResourceSection();
    ~PSDImageResourceSection();

    bool read(QIODevice &io);
    bool write(QIODevice &io);
    bool valid();

    static QString idToString(PSDResourceID id);

    QMap<PSDResourceID, PSDResourceBlock *> resources;

    QString error;
};

#endif // PSD_RESOURCE_SECTION_H
