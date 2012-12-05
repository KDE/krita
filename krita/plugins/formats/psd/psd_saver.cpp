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
#include "psd_saver.h"

#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoCompositeOp.h>
#include <KoUnit.h>

#include <kis_annotation.h>
#include <kis_types.h>
#include <kis_paint_layer.h>
#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include <kis_debug.h>

#include "psd.h"
#include "psd_header.h"
#include "psd_colormode_block.h"
#include "psd_utils.h"
#include "psd_resource_section.h"
#include "psd_layer_section.h"
#include "psd_resource_block.h"
#include "psd_image_data.h"



QPair<PSDColorMode, quint16> colormodelid_to_psd_colormode(const QString &colorSpaceId, const QString &colorDepthId)
{
    PSDColorMode colorMode = UNKNOWN;
    if (colorSpaceId == RGBAColorModelID.id()) {
        colorMode = RGB;
    }
    else if (colorSpaceId == CMYKAColorModelID.id()) {
        colorMode = CMYK;
    }
    else if (colorSpaceId == GrayAColorModelID.id()) {
        colorMode = Grayscale;
    }
    else if (colorSpaceId == LABAColorModelID.id()) {
        colorMode = Lab;
    }

    quint16 depth = 0;

    if (colorDepthId ==  Integer8BitsColorDepthID.id()) {
        depth = 8;
    }
    else if (colorDepthId == Integer16BitsColorDepthID.id()) {
        depth = 16;
    }
    else if (colorDepthId == Float16BitsColorDepthID.id()) {
        depth = 32;
    }
    else if (colorDepthId == Float32BitsColorDepthID.id()) {
        depth = 32;
    }

    return QPair<PSDColorMode, quint16>(colorMode, depth);
}



PSDSaver::PSDSaver(KisDoc2 *doc)
{
    m_doc = doc;
    m_image  = doc->image();
    m_job = 0;
    m_stop = false;
}

PSDSaver::~PSDSaver()
{
}

KisImageWSP PSDSaver::image()
{
    return m_image;
}


KisImageBuilder_Result PSDSaver::buildFile(const KUrl& uri)
{
    if (!m_image)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;

    // Open file for writing
    QFile f(uri.toLocalFile());
    if (!f.open(QIODevice::WriteOnly)) {
        return KisImageBuilder_RESULT_NOT_LOCAL;
    }

    // HEADER
    PSDHeader header;
    header.signature = "8BPS";
    header.version = 1;
    header.nChannels = m_image->colorSpace()->channelCount();
    header.width = m_image->width();
    header.height = m_image->height();

    QPair<PSDColorMode, quint16> colordef = colormodelid_to_psd_colormode(m_image->colorSpace()->colorModelId().id(),
                                                                          m_image->colorSpace()->colorDepthId().id());

    if (colordef.first == UNKNOWN || colordef.second == 0 || colordef.second == 32) {
        return KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE;
    }
    header.colormode = colordef.first;
    header.channelDepth = colordef.second;

    dbgFile << "header" << header << f.pos();

    if (!header.write(&f)) {
        dbgFile << "Failed to write header. Error:" << header.error << f.pos();
        return KisImageBuilder_RESULT_FAILURE;
    }

    // COLORMODE BlOCK
    PSDColorModeBlock colorModeBlock(header.colormode);
    // XXX: check for annotations that contain the duotone spec
    dbgFile << "colormode block" << f.pos();
    if (!colorModeBlock.write(&f)) {
        dbgFile << "Failed to write colormode block. Error:" << colorModeBlock.error << f.pos();
        return KisImageBuilder_RESULT_FAILURE;
    }

    // IMAGE RESOURCES SECTION
    PSDResourceSection resourceSection;

    // Add resolution block
    {
        RESN_INFO_1005 *resInfo = new RESN_INFO_1005;
        resInfo->hRes = INCH_TO_POINT(m_image->xRes());
        resInfo->vRes = INCH_TO_POINT(m_image->yRes());
        PSDResourceBlock *block = new PSDResourceBlock;
        block->resource = resInfo;
        resourceSection.resources[PSDResourceSection::RESN_INFO] = block;
    }

    // Add icc block
    {
        ICC_PROFILE_1039 *profileInfo = new ICC_PROFILE_1039;
        profileInfo->icc = m_image->profile()->rawData();
        PSDResourceBlock *block = new PSDResourceBlock;
        block->resource = profileInfo;
        resourceSection.resources[PSDResourceSection::ICC_PROFILE] = block;

    }

    // XXX: Add other blocks...

    dbgFile << "resource section" << f.pos();
    if (!resourceSection.write(&f)) {
        dbgFile << "Failed to write resource section. Error:" << resourceSection.error << f.pos();
        return KisImageBuilder_RESULT_FAILURE;
    }

    // LAYER AND MASK DATA
    // Only save layers and masks if there is more than one layer
    dbgFile << "m_image->rootLayer->childCount" << m_image->rootLayer()->childCount() << f.pos();
    if (m_image->rootLayer()->childCount() > 1) {

        PSDLayerSection layerSection(header);
        layerSection.hasTransparency = true;

        if (!layerSection.write(&f, m_image->rootLayer())) {
            dbgFile << "failed to write layer section. Error:" << layerSection.error << f.pos();
            return KisImageBuilder_RESULT_FAILURE;
        }
    }
    else {
        // else write a zero length block
        dbgFile << "No layers, saving empty layers/mask block" << f.pos();
        psdwrite(&f, (quint32)0);
    }

    // IMAGE DATA
    dbgFile << "Saving composited image" << f.pos();
    PSDImageData imagedata(&header);
    if (!imagedata.write(&f, m_image->projection())) {
        dbgFile << "Failed to write image data. Error:"  << imagedata.error;
        return KisImageBuilder_RESULT_FAILURE;
    }

    f.close();

    return KisImageBuilder_RESULT_OK;
}


void PSDSaver::cancel()
{
    m_stop = true;
}

#include "psd_saver.moc"

