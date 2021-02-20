/*
 *  SPDX-FileCopyrightText: 2018 Dirk Farin <farin@struktur.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "HeifExport.h"
#include "HeifError.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>
#include <QScopedPointer>
#include <QBuffer>

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceConstants.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>

#include <KisImportExportManager.h>
#include <KisExportCheckRegistry.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include <kis_meta_data_store.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_value.h>
#include <kis_meta_data_schema.h>
#include <kis_meta_data_schema_registry.h>
#include <kis_meta_data_filter_registry_model.h>
#include <kis_exif_info_visitor.h>
#include <kis_meta_data_io_backend.h>

#include "kis_iterator_ng.h"

#include "libheif/heif_cxx.h"


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
    cfg->setProperty("quality", 100);
    cfg->setProperty("lossless", true);
    cfg->setProperty("chroma", "444");
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

    heif_error write(const void* data, size_t size) override {
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


KisImportExportErrorCode HeifExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    KisImageSP image = document->savingImage();
    const KoColorSpace *cs = image->colorSpace();


    // Convert to 8 bits rgba on saving if not rgba+8bit, rgba+16bit or graya+8bit.

    if ( (cs->colorModelId() != RGBAColorModelID || cs->colorModelId() != GrayAColorModelID)
         || (cs->colorDepthId() != Integer8BitsColorDepthID || cs->colorDepthId() != Integer16BitsColorDepthID)) {
        const KoColorSpace *sRgb = KoColorSpaceRegistry::instance()->rgb8();
        image->convertImageColorSpace(sRgb, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    }


    if (cs->colorModelId() == GrayAColorModelID && cs->colorDepthId() != Integer8BitsColorDepthID) {
        const KoColorSpace *gray = KoColorSpaceRegistry::instance()->graya8(cs->profile()->name());
        image->convertImageColorSpace(gray, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    }

    image->waitForDone();
    cs = image->colorSpace();


    int quality = configuration->getInt("quality", 50);
    bool lossless = configuration->getBool("lossless", false);
    bool hasAlpha = configuration->getBool(KisImportExportFilter::ImageContainsTransparencyTag, false);


    // If we want to add information from the document to the metadata,
    // we should do that here.

    try {
        // --- use standard HEVC encoder


        heif::Encoder encoder(heif_compression_HEVC);


        if (mimeType() == "image/avif") {
            encoder = heif::Encoder(heif_compression_AV1);
        }

        encoder.set_lossy_quality(quality);
        encoder.set_lossless(lossless);
        encoder.set_parameter("chroma", configuration->getString("chroma", "444").toStdString());


        // --- convert KisImage to HEIF image ---
        int width = image->width();
        int height = image->height();

        heif::Context ctx;

        heif::Image img;

        if (cs->colorModelId() == RGBAColorModelID) {
            if (cs->colorDepthId() == Integer8BitsColorDepthID) {
                qDebug() << "saving as 8bit rgba";
                img.create(width,height, heif_colorspace_RGB, heif_chroma_444);
                img.add_plane(heif_channel_R, width,height, 8);
                img.add_plane(heif_channel_G, width,height, 8);
                img.add_plane(heif_channel_B, width,height, 8);

                uint8_t* ptrR {0};
                uint8_t* ptrG {0};
                uint8_t* ptrB {0};
                uint8_t* ptrA {0};
                int strideR, strideG, strideB, strideA;

                ptrR = img.get_plane(heif_channel_R, &strideR);
                ptrG = img.get_plane(heif_channel_G, &strideG);
                ptrB = img.get_plane(heif_channel_B, &strideB);

                if (hasAlpha) {
                    img.add_plane(heif_channel_Alpha, width,height, 8);
                    ptrA = img.get_plane(heif_channel_Alpha, &strideA);
                }

                KisPaintDeviceSP pd = image->projection();

                for (int y=0; y<height; y++) {
                    KisHLineIteratorSP it = pd->createHLineIteratorNG(0, y, width);

                    for (int x=0; x<width; x++) {
                        ptrR[y*strideR+x] = KoBgrTraits<quint8>::red(it->rawData());
                        ptrG[y*strideG+x] = KoBgrTraits<quint8>::green(it->rawData());
                        ptrB[y*strideB+x] = KoBgrTraits<quint8>::blue(it->rawData());

                        if (hasAlpha) {
                            ptrA[y * strideA + x] = cs->opacityU8(it->rawData());
                        }

                        it->nextPixel();
                    }
                }
            } else {
                qDebug() << "saving as 12bit rgba";
                img.create(width,height, heif_colorspace_RGB,
                           hasAlpha? heif_chroma_interleaved_RRGGBBAA_BE: heif_chroma_interleaved_RRGGBB_BE);
                img.add_plane(heif_channel_interleaved, width, height, 12);

                uint8_t* ptr {0};
                int stride;

                ptr = img.get_plane(heif_channel_interleaved, &stride);


                KisPaintDeviceSP pd = image->projection();

                for (int y=0; y < height; y++) {
                    KisHLineIteratorSP it = pd->createHLineIteratorNG(0, y, width);

                    for (int x=0; x < width; x++) {

                        QVector<quint16> pixelValues(4);
                        pixelValues[0] = KoBgrTraits<quint16>::red(it->rawData());
                        pixelValues[1] = KoBgrTraits<quint16>::green(it->rawData());
                        pixelValues[2] = KoBgrTraits<quint16>::blue(it->rawData());
                        pixelValues[3] = quint16(KoBgrTraits<quint16>::opacityF(it->rawData()) * 65535);

                        int channels = hasAlpha? 4: 3;
                        for (int ch = 0; ch < channels; ch++) {
                            uint16_t v = qBound(0, int((float(pixelValues[ch]) / 65535) * 4095), 4095);
                            ptr[2 * (x * channels) + y * stride + 0 + (ch*2)] = (uint8_t) (v >> 8);
                            ptr[2 * (x * channels) + y * stride + 1 + (ch*2)] = (uint8_t) (v & 0xFF);
                        }

                        it->nextPixel();
                    }
                }
            }

        } else {
            qDebug() << "saving as 8bit grayscale";
            img.create(width, height, heif_colorspace_monochrome, heif_chroma_monochrome);

            img.add_plane(heif_channel_Y, width, height, 8);

            uint8_t* ptrG {0};
            uint8_t* ptrA {0};
            int strideG, strideA;

            ptrG = img.get_plane(heif_channel_Y, &strideG);

            if (hasAlpha) {
                img.add_plane(heif_channel_Alpha, width, height, 8);
                ptrA = img.get_plane(heif_channel_Alpha, &strideA);
            }

            KisPaintDeviceSP pd = image->projection();

            for (int y = 0; y < height; y++) {
                KisHLineIteratorSP it = pd->createHLineIteratorNG(0, y, width);

                for (int x = 0; x < width; x++) {
                    ptrG[y * strideG + x] = KoGrayTraits<quint8>::gray(it->rawData());

                    if (hasAlpha) {
                        ptrA[y * strideA + x] = cs->opacityU8(it->rawData());
                    }

                    it->nextPixel();
                }
            }
        }



        // --- save the color profile.
        QByteArray rawProfileBA = image->colorSpace()->profile()->rawData();
        std::vector<uint8_t> rawProfile(rawProfileBA.begin(), rawProfileBA.end());
        img.set_raw_color_profile(heif_color_profile_type_prof, rawProfile);


        // --- encode and write image

        heif::ImageHandle handle = ctx.encode_image(img, encoder);


        // --- add Exif / XMP metadata

        KisExifInfoVisitor exivInfoVisitor;
        exivInfoVisitor.visit(image->rootLayer().data());

        QScopedPointer<KisMetaData::Store> metaDataStore;
        if (exivInfoVisitor.metaDataCount() == 1) {
            metaDataStore.reset(new KisMetaData::Store(*exivInfoVisitor.exifInfo()));
        }
        else {
            metaDataStore.reset(new KisMetaData::Store());
        }

        if (!metaDataStore->empty()) {
            {
                KisMetaData::IOBackend* exifIO = KisMetaData::IOBackendRegistry::instance()->value("exif");
                QBuffer buffer;
                exifIO->saveTo(metaDataStore.data(), &buffer, KisMetaData::IOBackend::NoHeader); // Or JpegHeader? Or something else?
                QByteArray data = buffer.data();

                // Write the data to the file
                if (data.size() > 4) {
                    ctx.add_exif_metadata(handle, data.constData(), data.size());
                }
            }
            {
                KisMetaData::IOBackend* xmpIO = KisMetaData::IOBackendRegistry::instance()->value("xmp");
                QBuffer buffer;
                xmpIO->saveTo(metaDataStore.data(), &buffer, KisMetaData::IOBackend::NoHeader); // Or JpegHeader? Or something else?
                QByteArray data = buffer.data();

                // Write the data to the file
                if (data.size() > 0) {
                    ctx.add_XMP_metadata(handle, data.constData(), data.size());
                }
            }
        }


        // --- write HEIF file

        Writer_QIODevice writer(io);

        ctx.write(writer);
    }
    catch (heif::Error err) {
        return setHeifError(document, err);
    }

    return ImportExportCodes::OK;
}

void HeifExport::initializeCapabilities()
{
    // This checks before saving for what the file format supports: anything that is supported needs to be mentioned here

    QList<QPair<KoID, KoID> > supportedColorModels;
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
            << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)/*
                                    << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID)*/
            ;
    addSupportedColorModels(supportedColorModels, "HEIF");
}


