/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "HeifImport.h"

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>




#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_group_layer.h>

#include "kis_iterator_ng.h"


#include "HeifConverter.h"

#include "libheif/heif.h"


K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_heif_import.json", registerPlugin<HeifImport>();)

HeifImport::HeifImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

HeifImport::~HeifImport()
{
}

KisImportExportFilter::ConversionStatus HeifImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    //HeifConverter imageBuilder(document, !batchMode());

    /*
    switch (imageBuilder.buildImage(filename())) {
    case KisImageBuilder_RESULT_UNSUPPORTED:
    case KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE:
        document->setErrorMessage(i18n("Krita does support this type of EXR file."));
        return KisImportExportFilter::NotImplemented;

    case KisImageBuilder_RESULT_INVALID_ARG:
        document->setErrorMessage(i18n("This is not a HEIF file."));
        return KisImportExportFilter::BadMimeType;

    case KisImageBuilder_RESULT_NO_URI:
    case KisImageBuilder_RESULT_NOT_LOCAL:
        document->setErrorMessage(i18n("The HEIF file does not exist."));
        return KisImportExportFilter::FileNotFound;

    case KisImageBuilder_RESULT_BAD_FETCH:
    case KisImageBuilder_RESULT_EMPTY:
        document->setErrorMessage(i18n("The HEIF file is corrupted."));
        return KisImportExportFilter::ParsingError;

    case KisImageBuilder_RESULT_FAILURE:
        document->setErrorMessage(i18n("Krita could not create a new image."));
        return KisImportExportFilter::InternalError;

    case KisImageBuilder_RESULT_OK:
        Q_ASSERT(imageBuilder.image());
        document -> setCurrentImage(imageBuilder.image());
        return KisImportExportFilter::OK;

    default:
        break;
    }
    */


    qint64 fileLength = io->bytesAvailable();
    char* mem = new char[fileLength];
    io->read(mem, fileLength);

    struct heif_context* ctx = heif_context_alloc();

    struct heif_error error;
    error = heif_context_read_from_memory(ctx, mem, fileLength, NULL);


    struct heif_image_handle* handle;
    error = heif_context_get_primary_image_handle(ctx, &handle);
    if (error.code) {
    }

    struct heif_image* heifimage;
    error = heif_decode_image(handle, &heifimage,
                              heif_colorspace_RGB, heif_chroma_444,
                              NULL);
    if (error.code) {
    }

    int width =heif_image_handle_get_width(handle);
    int height=heif_image_handle_get_height(handle);
    bool hasAlpha = heif_image_handle_has_alpha_channel(handle);

    int strideR, strideG, strideB, strideA;
    const uint8_t* imgR = heif_image_get_plane_readonly(heifimage, heif_channel_R, &strideR);
    const uint8_t* imgG = heif_image_get_plane_readonly(heifimage, heif_channel_G, &strideG);
    const uint8_t* imgB = heif_image_get_plane_readonly(heifimage, heif_channel_B, &strideB);
    const uint8_t* imgA = heif_image_get_plane_readonly(heifimage, heif_channel_Alpha, &strideA);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(document->createUndoStore(), width,height, colorSpace, "heif image");

    KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), 255);

    for (int y=0;y<height;y++) {
      KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, y, width);

      for (int x=0;x<width;x++) {
        KoBgrTraits<quint8>::setRed(it->rawData(), imgR[y*strideR+x]);
        KoBgrTraits<quint8>::setGreen(it->rawData(), imgG[y*strideG+x]);
        KoBgrTraits<quint8>::setBlue(it->rawData(), imgB[y*strideB+x]);

        if (hasAlpha) {
          colorSpace->setOpacity(it->rawData(), quint8(imgA[y*strideA+x]), 1);
        }
        else {
          colorSpace->setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
        }

        it->nextPixel();
      }
    }

    image->addNode(layer.data(), image->rootLayer().data());

    delete[] mem;

    document->setCurrentImage(image);
    return KisImportExportFilter::OK;

    //return KisImportExportFilter::StorageCreationError;
}

#include <HeifImport.moc>
