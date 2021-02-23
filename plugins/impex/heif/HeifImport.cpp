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


        // Get the colorprofile. Default to sRGB if no profile is available.
        const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        KoID colorModel = colorSpace->colorModelId();
        KoID colorDepth = colorSpace->colorDepthId();

        heif::Image heifimage = handle.decode_image(heif_colorspace_undefined, heif_chroma_undefined);
        heif_colorspace heifModel = heifimage.get_colorspace();
        heif_chroma heifChroma = heifimage.get_chroma_format();
        qDebug() << "loading heif" << heifModel << heifChroma << handle.get_luma_bits_per_pixel();

        if (heifModel == heif_colorspace_monochrome) {
            // Grayscale image.
            colorModel = GrayAColorModelID;
            if (heifChroma == heif_chroma_monochrome) {
                colorSpace = KoColorSpaceRegistry::instance()->graya8();
            }
        } else {
            // RGB
            heifimage = handle.decode_image(heif_colorspace_RGB, heif_chroma_444);
            colorModel = RGBAColorModelID;
            if (handle.get_luma_bits_per_pixel() == 8) {
                colorDepth = Integer8BitsColorDepthID;
            } else {
                if (handle.has_alpha_channel()) {
                    heifimage = handle.decode_image(heif_colorspace_RGB, heif_chroma_interleaved_RRGGBBAA_LE);
                } else {
                    heifimage = handle.decode_image(heif_colorspace_RGB, heif_chroma_interleaved_RRGGBB_LE);
                }
                heifChroma = heifimage.get_chroma_format();
                colorDepth = Integer16BitsColorDepthID;
            }
        }


        heif_color_profile_type profileType = heif_image_handle_get_color_profile_type(handle.get_raw_image_handle());
        const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(colorModel, colorDepth);
        QString profileName = KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId);

        qDebug() << "profile" << profileType;

        struct heif_error err;
        if (profileType == heif_color_profile_type_prof || profileType == heif_color_profile_type_rICC) {
            // rICC are 'restricted' icc profiles, and are matrix shaper profiles
            // that are either RGB or Grayscale, and are of the input or display types.
            // They are from the JPEG2000 spec.

            int rawProfileSize = (int) heif_image_handle_get_raw_color_profile_size(handle.get_raw_image_handle());
            if (rawProfileSize > 0) {
                QByteArray ba(rawProfileSize, 0);
                err = heif_image_handle_get_raw_color_profile(handle.get_raw_image_handle(), ba.data());
                if (err.code) {
                    qDebug() << "icc profile loading failed";
                } else {
                    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->createColorProfile(colorModel.id(), colorDepth.id(), ba);
                    KoColorSpaceRegistry::instance()->addProfile(profile);
                    profileName = profile->name();
                    qDebug() << "icc profile found" << profileName;
                }
            } else {
                qDebug() << "icc profile is empty";
            }
        } else if (profileType == heif_color_profile_type_nclx) {
            // NCLX parameters is a colorspace description used for videofiles.
            // We will need to generate a profile based on nclx parameters. We can use lcms for this, but code doesn't exist yet.

            //For now, we can try to get the profile we always have on hand...

            struct heif_color_profile_nclx *nclx = nullptr;
            err = heif_image_handle_get_nclx_color_profile(handle.get_raw_image_handle(), &nclx);
            if (err.code || !nclx) {
                qDebug() << "nclx profile loading failed";
            } else {
                KoColorSpaceEngine *engine = KoColorSpaceEngineRegistry::instance()->get("icc");
                if (engine) {
                    QVector<double>colorants = {nclx->color_primary_white_x, nclx->color_primary_white_y,
                                               nclx->color_primary_red_x, nclx->color_primary_red_y,
                                               nclx->color_primary_green_x, nclx->color_primary_green_y,
                                               nclx->color_primary_blue_x, nclx->color_primary_blue_y};
                    const KoColorProfile *profile = engine->generateAndAddProfile(colorants, nclx->color_primaries, nclx->transfer_characteristics);
                    profileName = profile->name();
                } else {
                    if (nclx->color_primaries == heif_color_primaries_ITU_R_BT_2020_2_and_2100_0 &&
                            nclx->transfer_characteristics == heif_transfer_characteristic_ITU_R_BT_2100_0_PQ) {
                        profileName = KoColorSpaceRegistry::instance()->p2020PQProfile()->name();
                    }
                }

                heif_nclx_color_profile_free(nclx);
                qDebug() << "nclx profile found" << profileName;
            }
        } else {
            qDebug() << "no profile found";
        }


        colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorModel.id(), colorDepth.id(), profileName);

        int width  = handle.get_width();
        int height = handle.get_height();
        bool hasAlpha = handle.has_alpha_channel();


        // convert HEIF image to Krita KisDocument
        KisImageSP image = new KisImage(document->createUndoStore(), width, height, colorSpace,
                                        "HEIF image");

        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), OPACITY_OPAQUE_U8);

        if (heifChroma == heif_chroma_monochrome) {
            qDebug() << "monochrome heif file, bits:" << handle.get_luma_bits_per_pixel();
            int strideG, strideA;
            const uint8_t* imgG = heifimage.get_plane(heif_channel_Y, &strideG);
            const uint8_t* imgA = heifimage.get_plane(heif_channel_Alpha, &strideA);

            for (int y=0;y<height;y++) {
                KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, y, width);

                for (int x=0;x<width;x++) {
                    KoGrayTraits<quint8>::setGray(it->rawData(), imgG[ y * strideG + x ]);

                    if (hasAlpha) {
                        colorSpace->setOpacity(it->rawData(), quint8(imgA[y*strideA+x]), 1);
                    }
                    else {
                        colorSpace->setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
                    }

                    it->nextPixel();
                }
            }

        } else if (heifChroma == heif_chroma_444) {
            qDebug() << "planar heif file, bits:" << handle.get_luma_bits_per_pixel();

            int strideR, strideG, strideB, strideA;
            const uint8_t* imgR = heifimage.get_plane(heif_channel_R, &strideR);
            const uint8_t* imgG = heifimage.get_plane(heif_channel_G, &strideG);
            const uint8_t* imgB = heifimage.get_plane(heif_channel_B, &strideB);
            const uint8_t* imgA = heifimage.get_plane(heif_channel_Alpha, &strideA);

            for (int y=0; y < height; y++) {
                KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, y, width);

                for (int x=0; x < width; x++) {

                    KoBgrTraits<quint8>::setRed(  it->rawData(), imgR[ y * strideR + x]);
                    KoBgrTraits<quint8>::setGreen(it->rawData(), imgG[ y * strideG + x]);
                    KoBgrTraits<quint8>::setBlue( it->rawData(), imgB[ y * strideB + x]);

                    if (hasAlpha) {
                        colorSpace->setOpacity(it->rawData(), quint8(imgA[y*strideA+x]), 1);
                    }
                    else {
                        colorSpace->setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
                    }

                    it->nextPixel();
                }
            }
        } else if (heifChroma == heif_chroma_interleaved_RRGGBB_LE || heifChroma == heif_chroma_interleaved_RRGGBBAA_LE)  {
            qDebug() << "interleaved heif file, bits:" << handle.get_luma_bits_per_pixel();
            int stride;

            const uint8_t* img = heifimage.get_plane(heif_channel_interleaved, &stride);

            for (int y=0; y < height; y++) {
                KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, y, width);

                for (int x=0; x < width; x++) {

                    QVector<float> pixelValues(4);
                    pixelValues.fill(1.0);

                    int channels = hasAlpha? 4: 3;
                    for (int ch = 0; ch < channels; ch++) {
                        uint16_t source = reinterpret_cast<const uint16_t*>(img)[y * (stride/2) + (x*channels) + ch];
                        if (handle.get_luma_bits_per_pixel() == 10) {

                            pixelValues[ch] = float(0x03ff & (source)) / 1023.0;

                        } else if (handle.get_luma_bits_per_pixel() == 12) {
                            pixelValues[ch] = float(0x0fff & (source)) / 4095.0;

                        } else {
                            qDebug() << "unknown bitdepth" << handle.get_luma_bits_per_pixel();
                            pixelValues[ch] = float(source)/65535.0;
                        }


                    }
                    qSwap(pixelValues.begin()[0], pixelValues.begin()[2]);
                    colorSpace->fromNormalisedChannelsValue(it->rawData(), pixelValues);

                    it->nextPixel();
                }


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


        document->setCurrentImage(image);
        return ImportExportCodes::OK;
    }
    catch (heif::Error err) {
        return setHeifError(document, err);
    }
}

#include <HeifImport.moc>
