/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_saver.h"

#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoCompositeOp.h>
#include <KoUnit.h>

#include <QFileInfo>

#include <kis_annotation.h>
#include <kis_types.h>
#include <kis_paint_layer.h>
#include "kis_painter.h"
#include <KisDocument.h>
#include <kis_image.h>
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





QPair<psd_color_mode, quint16> colormodelid_to_psd_colormode(const QString &colorSpaceId, const QString &colorDepthId)
{
    psd_color_mode colorMode = COLORMODE_UNKNOWN;
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

    return QPair<psd_color_mode, quint16>(colorMode, depth);
}



PSDSaver::PSDSaver(KisDocument *doc)
    : m_image(doc->savingImage())
    , m_doc(doc)
    , m_stop(false)
{
}

PSDSaver::~PSDSaver()
{
}

KisImageSP PSDSaver::image()
{
    return m_image;
}

KisImportExportErrorCode PSDSaver::buildFile(QIODevice &io)
{
    KIS_ASSERT_RECOVER_RETURN_VALUE(m_image, ImportExportCodes::InternalError);

    if (m_image->width() > MAX_PSD_SIZE || m_image->height() > MAX_PSD_SIZE) {
        return ImportExportCodes::Failure;
    }

    const bool haveLayers = m_image->rootLayer()->childCount() > 1 ||
        KisPainter::checkDeviceHasTransparency(
                m_image->rootLayer()->firstChild()->projection());

    // HEADER
    PSDHeader header;
    header.signature = "8BPS";
    header.version = 1;
    header.nChannels = haveLayers ?
        m_image->colorSpace()->channelCount() :
        m_image->colorSpace()->colorChannelCount();

    header.width = m_image->width();
    header.height = m_image->height();

    QPair<psd_color_mode, quint16> colordef = colormodelid_to_psd_colormode(m_image->colorSpace()->colorModelId().id(),
                                                                          m_image->colorSpace()->colorDepthId().id());

    if (colordef.first == COLORMODE_UNKNOWN || colordef.second == 0 || colordef.second == 32) {
        m_image->convertImageColorSpace(KoColorSpaceRegistry::instance()->rgb16(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
        m_image->waitForDone();
        colordef = colormodelid_to_psd_colormode(m_image->colorSpace()->colorModelId().id(), m_image->colorSpace()->colorDepthId().id());
    }
    header.colormode = colordef.first;
    header.channelDepth = colordef.second;

    dbgFile << "header" << header << io.pos();

    if (!header.write(io)) {
        dbgFile << "Failed to write header. Error:" << header.error << io.pos();
        return ImportExportCodes::ErrorWhileWriting;
    }

    // COLORMODE BlOCK
    PSDColorModeBlock colorModeBlock(header.colormode);
    // XXX: check for annotations that contain the duotone spec

    KisAnnotationSP annotation = m_image->annotation("DuotoneColormodeBlock");
    if (annotation) {
        colorModeBlock.duotoneSpecification = annotation->annotation();
    }

    dbgFile << "colormode block" << io.pos();
    if (!colorModeBlock.write(io)) {
        dbgFile << "Failed to write colormode block. Error:" << colorModeBlock.error << io.pos();
        return ImportExportCodes::ErrorWhileWriting;
    }

    // IMAGE RESOURCES SECTION
    PSDImageResourceSection resourceSection;

    vKisAnnotationSP_it it = m_image->beginAnnotations();
    vKisAnnotationSP_it endIt = m_image->endAnnotations();
    while (it != endIt) {
        KisAnnotationSP annotation = (*it);
        if (!annotation || annotation->type().isEmpty()) {
            dbgFile << "Warning: empty annotation";
            it++;
            continue;
        }

        dbgFile << "Annotation:" << annotation->type() << annotation->description();

        if (annotation->type().startsWith(QString("PSD Resource Block:"))) { //
            PSDResourceBlock *resourceBlock = dynamic_cast<PSDResourceBlock*>(annotation.data());
            if (resourceBlock) {
                dbgFile << "Adding PSD Resource Block" << resourceBlock->identifier;
                resourceSection.resources[(PSDImageResourceSection::PSDResourceID)resourceBlock->identifier] = resourceBlock;
            }
        }

        it++;
    }

    // Add resolution block
    {
        RESN_INFO_1005 *resInfo = new RESN_INFO_1005;
        resInfo->hRes = INCH_TO_POINT(m_image->xRes());
        resInfo->vRes = INCH_TO_POINT(m_image->yRes());
        PSDResourceBlock *block = new PSDResourceBlock;
        block->identifier = PSDImageResourceSection::RESN_INFO;
        block->resource = resInfo;
        resourceSection.resources[PSDImageResourceSection::RESN_INFO] = block;
    }

    // Add icc block
    {
        ICC_PROFILE_1039 *profileInfo = new ICC_PROFILE_1039;
        profileInfo->icc = m_image->profile()->rawData();
        PSDResourceBlock *block = new PSDResourceBlock;
        block->identifier = PSDImageResourceSection::ICC_PROFILE;
        block->resource = profileInfo;
        resourceSection.resources[PSDImageResourceSection::ICC_PROFILE] = block;

    }

    dbgFile << "resource section" << io.pos();
    if (!resourceSection.write(io)) {
        dbgFile << "Failed to write resource section. Error:" << resourceSection.error << io.pos();
        return ImportExportCodes::ErrorWhileWriting;
    }

    // LAYER AND MASK DATA
    // Only save layers and masks if there is more than one layer
    dbgFile << "m_image->rootLayer->childCount" << m_image->rootLayer()->childCount() << io.pos();

    if (haveLayers) {

        PSDLayerMaskSection layerSection(header);
        layerSection.hasTransparency = true;

        if (!layerSection.write(io, m_image->rootLayer(), psd_compression_type::RLE)) {
            dbgFile << "failed to write layer section. Error:" << layerSection.error << io.pos();
            return ImportExportCodes::ErrorWhileWriting;
        }
    }
    else {
        // else write a zero length block
        dbgFile << "No layers, saving empty layers/mask block" << io.pos();
        psdwrite(io, (quint32)0);
    }

    // IMAGE DATA
    dbgFile << "Saving composited image" << io.pos();
    PSDImageData imagedata(&header);
    // Photoshop compresses layer data by default with RLE.
    if (!imagedata.write(io, m_image->projection(), haveLayers, psd_compression_type::RLE)) {
        dbgFile << "Failed to write image data. Error:"  << imagedata.error;
        return ImportExportCodes::ErrorWhileWriting;
    }

    return ImportExportCodes::OK;
}


void PSDSaver::cancel()
{
    m_stop = true;
}


