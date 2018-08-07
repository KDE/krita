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
#include "psd_resource_section.h"

#include <kis_debug.h>
#include <QIODevice>
#include <QBuffer>

#include "psd_utils.h"
#include "psd_resource_block.h"

PSDImageResourceSection::PSDImageResourceSection()
{
}

PSDImageResourceSection::~PSDImageResourceSection()
{
    resources.clear();
}

bool PSDImageResourceSection::read(QIODevice* io)
{
    quint32 resourceSectionLength = 0;
    if (!psdread(io, &resourceSectionLength)) {
        error = "Could not read image resource section length";
        return false;
    }

    dbgFile << "Image Resource Sectionlength:" << resourceSectionLength << ", starts at:" << io->pos();

    QByteArray ba = io->read(resourceSectionLength);
    if ((quint32)ba.size() != resourceSectionLength) {
        error = "Could not read all resources";
        return false;
    }

    QBuffer buf;
    buf.setBuffer(&ba);
    buf.open(QBuffer::ReadOnly);

    while (!buf.atEnd()) {
        PSDResourceBlock* block = new PSDResourceBlock();
        if (!block->read(&buf)) {
            error = "Error reading block: " + block->error;
            dbgFile << error << ", skipping.";
            continue;
        }
        dbgFile << "resource block created. Type:" << block->identifier
                << "name" << block->name
                << "size" << block->dataSize
                << "," << buf.bytesAvailable() << "bytes to go";

        resources[(PSDResourceID)block->identifier] = block;
    }

    dbgFile << "Read" << resources.size() << "Image Resource Blocks";

    return valid();
}

bool PSDImageResourceSection::write(QIODevice* io)
{
    Q_UNUSED(io);

    if (!valid()) {
        error = "Resource Section is Invalid";
        return false;
    }
    // First write all the sections
    QByteArray ba;
    QBuffer buf;
    buf.setBuffer(&ba);
    buf.open(QBuffer::WriteOnly);

    Q_FOREACH (PSDResourceBlock* block, resources) {
        if (!block->write(&buf)) {
            error = block->error;
            return false;
        }
    }

    buf.close();

    // Then get the size
    quint32 resourceBlockLength = ba.size();
    dbgFile << "resource section has size" << resourceBlockLength;
    psdwrite(io, resourceBlockLength);

    // and write the whole buffer;
    return (io->write(ba.constData(), ba.size()) == resourceBlockLength);

}

bool PSDImageResourceSection::valid()
{
    return true;
}

