/*
 *  SPDX-FileCopyrightText: 2018 Dirk Farin <farin@struktur.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "HeifImport.h"
#include "HeifError.h"

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QBuffer>

#include <KisImportExportManager.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoColorProfile.h>
#include <KoColorTransferFunctions.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_group_layer.h>

#include <kis_meta_data_entry.h>
#include <kis_meta_data_value.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_io_backend.h>

#include "kis_iterator_ng.h"


#include "libheif/heif_cxx.h"

#include "DlgHeifImport.h"

using heif::Error;

K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_heif_import.json", registerPlugin<HeifImport>();)

HeifImport::HeifImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

HeifImport::~HeifImport()
{
}


class Reader_QIODevice : public heif::Context::Reader {
public:
  Reader_QIODevice(QIODevice* device) : m_device(device) { m_total_length=m_device->bytesAvailable(); }

  int64_t get_position() const { return m_device->pos(); }
  int read(void* data, size_t size)
  {
      qint64 readSize = m_device->read((char*)data, (qint64)size);
      return (readSize > 0 && (quint64)readSize != size);
  }
  int seek(int64_t position) { return !m_device->seek(position); }
  heif_reader_grow_status wait_for_file_size(int64_t target_size) {
    return (target_size > m_total_length) ? heif_reader_grow_status_size_beyond_eof : heif_reader_grow_status_size_reached;
  }

private:
  QIODevice* m_device;
  int64_t m_total_length;
};



KisImportExportErrorCode HeifImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    // Wrap input stream into heif Reader object
    Reader_QIODevice reader(io);

    try {
        heif::Context ctx;
        ctx.read_from_reader(reader);


        // decode primary image

        heif::ImageHandle handle = ctx.get_primary_image_handle();

        heif_color_profile_type profileType = heif_image_handle_get_color_profile_type(handle.get_raw_image_handle());


        heif::Image heifimage = handle.decode_image(heif_colorspace_undefined, heif_chroma_undefined);
        heif_colorspace heifModel = heifimage.get_colorspace();
        heif_chroma heifChroma = heifimage.get_chroma_format();
        int luma = handle.get_luma_bits_per_pixel();
        bool hasAlpha = handle.has_alpha_channel();

        dbgFile << "loading heif" << heifModel << heifChroma << luma;

        linearizePolicy linearizePolicy = keepTheSame;
        bool applyOOTF = true;
        float displayGamma = 1.2;
        float displayNits = 1000.0;

        struct heif_error err;
        const KoColorProfile *profile = 0;
        KoID colorDepth = Integer8BitsColorDepthID;
        QString colorModel = RGBAColorModelID.id();

        if (luma > 8) {
            colorDepth = Integer16BitsColorDepthID;
        }
        // First, get the profile, because sometimes the image may be encoded in YCrCb and the embedded icc profile is graya.

        if (profileType == heif_color_profile_type_prof || profileType == heif_color_profile_type_rICC) {
            dbgFile << "profile type is icc profile";
            // rICC are 'restricted' icc profiles, and are matrix shaper profiles
            // that are either RGB or Grayscale, and are of the input or display types.
            // They are from the JPEG2000 spec.

            int rawProfileSize = (int) heif_image_handle_get_raw_color_profile_size(handle.get_raw_image_handle());
            if (rawProfileSize > 0) {
                QByteArray ba(rawProfileSize, 0);
                err = heif_image_handle_get_raw_color_profile(handle.get_raw_image_handle(), ba.data());
                if (err.code) {
                    dbgFile << "icc profile loading failed:" << err.message;
                } else {
                    profile = KoColorSpaceRegistry::instance()->createColorProfile(colorModel, colorDepth.id(), ba);
                    KoColorSpaceRegistry::instance()->addProfile(profile);
                    colorModel = profile->colorModelID();
                }
            } else {
                dbgFile << "icc profile is empty";
            }
        } else if (profileType == heif_color_profile_type_nclx) {
            dbgFile << "profile type is nclx coding independant code points";
            // NCLX parameters is a colorspace description used for videofiles.
            // We generate a color profile for most entries, except that we cannot handle
            // PQ, HLG or 428 in an icc profile, so we convert those on the fly to linear.

            struct heif_color_profile_nclx *nclx = nullptr;
            err = heif_image_handle_get_nclx_color_profile(handle.get_raw_image_handle(), &nclx);
            if (err.code || !nclx) {
                dbgFile << "nclx profile loading failed" << err.message;
            } else {
                int transferCharacteristic = nclx->transfer_characteristics;
                int primaries = nclx->color_primaries;
                if (nclx->transfer_characteristics == heif_transfer_characteristic_ITU_R_BT_2100_0_PQ) {
                    dbgFile << "linearizing from PQ";
                    linearizePolicy = linearFromPQ;
                    transferCharacteristic = KoColorProfile::TRC_linear;
                }
                if (nclx->transfer_characteristics == heif_transfer_characteristic_ITU_R_BT_2100_0_HLG) {
                    dbgFile << "linearizing from HLG";
                    if (!document->fileBatchMode()) {
                        DlgHeifImport dlg(applyOOTF, displayGamma, displayNits);
                        dlg.exec();
                        applyOOTF = dlg.applyOOTF();
                        displayGamma = dlg.gamma();
                        displayNits = dlg.nominalPeakBrightness();
                    }
                    linearizePolicy = linearFromHLG;
                    transferCharacteristic = KoColorProfile::TRC_linear;
                }
                if (nclx->transfer_characteristics == heif_transfer_characteristic_SMPTE_ST_428_1) {
                    dbgFile << "linearizing from SMPTE 428";
                    linearizePolicy = linearFromSMPTE428;
                    transferCharacteristic = KoColorProfile::TRC_linear;
                }

                if (nclx->transfer_characteristics == heif_transfer_characteristic_IEC_61966_2_4 ||
                        nclx->transfer_characteristics == heif_transfer_characteristic_ITU_R_BT_1361) {
                    transferCharacteristic = heif_transfer_characteristic_ITU_R_BT_709_5;
                }

                KoColorSpaceEngine *engine = KoColorSpaceEngineRegistry::instance()->get("icc");
                if (engine) {

                    QVector<double>colorants = {nclx->color_primary_white_x, nclx->color_primary_white_y,
                                                nclx->color_primary_red_x, nclx->color_primary_red_y,
                                                nclx->color_primary_green_x, nclx->color_primary_green_y,
                                                nclx->color_primary_blue_x, nclx->color_primary_blue_y};

                    if (primaries == KoColorProfile::Primaries_Unspecified) {
                        colorants.clear();
                    }

                    profile = engine->getProfile(colorants,
                                                 primaries,
                                                 transferCharacteristic);

                    if (linearizePolicy != keepTheSame) {
                        colorDepth = Float32BitsColorDepthID;
                    }

                } else {
                    if (nclx->color_primaries == heif_color_primaries_ITU_R_BT_2020_2_and_2100_0 &&
                            nclx->transfer_characteristics == heif_transfer_characteristic_ITU_R_BT_2100_0_PQ) {
                        profile = KoColorSpaceRegistry::instance()->p2020PQProfile();
                    }
                }

                heif_nclx_color_profile_free(nclx);
                dbgFile << "nclx profile found" << profile->name();
            }
        } else {
            dbgFile << "no profile found";
        }

        // Now, to figure out the correct chroma and color model.
        if (heifModel == heif_colorspace_monochrome || colorModel == GrayAColorModelID.id()) {
            // Grayscale image.
            if (heifChroma != heif_chroma_monochrome && colorModel == GrayAColorModelID.id()) {
                heifimage = handle.decode_image(heif_colorspace_YCbCr, heif_chroma_monochrome);
            }
            colorModel = GrayAColorModelID.id();
            heifChroma = heif_chroma_monochrome;

        }

        // Get the default profile if we haven't found one up till now.
        if (!profile) {
            const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(colorModel, colorDepth.id());
            QString profileName = KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId);
            profile = KoColorSpaceRegistry::instance()->profileByName(profileName);
        }

        const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth.id(), profile);

        int width  = handle.get_width();
        int height = handle.get_height();

        // convert HEIF image to Krita KisDocument

        KisImageSP image = new KisImage(document->createUndoStore(), width, height, colorSpace,
                                        "HEIF image");

        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), OPACITY_OPAQUE_U8);

        const double max16bit = 65535.0;
        const double multiplier10bit = double(1.0 / 1023.0);
        const double multiplier12bit = double(1.0 / 4095.0);
        const double multiplier16bit = double(1.0 / max16bit);

        if (luma != 8 && luma != 10 && luma != 12) {
            dbgFile << "unknown bitdepth" << luma;
        }

        if (heifChroma == heif_chroma_monochrome) {
            dbgFile << "monochrome heif file, bits:" << luma;
            int strideG, strideA;
            const uint8_t* imgG = heifimage.get_plane(heif_channel_Y, &strideG);
            const uint8_t* imgA = heifimage.get_plane(heif_channel_Alpha, &strideA);
            KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, 0, width);

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    if (luma == 8) {
                        KoGrayU8Traits::setGray(it->rawData(), imgG[y * strideG + x]);

                        if (hasAlpha) {
                            KoGrayU8Traits::setOpacity(it->rawData(), quint8(imgA[y * strideA + x]), 1);
                        } else {
                            KoGrayU8Traits::setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
                        }
                    } else {
                        uint16_t source = KoGrayU16Traits::nativeArray(imgG)[y * (strideG / 2) + (x)];

                        if (luma == 10) {
                            KoGrayU16Traits::setGray(it->rawData(), float(0x03ff & (source)) * multiplier10bit * max16bit);
                        } else if (luma == 12) {
                            KoGrayU16Traits::setGray(it->rawData(), float(0x0fff & (source)) * multiplier12bit * max16bit);
                        } else {
                            KoGrayU16Traits::setGray(it->rawData(), float(source) * multiplier16bit);
                        }

                        if (hasAlpha) {
                            source = KoGrayU16Traits::nativeArray(imgA)[y * (strideA / 2) + x];
                            if (luma == 10) {
                                KoGrayU16Traits::setOpacity(it->rawData(), float(0x0fff & (source)) * multiplier10bit, 1);
                            } else if (luma == 12) {
                                KoGrayU16Traits::setOpacity(it->rawData(), float(0x0fff & (source)) * multiplier12bit, 1);
                            } else {
                                KoGrayU16Traits::setOpacity(it->rawData(), float(source) * multiplier16bit, 1);
                            }
                        } else {
                            KoGrayU16Traits::setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
                        }
                    }

                    it->nextPixel();
                }

                it->nextRow();
            }

        } else if (heifChroma == heif_chroma_444) {
            dbgFile << "planar heif file, bits:" << luma;

            int strideR, strideG, strideB, strideA;
            const uint8_t* imgR = heifimage.get_plane(heif_channel_R, &strideR);
            const uint8_t* imgG = heifimage.get_plane(heif_channel_G, &strideG);
            const uint8_t* imgB = heifimage.get_plane(heif_channel_B, &strideB);
            const uint8_t* imgA = heifimage.get_plane(heif_channel_Alpha, &strideA);
            QVector<qreal> lCoef {colorSpace->lumaCoefficients()};
            QVector<float> pixelValues(4);
            KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, 0, width);

            auto value =
                [&](const uint8_t *img, int stride, int x, int y) {
                    if (luma == 8) {
                        return linearizeValueAsNeeded(float(img[y * (stride) + x]) / 255.0f, linearizePolicy);
                    } else {
                        uint16_t source = reinterpret_cast<const uint16_t *>(img)[y * (stride / 2) + x];
                        if (luma == 10) {
                            return linearizeValueAsNeeded(float(0x03ff & (source)) * multiplier10bit, linearizePolicy);
                        } else if (luma == 12) {
                            return linearizeValueAsNeeded(float(0x0fff & (source)) * multiplier12bit, linearizePolicy);
                        } else {
                            return linearizeValueAsNeeded(float(source) * multiplier16bit, linearizePolicy);
                        }
                    }
                };

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    std::fill(pixelValues.begin(), pixelValues.end(), 1.0f);

                    pixelValues[0] = value(imgR, strideR, x, y);
                    pixelValues[1] = value(imgG, strideG, x, y);
                    pixelValues[2] = value(imgB, strideB, x, y);

                    if (hasAlpha) {
                        pixelValues[3] = value(imgA, strideA, x, y);
                    }

                    if (linearizePolicy == keepTheSame) {
                        qSwap(pixelValues.begin()[0], pixelValues.begin()[2]);
                    }
                    if (linearizePolicy == linearFromHLG && applyOOTF) {
                        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
                    }
                    colorSpace->fromNormalisedChannelsValue(it->rawData(), pixelValues);

                    it->nextPixel();
                }

                it->nextRow();
            }
        } else if (heifChroma == heif_chroma_interleaved_RGB || heifChroma == heif_chroma_interleaved_RGBA) {
            int stride;
            dbgFile << "interleaved SDR heif file, bits:" << luma;

            const uint8_t *img = heifimage.get_plane(heif_channel_interleaved, &stride);
            QVector<float> pixelValues(4);
            QVector<qreal> lCoef {colorSpace->lumaCoefficients()};
            KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, 0, width);
            int channels = hasAlpha ? 4 : 3;

            auto value = [&](const uint8_t *img, int stride, int x, int y, int ch) {
                uint8_t source = img[(y * stride) + (x * channels) + ch];
                return linearizeValueAsNeeded(float(source) / 255.0f, linearizePolicy);
            };

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    std::fill(pixelValues.begin(), pixelValues.end(), 1.0f);

                    for (int ch = 0; ch < channels; ch++) {
                        pixelValues[ch] = value(img, stride, x, y, ch);
                    }

                    if (linearizePolicy == keepTheSame) {
                        qSwap(pixelValues.begin()[0], pixelValues.begin()[2]);
                    }
                    if (linearizePolicy == linearFromHLG && applyOOTF) {
                        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
                    }
                    colorSpace->fromNormalisedChannelsValue(it->rawData(), pixelValues);

                    it->nextPixel();
                }

                it->nextRow();
            }
        } else if (heifChroma == heif_chroma_interleaved_RRGGBB_LE || heifChroma == heif_chroma_interleaved_RRGGBBAA_LE || heifChroma == heif_chroma_interleaved_RRGGBB_BE || heifChroma == heif_chroma_interleaved_RRGGBB_BE) {
            int stride;
            dbgFile << "interleaved HDR heif file, bits:" << luma;

            const uint8_t* img = heifimage.get_plane(heif_channel_interleaved, &stride);
            QVector<float> pixelValues(4);
            QVector<qreal> lCoef {colorSpace->lumaCoefficients()};
            KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, 0, width);
            int channels = hasAlpha ? 4 : 3;

            auto value = [&](const uint8_t *img, int stride, int x, int y, int ch) {
                uint16_t source = reinterpret_cast<const uint16_t *>(img)[y * (stride / 2) + (x * channels) + ch];
                if (luma == 10) {
                    return linearizeValueAsNeeded(float(0x03ff & (source)) * multiplier10bit, linearizePolicy);
                } else if (luma == 12) {
                    return linearizeValueAsNeeded(float(0x0fff & (source)) * multiplier12bit, linearizePolicy);
                } else {
                    return linearizeValueAsNeeded(float(source) * multiplier16bit, linearizePolicy);
                }
            };

            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    std::fill(pixelValues.begin(), pixelValues.end(), 1.0f);

                    for (int ch = 0; ch < channels; ch++) {
                        pixelValues[ch] = value(img, stride, x, y, ch);
                    }

                    if (linearizePolicy == keepTheSame) {
                        qSwap(pixelValues.begin()[0], pixelValues.begin()[2]);
                    }
                    if (linearizePolicy == linearFromHLG && applyOOTF) {
                        applyHLGOOTF(pixelValues, lCoef, displayGamma, displayNits);
                    }
                    colorSpace->fromNormalisedChannelsValue(it->rawData(), pixelValues);

                    it->nextPixel();
                }

                it->nextRow();
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
              size_t skip = ((exif_data[0]<<24) | (exif_data[1]<<16) | (exif_data[2]<<8) | exif_data[3]) + 4;

              if (exif_data.size()>skip) {
                KisMetaData::IOBackend* exifIO = KisMetaData::IOBackendRegistry::instance()->value("exif");

                // Copy the exif data into the byte array
                QByteArray ba(reinterpret_cast<char *>(exif_data.data()+skip), static_cast<int>(exif_data.size()-skip));
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
            QByteArray ba(reinterpret_cast<char *>(xmp_data.data()), static_cast<int>(xmp_data.size()));
            QBuffer buf(&ba);
            xmpIO->loadFrom(layer->metaData(), &buf);
          }
        }

        document->setCurrentImage(image);
        return ImportExportCodes::OK;
    } catch (Error &err) {
        return setHeifError(document, err);
    }
}

float HeifImport::linearizeValueAsNeeded(float value, HeifImport::linearizePolicy policy)
{
    if ( policy == linearFromPQ) {
        return removeSmpte2048Curve(value);
    } else if ( policy == linearFromHLG) {
        return removeHLGCurve(value);
    } else if ( policy == linearFromSMPTE428) {
        return removeSMPTE_ST_428Curve(value);
    }
    return value;
}

#include <HeifImport.moc>
