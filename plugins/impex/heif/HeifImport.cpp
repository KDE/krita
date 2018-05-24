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

#include "kis_iterator_ng.h"


#include "libheif/heif-cxx.h"


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


    // Load the file into memory and decode from there
    // TODO: There will be a loader-API in libheif that we should use to connect to QUIDevice

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
      KisImageSP image = new KisImage(document->createUndoStore(), width,height, colorSpace,
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