QString PSDImageResourceSection::idToString(PSDImageResourceSection::PSDResourceID id)
{
    switch(id) {
    case UNKNOWN: return "Unknown";

    case PS2_IMAGE_INFO    : return "0x03e8 - Obsolete - ps 2.0 image info";
    case MAC_PRINT_INFO    : return "0x03e9 - Optional - Mac print manager print info record";
    case PS2_COLOR_TAB     : return "0x03eb - Obsolete - ps 2.0 indexed colour table";
    case RESN_INFO         : return "0x03ed - ResolutionInfo structure";
    case ALPHA_NAMES       : return "0x03ee - Alpha channel names";
    case DISPLAY_INFO      : return "0x03ef - DisplayInfo structure";
    case CAPTION           : return "0x03f0 - Optional - Caption string";
    case BORDER_INFO       : return "0x03f1 - Border info";

    case BACKGROUND_COL    : return "0x03f2 - Background colour";
    case PRINT_FLAGS       : return "0x03f3 - Print flags";
    case GREY_HALFTONE     : return "0x03f4 - Greyscale and multichannel halftoning info";
    case COLOR_HALFTONE    : return "0x03f5 - Colour halftoning info";
    case DUOTONE_HALFTONE  : return "0x03f6 - Duotone halftoning info";
    case GREY_XFER         : return "0x03f7 - Greyscale and multichannel transfer functions";
    case COLOR_XFER        : return "0x03f8 - Colour transfer functions";
    case DUOTONE_XFER      : return "0x03f9 - Duotone transfer functions";
    case DUOTONE_INFO      : return "0x03fa - Duotone image information";
    case EFFECTIVE_BW      : return "0x03fb - Effective black & white values for dot range";

    case OBSOLETE_01       : return "0x03fc - Obsolete";
    case EPS_OPT           : return "0x03fd - EPS options";
    case QUICK_MASK        : return "0x03fe - Quick mask info";
    case OBSOLETE_02       : return "0x03ff - Obsolete";
    case LAYER_STATE       : return "0x0400 - Layer state info";
    case WORKING_PATH      : return "0x0401 - Working path (not saved)";
    case LAYER_GROUP       : return "0x0402 - Layers group info";
    case OBSOLETE_03       : return "0x0403 - Obsolete";
    case IPTC_NAA_DATA     : return "0x0404 - IPTC-NAA record (IMV4.pdf)";
    case IMAGE_MODE_RAW    : return "0x0405 - Image mode for raw format files";

    case JPEG_QUAL         : return "0x0406 - JPEG quality";
    case GRID_GUIDE        : return "0x0408 - Grid & guide info";
    case THUMB_RES         : return "0x0409 - Thumbnail resource";
    case COPYRIGHT_FLG     : return "0x040a - Copyright flag";
    case URL               : return "0x040b - URL string";
    case THUMB_RES2        : return "0x040c - Thumbnail resource";
    case GLOBAL_ANGLE      : return "0x040d - Global angle";
    case COLOR_SAMPLER     : return "0x040e - Colour samplers resource";
    case ICC_PROFILE       : return "0x040f - ICC Profile";

    case WATERMARK         : return "0x0410 - Watermark";
    case ICC_UNTAGGED      : return "0x0411 - Do not use ICC profile flag";
    case EFFECTS_VISIBLE   : return "0x0412 - Show / hide all effects layers";
    case SPOT_HALFTONE     : return "0x0413 - Spot halftone";
    case DOC_IDS           : return "0x0414 - Document specific IDs";
    case ALPHA_NAMES_UNI   : return "0x0415 - Unicode alpha names";
    case IDX_COL_TAB_CNT   : return "0x0416 - Indexed colour table count";
    case IDX_TRANSPARENT   : return "0x0417 - Index of transparent colour (if any)";
    case GLOBAL_ALT        : return "0x0419 - Global altitude";

    case SLICES            : return "0x041a - Slices";
    case WORKFLOW_URL_UNI  : return "0x041b - Workflow URL - Unicode string";
    case JUMP_TO_XPEP      : return "0x041c - Jump to XPEP (?)";
    case ALPHA_ID          : return "0x041d - Alpha IDs";
    case URL_LIST_UNI      : return "0x041e - URL list - unicode";
    case VERSION_INFO      : return "0x0421 - Version info";
    case EXIF_DATA         : return "0x0422 - (Photoshop 7.0) EXIF data 1. See http://www.kodak.com/global/plugins/acrobat/en/service/digCam/exifStandard2.pdf";
    case EXIF_DATA_3       : return "0x0423 - (Photoshop 7.0) EXIF data 3. See http://www.kodak.com/global/plugins/acrobat/en/service/digCam/exifStandard2.pdf";

    case XMP_DATA          : return "0x0424 - XMP data block";
    case CAPTION_DIGEST    : return "0x0425 - (Photoshop 7.0) Caption digest. 16 bytes: RSA Data Security, MD5 message-digest algorithm";
    case PRINT_SCALE       : return "0x0426 - (Photoshop 7.0) Print scale. 2 bytes style (0 = centered, 1 = size to fit, 2 = user defined). 4 bytes x location (floating point). 4 bytes y location (floating point). 4 bytes scale (floating point)";
    case PIXEL_ASPECT_RATION : return "0x0428 - (Photoshop CS) Pixel Aspect Ratio. 4 bytes (version = 1 or 2), 8 bytes double, x / y of a pixel. Version 2, attempting to correct values for NTSC and PAL, previously off by a factor of approx. 5%.";
    case LAYER_COMPS       : return "0x0429 - (Photoshop CS) Layer Comps. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure)";
    case ALTERNATE_DUOTONE : return "0x042A - (Photoshop CS) Alternate Duotone Colors. 2 bytes (version = 1), 2 bytes count, following is repeated for each count: [ Color: 2 bytes for space followed by 4 * 2 byte color component ], following this is another 2 byte count, usually 256, followed by Lab colors one byte each for L, a, b. This resource is not read or used by Photoshop.";
    case ALTERNATE_SPOT    : return "0x042B - (Photoshop CS)Alternate Spot Colors. 2 bytes (version = 1), 2 bytes channel count, following is repeated for each count: 4 bytes channel ID, Color: 2 bytes for space followed by 4 * 2 byte color component. This resource is not read or used by Photoshop.";
    case LAYER_SELECTION_ID : return "0x042D - (Photoshop CS2) Layer Selection ID(s). 2 bytes count, following is repeated for each count: 4 bytes layer ID";

    case HDR_TONING        : return "0x042E - (Photoshop CS2) HDR Toning information";
    case CS2_PRINT_INFO    : return "0x042F - (Photoshop CS2) Print info";
    case LAYER_GROUP_ENABLED_ID: return "0x0430 - (Photoshop CS2) Layer Group(s) Enabled ID. 1 byte for each layer in the document, repeated by length of the resource. NOTE: Layer groups have start and end markers";
    case COLOR_SAMPLERS     : return "0x0431 - (Photoshop CS3) Color samplers resource. Also see ID 1038 for old format. See See Color samplers resource format.";
    case MEASUREMENT_SCALE : return "0x0432 - (Photoshop CS3) Measurement Scale. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure)";
    case TIMELINE_INFO     : return "0x0433 - (Photoshop CS3) Timeline Information. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure)";
    case SHEET_DISCLOSURE  : return "0x0434 - (Photoshop CS3) Sheet Disclosure. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure)";
    case CS3_DISPLAY_INFO  : return "0x0435 - (Photoshop CS3) DisplayInfo structure to support floating point clors. Also see ID 1007. See Appendix A in Photoshop API Guide.pdf .";
    case ONION_SKINS       : return "0x0436 - (Photoshop CS3) Onion Skins. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure)";

    case COUNT_INFO        : return "0x0438 - (Photoshop CS4) Count Information. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) Information about the count in the document. See the Count Tool.";
    case CS5_PRINT_INFO    : return "0x043A - (Photoshop CS5) Print Information. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) Information about the current print settings in the document. The color management options.";
    case CS5_PRINT_STYLE   : return "0x043B - (Photoshop CS5) Print Style. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) Information about the current print style in the document. The printing marks, labels, ornaments, etc.";
    case CS5_NSPrintInfo   : return "0x043C - (Photoshop CS5) Macintosh NSPrintInfo. Variable OS specific info for Macintosh. NSPrintInfo. It is recommended that you do not interpret or use this data.";
    case CS5_WIN_DEVMODE   : return "0x043D - (Photoshop CS5) Windows DEVMODE. Variable OS specific info for Windows. DEVMODE. It is recommended that you do not interpret or use this data.";
    case CS6_AUTOSAVE_FILE_PATH : return "0x043E - (Photoshop CS6) Auto Save File Path. Unicode string. It is recommended that you do not interpret or use this data.";
    case CS6_AUTOSAVE_FORMAT : return "0x043F - (Photoshop CS6) Auto Save Format. Unicode string. It is recommended that you do not interpret or use this data.";
    case CC_PATH_SELECTION_SATE : return "0x0440 - (Photoshop CC) Path Selection State. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) Information about the current path selection state.";

    case PATH_INFO_FIRST   : return "0x07d0 - First path info block";
    case PATH_INFO_LAST    : return "0x0bb6 - Last path info block";
    case CLIPPING_PATH     : return "0x0bb7 - Name of clipping path";

    case CC_ORIGIN_PATH_INFO : return "0x0BB8 (Photoshop CC) Origin Path Info. 4 bytes (descriptor version = 16), Descriptor (see See Descriptor structure) Information about the origin path data.";

    case PLUGIN_RESOURCE_START : return "0x0FA0-0x1387 Plug-In resource(s). Resources added by a plug-in. See the plug-in API found in the SDK documentation ";
    case PLUGIN_RESOURCE_END : return "Last plug-in resource";

    case IMAGE_READY_VARS   : return "0x1B58 Image Ready variables. XML representation of variables definition";
    case IMAGE_READY_DATA_SETS : return "0x1B59 Image Ready data sets";

    case LIGHTROOM_WORKFLOW : return "0x1F40 (Photoshop CS3) Lightroom workflow, if present the document is in the middle of a Lightroom workflow.";

    case PRINT_FLAGS_2     : return "0x2710 - Print flags";
    default: {
        if (id > PATH_INFO_FIRST && id < PATH_INFO_LAST) return "Path Info Block";
        if (id > PLUGIN_RESOURCE_START && id < PLUGIN_RESOURCE_END) return "Plug-In Resource";
    }
    };
    return QString("Unknown Resource Block: %1").arg(id);
}
