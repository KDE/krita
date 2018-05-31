/*
 *  Copyright (c) 2018 Dirk Farin <farin@struktur.de>
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
#include "HeifError.h"

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QBuffer>

#include <KisImportExportManager.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_group_layer.h>

#include <metadata/kis_meta_data_entry.h>
#include <metadata/kis_meta_data_value.h>
#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_io_backend.h>

#include "kis_iterator_ng.h"


#include "libheif/heif_cxx.h"


K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_heif_import.json", registerPlugin<HeifImport>();)

HeifImport::HeifImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

HeifImport::~HeifImport()
{
}


KisImportExportFilter::ConversionStatus HeifImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    // Load the file into memory and decode from there
    // TODO: There will be a loader-API in libheif that we should use to connect to QIODevice

    qint64 fileLength = io->bytesAvailable();
    char* mem = new char[fileLength];
    io->read(mem, fileLength);

    try {
        heif::Context ctx;
        ctx.read_from_memory(mem, fileLength);


        // decode primary image

        heif::ImageHandle handle = ctx.get_primary_image_handle();
        heif::Image heifimage = handle.decode_image(heif_colorspace_RGB, heif_chroma_444);

        int width =handle.get_width();
        int height=handle.get_height();
        bool hasAlpha = handle.has_alpha_channel();


        // convert HEIF image to Krita KisDocument

        int strideR, strideG, strideB, strideA;
        const uint8_t* imgR = heifimage.get_plane(heif_channel_R, &strideR);
        const uint8_t* imgG = heifimage.get_plane(heif_channel_G, &strideG);
        const uint8_t* imgB = heifimage.get_plane(heif_channel_B, &strideB);
        const uint8_t* imgA = heifimage.get_plane(heif_channel_Alpha, &strideA);

        const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        KisImageSP image = new KisImage(document->createUndoStore(), width, height, colorSpace,
                                        "HEIF image");

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



        // --- Iterate through all metadata blocks and extract Exif and XMP metadata ---

        std::vector<heif_item_id> metadata_IDs = handle.get_list_of_metadata_block_IDs();

        for (heif_item_id id : metadata_IDs) {

          if (handle.get_metadata_type(id) == "Exif") {
            // Read exif information

            std::vector<uint8_t> exif_data = handle.get_metadata(id);

            if (exif_data.size()>4) {
              uint32_t skip = ((exif_data[0]<<24) | (exif_data[1]<<16) | (exif_data[2]<<8) | exif_data[3]) + 4;

              if (exif_data.size()>skip) {
                KisMetaData::IOBackend* exifIO = KisMetaData::IOBackendRegistry::instance()->value("exif");

                // Copy the exif data into the byte array
                QByteArray ba;
                ba.append((char*)(exif_data.data()+skip), exif_data.size()-skip);
                QBuffer buf(&ba);
                exifIO->loadFrom(layer->metaData(), &buf);
              }
            }
          }

          if (handle.get_metadata_type(id) == "mime" &&
              handle.get_metadata_content_type(id) == "application/rdf+xml") {
            // Read XMP information

            std::vector<uint8_t> xmp_data = handle.get_metadata(id);

            KisMetaData::IOBackend* xmpIO = KisMetaData::IOBackendRegistry::instance()->value("xmp");

            // Copy the xmp data into the byte array
            QByteArray ba;
            ba.append((char*)(xmp_data.data()), xmp_data.size());

            QBuffer buf(&ba);
            xmpIO->loadFrom(layer->metaData(), &buf);
          }
        }

        delete[] mem;


        document->setCurrentImage(image);
        return KisImportExportFilter::OK;
    }
    catch (heif::Error err) {
        delete[] mem;

        return setHeifError(document, err);
    }
}

#include <HeifImport.moc>
