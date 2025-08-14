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

#include <QApplication>
#include <QBuffer>
#include <QCheckBox>
#include <QFileInfo>
#include <QScopedPointer>
#include <QSlider>

#include <algorithm>
#include <kpluginfactory.h>
#include <libheif/heif_cxx.h>

#include <KisDocument.h>
#include <KisExportCheckRegistry.h>
#include <KisImportExportManager.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpaceConstants.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorTransferFunctions.h>
#include <kis_assert.h>
#include <kis_config.h>
#include <kis_exif_info_visitor.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_iterator_ng.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_filter_registry_model.h>
#include <kis_meta_data_schema.h>
#include <kis_meta_data_schema_registry.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_value.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_properties_configuration.h>

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
    cfg->setProperty("monochromeToSRGB", false);
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
        qint64 n = m_io->write(static_cast<const char *>(data),
                               static_cast<int>(size));
        if (n != static_cast<qint64>(size)) {
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

#if LIBHEIF_HAVE_VERSION(1, 13, 0)
class Q_DECL_HIDDEN HeifLock
{
public:
    HeifLock()
        : p()
    {
        heif_init(&p);
    }

    ~HeifLock()
    {
        heif_deinit();
    }

private:
    heif_init_params p;
};
#endif

KisImportExportErrorCode HeifExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
#if LIBHEIF_HAVE_VERSION(1, 13, 0)
    HeifLock lock;
#endif

#if LIBHEIF_HAVE_VERSION(1, 20, 2)
    using HeifStrideType = size_t;
    auto heifGetPlaneMethod = std::mem_fn(qNonConstOverload<heif_channel, HeifStrideType*>(&heif::Image::get_plane2));
#elif LIBHEIF_HAVE_VERSION(1, 20, 0)
    using HeifStrideType = size_t;
    auto heifGetPlaneMethod = std::mem_fn(qNonConstOverload<heif_channel, HeifStrideType*>(&heif::Image::get_plane));
#else
    using HeifStrideType = int;
    auto heifGetPlaneMethod = std::mem_fn(qNonConstOverload<heif_channel, HeifStrideType*>(&heif::Image::get_plane));
#endif


    KisImageSP image = document->savingImage();
    const KoColorSpace *cs = image->colorSpace();



    dbgFile << "Starting" << mimeType() << "encoding.";

    bool convertToSRGB = (configuration->getBool("monochromeToSRGB") && cs->colorModelId() == GrayAColorModelID);

    // Convert to 8 bits rgba on saving if not rgba or graya.
    if ( (cs->colorModelId() != RGBAColorModelID && cs->colorModelId() != GrayAColorModelID) || convertToSRGB) {
        const KoColorSpace *sRgb = KoColorSpaceRegistry::instance()->rgb8();
        image->convertImageColorSpace(sRgb,
                                      KoColorConversionTransformation::internalRenderingIntent(),
                                      KoColorConversionTransformation::internalConversionFlags());
    }

    if (cs->colorModelId() == GrayAColorModelID && cs->hasHighDynamicRange() && !convertToSRGB) {
        const KoColorSpace *gray = KoColorSpaceRegistry::instance()->graya16(cs->profile()->name());
        image->convertImageColorSpace(gray,
                                      KoColorConversionTransformation::internalRenderingIntent(),
                                      KoColorConversionTransformation::internalConversionFlags());
    }

    ConversionPolicy conversionPolicy = ConversionPolicy::KeepTheSame;
    bool convertToRec2020 = false;
    
    if (cs->hasHighDynamicRange() && cs->colorModelId() != GrayAColorModelID) {
        QString conversionOption =
            (configuration->getString("floatingPointConversionOption",
                                      "KeepSame"));
        if (conversionOption == "Rec2100PQ") {
            convertToRec2020 = true;
            conversionPolicy = ConversionPolicy::ApplyPQ;
        } else if (conversionOption == "Rec2100HLG") {
            convertToRec2020 = true;
            conversionPolicy = ConversionPolicy::ApplyHLG;
        } else if (conversionOption == "ApplyPQ") {
            conversionPolicy = ConversionPolicy::ApplyPQ;
        } else if (conversionOption == "ApplyHLG") {
            conversionPolicy = ConversionPolicy::ApplyHLG;
        }  else if (conversionOption == "ApplySMPTE428") {
            conversionPolicy = ConversionPolicy::ApplySMPTE428;
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
        if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
            chroma = hasAlpha? heif_chroma_interleaved_RRGGBBAA_BE: heif_chroma_interleaved_RRGGBB_BE;
        }

        heif::Image img;

        if (cs->colorModelId() == RGBAColorModelID) {
            if (cs->colorDepthId() == Integer8BitsColorDepthID) {
                dbgFile << "saving as 8bit rgba";
                img.create(width,height, heif_colorspace_RGB, heif_chroma_444);
                img.add_plane(heif_channel_R, width,height, 8);
                img.add_plane(heif_channel_G, width,height, 8);
                img.add_plane(heif_channel_B, width,height, 8);

                HeifStrideType strideR = 0;
                HeifStrideType strideG = 0;
                HeifStrideType strideB = 0;
                HeifStrideType strideA = 0;

                uint8_t *ptrR = heifGetPlaneMethod(img, heif_channel_R, &strideR);
                uint8_t *ptrG = heifGetPlaneMethod(img, heif_channel_G, &strideG);
                uint8_t *ptrB = heifGetPlaneMethod(img, heif_channel_B, &strideB);

                uint8_t *ptrA = [&]() -> uint8_t * {
                    if (hasAlpha) {
                        img.add_plane(heif_channel_Alpha, width, height, 8);
                        return heifGetPlaneMethod(img, heif_channel_Alpha, &strideA);
                    } else {
                        return nullptr;
                    }
                }();

                KisPaintDeviceSP pd = image->projection();
                KisHLineConstIteratorSP it =
                    pd->createHLineConstIteratorNG(0, 0, width);

                Planar::writeLayer(hasAlpha,
                                   width,
                                   height,
                                   ptrR,
                                   strideR,
                                   ptrG,
                                   strideG,
                                   ptrB,
                                   strideB,
                                   ptrA,
                                   strideA,
                                   it);
            } else {
                dbgFile << "Saving as 12bit rgba";
                img.create(width, height, heif_colorspace_RGB, chroma);
                img.add_plane(heif_channel_interleaved, width, height, 12);

                HeifStrideType stride = 0;

                uint8_t *ptr = heifGetPlaneMethod(img, heif_channel_interleaved, &stride);

                KisPaintDeviceSP pd = image->projection();
                KisHLineConstIteratorSP it =
                    pd->createHLineConstIteratorNG(0, 0, width);

                if (cs->colorDepthId() == Integer16BitsColorDepthID) {
                    HDRInt::writeInterleavedLayer(QSysInfo::ByteOrder,
                                                  hasAlpha,
                                                  width,
                                                  height,
                                                  ptr,
                                                  stride,
                                                  it);
                } else {
                    HDRFloat::writeInterleavedLayer(cs->colorDepthId(),
                                                    QSysInfo::ByteOrder,
                                                    hasAlpha,
                                                    convertToRec2020,
                                                    cs->profile()->isLinear(),
                                                    conversionPolicy,
                                                    removeHGLOOTF,
                                                    width,
                                                    height,
                                                    ptr,
                                                    stride,
                                                    it,
                                                    hlgGamma,
                                                    hlgNominalPeak,
                                                    cs);
                }
            }
        } else {
            if (cs->colorDepthId() == Integer8BitsColorDepthID) {
                dbgFile << "Saving as 8 bit monochrome.";
                img.create(width, height, heif_colorspace_monochrome, heif_chroma_monochrome);

                img.add_plane(heif_channel_Y, width, height, 8);

                HeifStrideType strideG = 0;
                HeifStrideType strideA = 0;

                uint8_t *ptrG = heifGetPlaneMethod(img, heif_channel_Y, &strideG);
                uint8_t *ptrA = [&]() -> uint8_t * {
                    if (hasAlpha) {
                        img.add_plane(heif_channel_Alpha, width, height, 8);
                        return heifGetPlaneMethod(img, heif_channel_Alpha, &strideA);
                    } else {
                        return nullptr;
                    }
                }();

                KisPaintDeviceSP pd = image->projection();
                KisHLineConstIteratorSP it =
                    pd->createHLineConstIteratorNG(0, 0, width);

                Gray::writePlanarLayer(QSysInfo::ByteOrder,
                                       8,
                                       hasAlpha,
                                       width,
                                       height,
                                       ptrG,
                                       strideG,
                                       ptrA,
                                       strideA,
                                       it);
            } else {
                dbgFile << "Saving as 12 bit monochrome";
                img.create(width, height, heif_colorspace_monochrome, heif_chroma_monochrome);

                img.add_plane(heif_channel_Y, width, height, 12);

                HeifStrideType strideG = 0;
                HeifStrideType strideA = 0;

                uint8_t *ptrG = heifGetPlaneMethod(img, heif_channel_Y, &strideG);
                uint8_t *ptrA = [&]() -> uint8_t * {
                    if (hasAlpha) {
                        img.add_plane(heif_channel_Alpha, width, height, 12);
                        return heifGetPlaneMethod(img, heif_channel_Alpha, &strideA);
                    } else {
                        return nullptr;
                    }
                }();

                KisPaintDeviceSP pd = image->projection();
                KisHLineConstIteratorSP it =
                    pd->createHLineConstIteratorNG(0, 0, width);

                Gray::writePlanarLayer(QSysInfo::ByteOrder,
                                       12,
                                       hasAlpha,
                                       width,
                                       height,
                                       ptrG,
                                       strideG,
                                       ptrA,
                                       strideA,
                                       it);
            }
        }

        // --- save the color profile.
        if (conversionPolicy == ConversionPolicy::KeepTheSame) {
            QByteArray rawProfileBA = image->colorSpace()->profile()->rawData();
            std::vector<uint8_t> rawProfile(rawProfileBA.begin(), rawProfileBA.end());
            img.set_raw_color_profile(heif_color_profile_type_prof, rawProfile);
        } else {
           heif::ColorProfile_nclx nclxDescription;
           nclxDescription.set_full_range_flag(true);
           nclxDescription.set_matrix_coefficients(heif_matrix_coefficients_RGB_GBR);
           if (convertToRec2020) {
#if LIBHEIF_HAVE_VERSION(1, 14, 1)
               nclxDescription.set_color_primaries(heif_color_primaries_ITU_R_BT_2020_2_and_2100_0);
#else
               nclxDescription.set_color_primaties(heif_color_primaries_ITU_R_BT_2020_2_and_2100_0);
#endif
           } else {
                const ColorPrimaries primaries =
                    image->colorSpace()->profile()->getColorPrimaries();
                // PRIMARIES_ADOBE_RGB_1998 and higher are not valid for CICP.
                // But this should have already been caught by the KeepTheSame
                // clause...
                KIS_SAFE_ASSERT_RECOVER(primaries <= PRIMARIES_EBU_Tech_3213_E) {
                    errFile << "Attempt to export a file with unsupported primaries" << primaries;
                    return ImportExportCodes::FormatColorSpaceUnsupported;
                }
#if LIBHEIF_HAVE_VERSION(1, 14, 1)
                nclxDescription.set_color_primaries(heif_color_primaries(primaries));
#else
                nclxDescription.set_color_primaties(heif_color_primaries(primaries));
#endif
           }

           if (conversionPolicy == ConversionPolicy::ApplyPQ) {
               nclxDescription.set_transfer_characteristics(heif_transfer_characteristic_ITU_R_BT_2100_0_PQ);
           } else if (conversionPolicy == ConversionPolicy::ApplyHLG) {
               nclxDescription.set_transfer_characteristics(heif_transfer_characteristic_ITU_R_BT_2100_0_HLG);
           } else if (conversionPolicy == ConversionPolicy::ApplySMPTE428) {
               nclxDescription.set_transfer_characteristics(heif_transfer_characteristic_SMPTE_ST_428_1);
           }

           img.set_nclx_color_profile(nclxDescription);
        }


        // --- encode and write image

        heif::Context::EncodingOptions options;

        // iOS gets confused when a heif file contains an nclx.
        // but we absolutely need it for hdr.
        if (conversionPolicy != ConversionPolicy::KeepTheSame && cs->hasHighDynamicRange()) {
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
                KisMetaData::IOBackend *exifIO = KisMetadataBackendRegistry::instance()->value("exif");
                QBuffer buffer;
                exifIO->saveTo(metaDataStore.data(), &buffer, KisMetaData::IOBackend::NoHeader); // Or JpegHeader? Or something else?
                QByteArray data = buffer.data();

                // Write the data to the file
                if (data.size() > 4) {
                    ctx.add_exif_metadata(handle, data.constData(), data.size());
                }
            }
            {
                KisMetaData::IOBackend *xmpIO = KisMetadataBackendRegistry::instance()->value("xmp");
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

    int cicpPrimaries = cfg->getInt(KisImportExportFilter::CICPPrimariesTag,
                                    static_cast<int>(PRIMARIES_UNSPECIFIED));

    // Rav1e doesn't support monochrome. To get around this, people may need to convert to sRGB first.
    chkMonochromesRGB->setVisible(cfg->getString(KisImportExportFilter::ColorModelIDTag) == "GRAYA");
    
    conversionSettings->setVisible(cfg->getBool(KisImportExportFilter::HDRTag, false));

    QStringList conversionOptionsList = { i18nc("Color space name", "Rec 2100 PQ"), i18nc("Color space name", "Rec 2100 HLG")};
    QStringList toolTipList = {i18nc("@tooltip", "The image will be converted to Rec 2020 linear first, and then encoded with a perceptual quantizer curve"
                               " (also known as SMPTE 2048 curve). Recommended for HDR images where the absolute brightness is important."),
                              i18nc("@tooltip", "The image will be converted to Rec 2020 linear first, and then encoded with a Hybrid Log Gamma curve."
                               " Recommended for HDR images where the display may not understand HDR.")};
    QStringList conversionOptionName = {"Rec2100PQ", "Rec2100HLG"};
    
    if (cfg->getString(KisImportExportFilter::ColorModelIDTag) == "RGBA") {
        if (cicpPrimaries != PRIMARIES_UNSPECIFIED) {
            conversionOptionsList << i18nc("Color space option plus transfer function name", "Keep colorants, encode PQ");
            toolTipList << i18nc("@tooltip", "The image will be linearized first, and then encoded with a perceptual quantizer curve"
                                            " (also known as the SMPTE 2048 curve). Recommended for images where the absolute brightness is important.");
            conversionOptionName << "ApplyPQ";
            
            conversionOptionsList << i18nc("Color space option plus transfer function name", "Keep colorants, encode HLG");
            toolTipList << i18nc("@tooltip", "The image will be linearized first, and then encoded with a Hybrid Log Gamma curve."
                                            " Recommended for images intended for screens which cannot understand PQ");
            conversionOptionName << "ApplyHLG";
            
            conversionOptionsList << i18nc("Color space option plus transfer function name", "Keep colorants, encode SMPTE ST 428");
            toolTipList << i18nc("@tooltip", "The image will be linearized first, and then encoded with SMPTE ST 428."
                                            " Krita always opens images like these as linear floating point, this option is there to reverse that");
            conversionOptionName << "ApplySMPTE428";
        }

        conversionOptionsList << i18nc("Color space option", "No changes, clip");
        toolTipList << i18nc("@tooltip", "The image will be converted plainly to 12bit integer, and values that are out of bounds are clipped, the icc profile will be embedded.");
        conversionOptionName << "KeepSame";
    }
    cmbConversionPolicy->addItems(conversionOptionsList);
    for (int i=0; i< toolTipList.size(); i++) {
        cmbConversionPolicy->setItemData(i, toolTipList.at(i), Qt::ToolTipRole);
        cmbConversionPolicy->setItemData(i, conversionOptionName.at(i), Qt::UserRole+1);
    }
    QString optionName =
        cfg->getString("floatingPointConversionOption", "KeepSame");
    if (conversionOptionName.contains(optionName)) {
        cmbConversionPolicy->setCurrentIndex(
            conversionOptionName.indexOf(optionName));
    }
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
    cfg->setProperty("monochromeToSRGB", chkMonochromesRGB->isChecked());
    cfg->setProperty("HLGnominalPeak", spnNits->value());
    cfg->setProperty("HLGgamma", spnGamma->value());
    cfg->setProperty("removeHGLOOTF", chkHLGOOTF->isChecked());
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