void KisWdgOptionsHeif::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    QStringList chromaOptions;
    chromaOptions << "420" << "422" << "444";
    cmbChroma->addItems(chromaOptions);
    cmbChroma->setItemData(0, i18nc("@tooltip", "The brightness of the image will be at full resolution, while the colorfulness will be halved in both dimensions."), Qt::ToolTipRole);
    cmbChroma->setItemData(1, i18nc("@tooltip", "The brightness of the image will be at full resolution, while the colorfulness will be halved horizontally."), Qt::ToolTipRole);
    cmbChroma->setItemData(2, i18nc("@tooltip", "Both brightness and colorfulness of the image will be at full resolution."), Qt::ToolTipRole);
    chkLossless->setChecked(cfg->getBool("lossless", true));
    sliderQuality->setValue(qreal(cfg->getInt("quality", 50)));
    cmbChroma->setCurrentIndex(chromaOptions.indexOf(cfg->getString("chroma", "444")));
    m_hasAlpha = cfg->getBool(KisImportExportFilter::ImageContainsTransparencyTag, false);
}

KisPropertiesConfigurationSP KisWdgOptionsHeif::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("lossless", chkLossless->isChecked());
    cfg->setProperty("quality", int(sliderQuality->value()));
    cfg->setProperty("chroma", cmbChroma->currentText());
    cfg->setProperty(KisImportExportFilter::ImageContainsTransparencyTag, m_hasAlpha);
    return cfg;
}

void KisWdgOptionsHeif::toggleQualitySlider(bool toggle)
{
    // Disable the quality slider if lossless is true
    lossySettings->setEnabled(!toggle);
}

#include <HeifExport.moc>
