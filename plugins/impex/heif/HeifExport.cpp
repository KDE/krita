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

#include "HeifExport.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceConstants.h>
#include <KisImportExportManager.h>
#include <KisExportCheckRegistry.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "kis_iterator_ng.h"

#include "HeifConverter.h"

#include "libheif/heif-cxx.h"


class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_heif_export.json", registerPlugin<HeifExport>();)

HeifExport::HeifExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

HeifExport::~HeifExport()
{
}

KisPropertiesConfigurationSP HeifExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("quality", 50);
    cfg->setProperty("lossless", true);
    return cfg;
}

KisConfigWidget *HeifExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisWdgOptionsHeif(parent);
}



class Writer_QIODevice : public heif::Context::Writer
{
public:
  Writer_QIODevice(QIODevice* io)
    : m_io(io)
  {
  }

  heif_error write(heif::Context&, const void* data, size_t size) override {
    qint64 n = m_io->write((const char*)data,size);
    if (n != (qint64)size) {
      QString error = m_io->errorString();

      heif_error err = {
        heif_error_Encoding_error,
        heif_suberror_Cannot_write_output_data,
        "Could not write output data" };

      return err;
    }

    struct heif_error heif_error_ok = { heif_error_Ok, heif_suberror_Unspecified, "Success" };
    return heif_error_ok;
  }

private:
  QIODevice* m_io;
};


KisImportExportFilter::ConversionStatus HeifExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    KisImageSP image = document->savingImage();

    int quality = configuration->getInt("quality", 50);
    bool lossless = configuration->getBool("lossless", false);

    bool has_alpha = false; // TODO

    try {
      // --- use standard HEVC encoder

      heif::Encoder encoder(heif_compression_HEVC);

      encoder.set_lossy_quality(quality);
      encoder.set_lossless(lossless);


    /*
    HeifConverter heifConverter(document, !batchMode());

    KisImageBuilder_Result res;

    res = heifConverter.buildFile(filename(), image);

    dbgFile  << " Result =" << res;
    switch (res) {
    case KisImageBuilder_RESULT_INVALID_ARG:
        document->setErrorMessage(i18n("This layer cannot be saved to HEIF."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_EMPTY:
        document->setErrorMessage(i18n("The layer does not have an image associated with it."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_NO_URI:
        document->setErrorMessage(i18n("The filename is empty."));
        return KisImportExportFilter::CreationError;

    case KisImageBuilder_RESULT_NOT_LOCAL:
        document->setErrorMessage(i18n("HEIF images cannot be saved remotely."));
        return KisImportExportFilter::InternalError;

    case KisImageBuilder_RESULT_OK:
        if (!heifConverter.errorMessage().isNull()) {
            document->setErrorMessage(heifConverter.errorMessage());
        }
        return KisImportExportFilter::OK;
    default:
        break;
    }

    document->setErrorMessage(i18n("Internal Error"));
    return KisImportExportFilter::InternalError;
    */


      // --- convert KisImage to HEIF image ---

      int width = image->width();
      int height = image->height();

      heif::Context ctx;

      heif::Image img;
      img.create(width,height, heif_colorspace_RGB, heif_chroma_444);
      img.add_plane(heif_channel_R, width,height, 8);
      img.add_plane(heif_channel_G, width,height, 8);
      img.add_plane(heif_channel_B, width,height, 8);


      uint8_t* ptrR;
      uint8_t* ptrG;
      uint8_t* ptrB;
      uint8_t* ptrA;
      int strideR,strideG,strideB,strideA;

      ptrR = img.get_plane(heif_channel_R, &strideR);
      ptrG = img.get_plane(heif_channel_G, &strideG);
      ptrB = img.get_plane(heif_channel_B, &strideB);

      if (has_alpha) {
        img.add_plane(heif_channel_Alpha, width,height, 8);
        ptrA = img.get_plane(heif_channel_Alpha, &strideA);
      }


      KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());

      for (int y=0; y<height; y++) {
        KisHLineIteratorSP it = pd->createHLineIteratorNG(0, y, width);

        for (int x=0; x<width; x++) {
          ptrR[y*strideR+x] = KoBgrTraits<quint8>::red(it->rawData());
          ptrG[y*strideG+x] = KoBgrTraits<quint8>::green(it->rawData());
          ptrB[y*strideB+x] = KoBgrTraits<quint8>::blue(it->rawData());

          if (has_alpha) {
            ptrA[y*strideA+x] = 255; // TODO: how do I get alpha channel data
          }

          it->nextPixel();
        }
      }



      // --- encode and write image

      ctx.encode_image(img, encoder);

      Writer_QIODevice writer(io);

      ctx.write(writer);
    }
    catch (heif::Error err) {
    }

    return KisImportExportFilter::OK;
}

void HeifExport::initializeCapabilities()
{
    // This checks before saving for what the file format supports: anything that is supported needs to be mentioned here

  /*
    addCapability(KisExportCheckRegistry::instance()->get("NodeTypeCheck/KisGroupLayer")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
  */

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
          /*<< QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID)*/
            ;
    addSupportedColorModels(supportedColorModels, "HEIF");
}


void KisWdgOptionsHeif::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    chkLossless->setChecked(cfg->getBool("lossless", true));
    sliderQuality->setValue(cfg->getInt("quality", 50));
}

KisPropertiesConfigurationSP KisWdgOptionsHeif::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("lossless", chkLossless->isChecked());
    cfg->setProperty("quality", sliderQuality->value());
    return cfg;
}

#include <HeifExport.moc>
