/*
 *  SPDX-FileCopyrightText: 2018 Dirk Farin <farin@struktur.de>
 *  SPDX-FileCopyrightText: 2020-2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Daniel Novomesky <dnovomesky@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
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

#include <algorithm>
#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceConstants.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorTransferFunctions.h>

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

using heif::Error;

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
    cfg->setProperty("floatingPointConversionOption", "KeepSame");
    cfg->setProperty("HLGnominalPeak", 1000.0);
    cfg->setProperty("HLGgamma", 1.2);
    cfg->setProperty("removeHGLOOTF", true);
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



    dbgFile << "Starting" << mimeType() << "encoding.";

    // Convert to 8 bits rgba on saving if not rgba or graya.
    if ( cs->colorModelId() != RGBAColorModelID && cs->colorModelId() != GrayAColorModelID ) {
        const KoColorSpace *sRgb = KoColorSpaceRegistry::instance()->rgb8();
        image->convertImageColorSpace(sRgb,
                                      KoColorConversionTransformation::internalRenderingIntent(),
                                      KoColorConversionTransformation::internalConversionFlags());
    }

    if (cs->colorModelId() == GrayAColorModelID && cs->hasHighDynamicRange()) {
        const KoColorSpace *gray = KoColorSpaceRegistry::instance()->graya16(cs->profile()->name());
        image->convertImageColorSpace(gray,
                                      KoColorConversionTransformation::internalRenderingIntent(),
                                      KoColorConversionTransformation::internalConversionFlags());
    }

    conversionPolicy conversionPolicy = keepTheSame;
    bool convertToRec2020 = false;
    
    if (cs->hasHighDynamicRange() && cs->colorModelId() != GrayAColorModelID) {
        QString conversionOption = (configuration->getString("floatingPointConversionOption", "Rec2100PQ"));
        if (conversionOption == "Rec2100PQ") {
            convertToRec2020 = true;
            conversionPolicy = applyPQ;
        } else if (conversionOption == "Rec2100HLG") {
            convertToRec2020 = true;
            conversionPolicy = applyHLG;
        } else if (conversionOption == "ApplyPQ") {
            conversionPolicy = applyPQ;
        } else if (conversionOption == "ApplyHLG") {
            conversionPolicy = applyHLG;
        }  else if (conversionOption == "ApplySMPTE428") {
            conversionPolicy = applySMPTE428;
        }
    }

    if (cs->hasHighDynamicRange() && convertToRec2020) {
        const KoColorProfile *linear = KoColorSpaceRegistry::instance()->profileFor(QVector<double>(),
                                                                                   PRIMARIES_ITU_R_BT_2020_2_AND_2100_0,
                                                                                   TRC_LINEAR);
        const KoColorSpace *linearRec2020 = KoColorSpaceRegistry::instance()->colorSpace("RGBA", "F32", linear);
        image->convertImageColorSpace(linearRec2020,
                                      KoColorConversionTransformation::internalRenderingIntent(),
                                      KoColorConversionTransformation::internalConversionFlags());
    }

    image->waitForDone();
    cs = image->colorSpace();

    int quality = configuration->getInt("quality", 50);
    bool lossless = configuration->getBool("lossless", false);
    bool hasAlpha = configuration->getBool(KisImportExportFilter::ImageContainsTransparencyTag, false);
    float hlgGamma = configuration->getFloat("HLGgamma", 1.2f);
    float hlgNominalPeak = configuration->getFloat("HLGnominalPeak", 1000.0f);
    bool removeHGLOOTF = configuration->getBool("removeHGLOOTF", true);

    // If we want to add information from the document to the metadata,
    // we should do that here.

    try {
        // --- use standard HEVC encoder


        heif::Encoder encoder(heif_compression_HEVC);


        if (mimeType() == "image/avif") {
            encoder = heif::Encoder(heif_compression_AV1);
        }


        encoder.set_lossy_quality(quality);
        if (lossless) {
            //https://invent.kde.org/graphics/krita/-/merge_requests/530#note_169521
            encoder.set_lossy_quality(100);
        }
        encoder.set_lossless(lossless);
        if (cs->colorModelId() != GrayAColorModelID) {
        encoder.set_parameter("chroma", configuration->getString("chroma", "444").toStdString());
        }


        // --- convert KisImage to HEIF image ---
        int width = image->width();
        int height = image->height();

        heif::Context ctx;

        heif_chroma chroma = hasAlpha? heif_chroma_interleaved_RRGGBBAA_LE: heif_chroma_interleaved_RRGGBB_LE;
        int endValue0 = 1;
        int endValue1 = 0;
        if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            endValue0 = 0;
            endValue1 = 1;
            chroma = hasAlpha? heif_chroma_interleaved_RRGGBBAA_BE: heif_chroma_interleaved_RRGGBB_BE;
        }

        heif::Image img;

        const int max12bit = 4095;
        const int max16bit = 65535;
        const double multiplier16bit = (1.0/double(max16bit));

        if (cs->colorModelId() == RGBAColorModelID) {
            if (cs->colorDepthId() == Integer8BitsColorDepthID) {
                dbgFile << "saving as 8bit rgba";
                img.create(width,height, heif_colorspace_RGB, heif_chroma_444);
                img.add_plane(heif_channel_R, width,height, 8);
                img.add_plane(heif_channel_G, width,height, 8);
                img.add_plane(heif_channel_B, width,height, 8);

                uint8_t* ptrR {nullptr};
                uint8_t *ptrG {nullptr};
                uint8_t *ptrB {nullptr};
                uint8_t *ptrA {nullptr};
                int strideR, strideG, strideB, strideA;

                ptrR = img.get_plane(heif_channel_R, &strideR);
                ptrG = img.get_plane(heif_channel_G, &strideG);
                ptrB = img.get_plane(heif_channel_B, &strideB);

                if (hasAlpha) {
                    img.add_plane(heif_channel_Alpha, width,height, 8);
                    ptrA = img.get_plane(heif_channel_Alpha, &strideA);
                }

                KisPaintDeviceSP pd = image->projection();
                KisHLineIteratorSP it = pd->createHLineIteratorNG(0, 0, width);

                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        ptrR[y*strideR+x] = KoBgrU8Traits::red(it->rawData());
                        ptrG[y*strideG+x] = KoBgrU8Traits::green(it->rawData());
                        ptrB[y*strideB+x] = KoBgrU8Traits::blue(it->rawData());

                        if (hasAlpha) {
                            ptrA[y * strideA + x] = cs->opacityU8(it->rawData());
                        }

                        it->nextPixel();
                    }

                    it->nextRow();
                }
            } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
                dbgFile << "Saving as 12bit rgba";
                img.create(width,height, heif_colorspace_RGB, chroma);
                img.add_plane(heif_channel_interleaved, width, height, 12);

                uint8_t *ptr {nullptr};
                int stride;

                ptr = img.get_plane(heif_channel_interleaved, &stride);

                KisPaintDeviceSP pd = image->projection();
                QVector<quint16> pixelValues(4);
                KisHLineIteratorSP it = pd->createHLineIteratorNG(0, 0, width);

                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        pixelValues[0] = KoBgrU16Traits::red(it->rawData());
                        pixelValues[1] = KoBgrU16Traits::green(it->rawData());
                        pixelValues[2] = KoBgrU16Traits::blue(it->rawData());
                        pixelValues[3] = quint16(KoBgrU16Traits::opacityF(it->rawData()) * max16bit);

                        int channels = hasAlpha? 4: 3;
                        for (int ch = 0; ch < channels; ch++) {
                            uint16_t v = qBound(0, int(float(pixelValues[ch]) * multiplier16bit * max12bit), max12bit);
                            ptr[2 * (x * channels) + y * stride + endValue0 + (ch*2)] = (uint8_t) (v >> 8);
                            ptr[2 * (x * channels) + y * stride + endValue1 + (ch*2)] = (uint8_t) (v & 0xFF);
                        }

                        it->nextPixel();
                    }

                    it->nextRow();
                }
            } else {
                dbgFile << "Saving floating point as 12bit rgba";
                img.create(width,height, heif_colorspace_RGB, chroma);
                img.add_plane(heif_channel_interleaved, width, height, 12);

                int stride;

                uint8_t *ptr = img.get_plane(heif_channel_interleaved, &stride);

                KisPaintDeviceSP pd = image->projection();
                QVector<float> pixelValues(4);
                QVector<qreal> pixelValuesLinear(4);
                QVector<qreal> lCoef {cs->lumaCoefficients()};
                KisHLineIteratorSP it = pd->createHLineIteratorNG(0, 0, width);
                const KoColorProfile *profile = cs->profile();
                bool isLinear = profile->isLinear();

                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        cs->normalisedChannelsValue(it->rawData(), pixelValues);
                        if (!convertToRec2020 && !isLinear) {
                            std::copy(pixelValues.begin(), pixelValues.end(), pixelValuesLinear.begin());
                            profile->linearizeFloatValue(pixelValuesLinear);
                            std::copy(pixelValuesLinear.begin(), pixelValuesLinear.end(), pixelValues.begin());
                        }

                        if (conversionPolicy == applyHLG && removeHGLOOTF) {
                            removeHLGOOTF(pixelValues, lCoef, hlgGamma, hlgNominalPeak);
                        }

                        int channels = hasAlpha? 4: 3;
                        for (int ch = 0; ch < channels; ch++) {
                            uint16_t v = qBound(0, int(applyCurveAsNeeded(pixelValues[ch], conversionPolicy) * max12bit), max12bit);
                            ptr[2 * (x * channels) + y * stride + endValue0 + (ch*2)] = (uint8_t) (v >> 8);
                            ptr[2 * (x * channels) + y * stride + endValue1 + (ch*2)] = (uint8_t) (v & 0xFF);
                        }

                        it->nextPixel();
                    }

                    it->nextRow();
                }
            }

        } else {
            if (cs->colorDepthId() == Integer8BitsColorDepthID) {
                dbgFile << "Saving as 8 bit monochrome.";
                img.create(width, height, heif_colorspace_monochrome, heif_chroma_monochrome);

                img.add_plane(heif_channel_Y, width, height, 8);

                uint8_t *ptrG {nullptr};
                uint8_t *ptrA {nullptr};
                int strideG, strideA;

                ptrG = img.get_plane(heif_channel_Y, &strideG);

                if (hasAlpha) {
                    img.add_plane(heif_channel_Alpha, width, height, 8);
                    ptrA = img.get_plane(heif_channel_Alpha, &strideA);
                }

                KisPaintDeviceSP pd = image->projection();
                KisHLineIteratorSP it = pd->createHLineIteratorNG(0, 0, width);

                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        ptrG[y * strideG + x] = KoGrayU8Traits::gray(it->rawData());

                        if (hasAlpha) {
                            ptrA[y * strideA + x] = cs->opacityU8(it->rawData());
                        }

                        it->nextPixel();
                    }

                    it->nextRow();
                }
            } else {
                dbgFile << "Saving as 12 bit monochrome";
                img.create(width, height, heif_colorspace_monochrome, heif_chroma_monochrome);

                img.add_plane(heif_channel_Y, width, height, 12);

                uint8_t *ptrG {nullptr};
                uint8_t *ptrA {nullptr};
                int strideG, strideA;

                ptrG = img.get_plane(heif_channel_Y, &strideG);

                if (hasAlpha) {
                    img.add_plane(heif_channel_Alpha, width, height, 12);
                    ptrA = img.get_plane(heif_channel_Alpha, &strideA);
                }

                KisPaintDeviceSP pd = image->projection();
                KisHLineIteratorSP it = pd->createHLineIteratorNG(0, 0, width);

                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        uint16_t v = qBound(0, int(float( KoGrayU16Traits::gray(it->rawData()) ) * multiplier16bit * max12bit), max12bit);
                        ptrG[(x*2) + y * strideG + endValue0] = (uint8_t) (v >> 8);
                        ptrG[(x*2) + y * strideG + endValue1] = (uint8_t) (v & 0xFF);

                        if (hasAlpha) {
                            uint16_t vA = qBound(0, int( cs->opacityF(it->rawData()) * max12bit), max12bit);
                            ptrA[(x*2) + y * strideA + endValue0] = (uint8_t) (vA >> 8);
                            ptrA[(x*2) + y * strideA + endValue1] = (uint8_t) (vA & 0xFF);
                        }

                        it->nextPixel();
                    }

                    it->nextRow();
                }
            }
        }

        // --- save the color profile.
        if (conversionPolicy == keepTheSame) {
            QByteArray rawProfileBA = image->colorSpace()->profile()->rawData();
            std::vector<uint8_t> rawProfile(rawProfileBA.begin(), rawProfileBA.end());
            img.set_raw_color_profile(heif_color_profile_type_prof, rawProfile);
        } else {
           heif::ColorProfile_nclx nclxDescription;
           nclxDescription.set_full_range_flag(true);
           nclxDescription.set_matrix_coefficients(heif_matrix_coefficients_RGB_GBR);
           if (convertToRec2020) {
               nclxDescription.set_color_primaties(heif_color_primaries_ITU_R_BT_2020_2_and_2100_0);
           } else {
               ColorPrimaries primaries = image->colorSpace()->profile()->getColorPrimaries();
               if (primaries >= 256) {
                   primaries = PRIMARIES_UNSPECIFIED;
               }
               nclxDescription.set_color_primaties(heif_color_primaries(primaries));
           }

           if (conversionPolicy == applyPQ) {
               nclxDescription.set_transfer_characteristics(heif_transfer_characteristic_ITU_R_BT_2100_0_PQ);
           } else if (conversionPolicy == applyHLG) {
               nclxDescription.set_transfer_characteristics(heif_transfer_characteristic_ITU_R_BT_2100_0_HLG);
           } else if (conversionPolicy == applySMPTE428) {
               nclxDescription.set_transfer_characteristics(heif_transfer_characteristic_SMPTE_ST_428_1);
           }

           img.set_nclx_color_profile(nclxDescription);
        }


        // --- encode and write image

        heif::Context::EncodingOptions options;

        // iOS gets confused when a heif file contains an nclx.
        // but we absolutely need it for hdr.
        if (conversionPolicy != keepTheSame && cs->hasHighDynamicRange()) {
            options.macOS_compatibility_workaround_no_nclx_profile = false;
        }

        heif::ImageHandle handle = ctx.encode_image(img, encoder, options);


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
    } catch (Error &err) {
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
            << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)
            << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID)
            ;
    addSupportedColorModels(supportedColorModels, "HEIF");
}

float HeifExport::applyCurveAsNeeded(float value, HeifExport::conversionPolicy policy)
{
    if ( policy == applyPQ) {
        return applySmpte2048Curve(value);
    } else if ( policy == applyHLG) {
        return applyHLGCurve(value);
    } else if ( policy == applySMPTE428) {
        return applySMPTE_ST_428Curve(value);
    }
    return value;
}


void KisWdgOptionsHeif::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    // the export manager should have prepared some info for us!
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::ImageContainsTransparencyTag));
    KIS_SAFE_ASSERT_RECOVER_NOOP(cfg->hasProperty(KisImportExportFilter::ColorModelIDTag));

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
    
    int CicpPrimaries = cfg->getInt(KisImportExportFilter::CICPPrimariesTag, 2);
    
    conversionSettings->setVisible(cfg->getBool(KisImportExportFilter::HDRTag, false));

    QStringList conversionOptionsList = { i18nc("Colorspace name", "Rec 2100 PQ"), i18nc("Colorspace name", "Rec 2100 HLG")};
    QStringList toolTipList = {i18nc("@tooltip", "The image will be converted to Rec 2020 linear first, and then encoded with a perceptual quantizer curve"
                               " (also known as SMPTE 2048 curve). Recommended for HDR images where the absolute brightness is important."),
                              i18nc("@tooltip", "The image will be converted to Rec 2020 linear first, and then encoded with a Hybrid Log Gamma curve."
                               " Recommended for HDR images where the display may not understand HDR.")};
    QStringList conversionOptionName = {"Rec2100PQ", "Rec2100HLG"};
    
    if (cfg->getString(KisImportExportFilter::ColorModelIDTag) == "RGBA") {
        if (CicpPrimaries != PRIMARIES_UNSPECIFIED) {
            conversionOptionsList << i18nc("Colorspace option plus transfer function name", "Keep colorants, encode PQ");
            toolTipList << i18nc("@tooltip", "The image will be linearized first, and then encoded with a perceptual quantizer curve"
                                            " (also known as the SMPTE 2048 curve). Recommended for images where the absolute brightness is important.");
            conversionOptionName << "ApplyPQ";
            
            conversionOptionsList << i18nc("Colorspace option plus transfer function name", "Keep colorants, encode HLG");
            toolTipList << i18nc("@tooltip", "The image will be linearized first, and then encoded with a Hybrid Log Gamma curve."
                                            " Recommended for images intended for screens which cannot understand PQ");
            conversionOptionName << "ApplyHLG";
            
            conversionOptionsList << i18nc("Colorspace option plus transfer function name", "Keep colorants, encode SMPTE ST 428");
            toolTipList << i18nc("@tooltip", "The image will be linearized first, and then encoded with SMPTE ST 428"
                                            " Krita always opens images like these as linear floating point, this option is there to reverse that");
            conversionOptionName << "ApplySMPTE428";
        }
        
        conversionOptionsList << i18nc("Colorspace option", "No changes, clip");
        toolTipList << i18nc("@tooltip", "The image will be converted plainly to 12bit integer, and values that are out of bounds are clipped, the icc profile will be embedded.");
        conversionOptionName << "KeepSame";
    }
    cmbConversionPolicy->addItems(conversionOptionsList);
    for (int i=0; i< toolTipList.size(); i++) {
        cmbConversionPolicy->setItemData(i, toolTipList.at(i), Qt::ToolTipRole);
        cmbConversionPolicy->setItemData(i, conversionOptionName.at(i), Qt::UserRole+1);
    }
    QString optionName = cfg->getString("floatingPointConversionOption", "Rec2100PQ");
    cmbConversionPolicy->setCurrentIndex(conversionOptionName.indexOf(optionName));
    chkHLGOOTF->setChecked(cfg->getBool("removeHGLOOTF", true));
    spnNits->setValue(cfg->getDouble("HLGnominalPeak", 1000.0));
    spnGamma->setValue(cfg->getDouble("HLGgamma", 1.2));
}

KisPropertiesConfigurationSP KisWdgOptionsHeif::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("lossless", chkLossless->isChecked());
    cfg->setProperty("quality", int(sliderQuality->value()));
    cfg->setProperty("chroma", cmbChroma->currentText());
    cfg->setProperty("floatingPointConversionOption", cmbConversionPolicy->currentData(Qt::UserRole+1).toString());
    cfg->setProperty("HLGnominalPeak", spnNits->value());
    cfg->setProperty("HLGgamma", spnGamma->value());
    cfg->setProperty("removeHGLOOTF", chkLossless->isChecked());
    cfg->setProperty(KisImportExportFilter::ImageContainsTransparencyTag, m_hasAlpha);
    return cfg;
}

void KisWdgOptionsHeif::toggleQualitySlider(bool toggle)
{
    // Disable the quality slider if lossless is true
    lossySettings->setEnabled(!toggle);
}

void KisWdgOptionsHeif::toggleHLGOptions(bool toggle)
{
    spnNits->setEnabled(toggle);
    spnGamma->setEnabled(toggle);
}

void KisWdgOptionsHeif::toggleExtraHDROptions(int index) {
    Q_UNUSED(index)
    bool toggle = cmbConversionPolicy->currentData(Qt::UserRole+1).toString().contains("HLG");
    chkHLGOOTF->setEnabled(toggle);
    spnNits->setEnabled(toggle);
    spnGamma->setEnabled(toggle);
}
#include <HeifExport.moc>
