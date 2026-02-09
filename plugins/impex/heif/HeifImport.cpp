/*
 *  SPDX-FileCopyrightText: 2018 Dirk Farin <farin@struktur.de>
 *  SPDX-FileCopyrightText: 2020-2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Daniel Novomesky <dnovomesky@gmail.com>
 *  SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "HeifImport.h"
#include "HeifError.h"

#include <QBuffer>

#include <kpluginfactory.h>
#include <libheif/heif.h>
#include <libheif/heif_cxx.h>

#include <KisDocument.h>
#include <KisImportExportManager.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceEngine.h>
#include <KoColorSpaceRegistry.h>
#include <dialogs/kis_dlg_hlg_import.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_iterator_ng.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_meta_data_entry.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_value.h>
#include <kis_node.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_transaction.h>
#include <qmutex.h>

#include "kis_heif_import_tools.h"

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
    Reader_QIODevice(QIODevice *device)
        : m_device(device)
        , m_total_length(m_device->bytesAvailable())
    {
    }

  int64_t get_position() const override
  {
      return m_device->pos();
  }
  int read(void *data, size_t size) override
  {
      qint64 readSize =
          m_device->read(static_cast<char *>(data), static_cast<qint64>(size));
      return (readSize > 0 && readSize != static_cast<qint64>(size));
  }
  int seek(int64_t position) override
  {
      return !m_device->seek(position);
  }
  heif_reader_grow_status wait_for_file_size(int64_t target_size) override
  {
      return (target_size > m_total_length)
          ? heif_reader_grow_status_size_beyond_eof
          : heif_reader_grow_status_size_reached;
  }

private:
  QIODevice* m_device;
  int64_t m_total_length;
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

namespace Planar
{
inline auto readPlanarLayer(const int luma,
                            LinearizePolicy policy,
                            bool applyOOTF,
                            bool hasAlpha,
                            const int width,
                            const int height,
                            const uint8_t *imgR,
                            const int strideR,
                            const uint8_t *imgG,
                            const int strideG,
                            const uint8_t *imgB,
                            const int strideB,
                            const uint8_t *imgA,
                            const int strideA,
                            KisHLineIteratorSP it,
                            float displayGamma,
                            float displayNits,
                            const KoColorSpace *colorSpace)
{
    return createOptimizedClass<readLayerImpl>(luma,
                                               policy,
                                               applyOOTF,
                                               hasAlpha,
                                               width,
                                               height,
                                               imgR,
                                               strideR,
                                               imgG,
                                               strideG,
                                               imgB,
                                               strideB,
                                               imgA,
                                               strideA,
                                               it,
                                               displayGamma,
                                               displayNits,
                                               colorSpace);
}
} // namespace Planar

namespace HDR
{
template<typename... Args>
inline auto readInterleavedLayer(const int luma,
                                 LinearizePolicy linearizePolicy,
                                 bool applyOOTF,
                                 const int channels,
                                 const int width,
                                 const int height,
                                 const uint8_t *img,
                                 const int stride,
                                 KisHLineIteratorSP it,
                                 float displayGamma,
                                 float displayNits,
                                 const KoColorSpace *colorSpace)
{
    return createOptimizedClass<HDR::readLayerImpl>(luma,
                                                    linearizePolicy,
                                                    applyOOTF,
                                                    channels,
                                                    width,
                                                    height,
                                                    img,
                                                    stride,
                                                    it,
                                                    displayGamma,
                                                    displayNits,
                                                    colorSpace);
}
} // namespace HDR

namespace SDR
{
template<typename... Args>
inline auto readInterleavedLayer(LinearizePolicy linearizePolicy,
                                 bool applyOOTF,
                                 const int channels,
                                 const int width,
                                 const int height,
                                 const uint8_t *img,
                                 const int stride,
                                 KisHLineIteratorSP it,
                                 float displayGamma,
                                 float displayNits,
                                 const KoColorSpace *colorSpace)
{
    return createOptimizedClass<SDR::readLayerImpl>(linearizePolicy,
                                                    applyOOTF,
                                                    channels,
                                                    width,
                                                    height,
                                                    img,
                                                    stride,
                                                    it,
                                                    displayGamma,
                                                    displayNits,
                                                    colorSpace);
}
} // namespace SDR

KisImportExportErrorCode HeifImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
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

        LinearizePolicy linearizePolicy = LinearizePolicy::KeepTheSame;
        bool applyOOTF = true;
        float displayGamma = 1.2f;
        float displayNits = 1000.0;

        struct heif_error err {
        };
        const KoColorProfile *profile = nullptr;
        KoID colorDepth = Integer8BitsColorDepthID;
        QString colorModel = RGBAColorModelID.id();

        if (luma > 8) {
            colorDepth = Integer16BitsColorDepthID;
        }
        // First, get the profile, because sometimes the image may be encoded in YCbCr and the embedded icc profile is graya.

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
            dbgFile << "profile type is nclx coding independent code points";
            // NCLX parameters is a colorspace description used for videofiles.
            // We generate a color profile for most entries, except that we cannot handle
            // PQ, HLG or 428 in an icc profile, so we convert those on the fly to linear.

            struct heif_color_profile_nclx *nclx = nullptr;
            err = heif_image_handle_get_nclx_color_profile(handle.get_raw_image_handle(), &nclx);
            if (err.code || !nclx) {
                dbgFile << "nclx profile loading failed" << err.message;
            } else {
                TransferCharacteristics transferCharacteristic = TransferCharacteristics(nclx->transfer_characteristics);
                ColorPrimaries primaries = ColorPrimaries(nclx->color_primaries);
                if (nclx->transfer_characteristics == heif_transfer_characteristic_ITU_R_BT_2100_0_PQ) {
                    dbgFile << "linearizing from PQ";
                    linearizePolicy = LinearizePolicy::LinearFromPQ;
                    transferCharacteristic = TRC_LINEAR;
                }
                if (nclx->transfer_characteristics == heif_transfer_characteristic_ITU_R_BT_2100_0_HLG) {
                    dbgFile << "linearizing from HLG";
                    if (!document->fileBatchMode()) {
                        KisDlgHLGImport dlg(applyOOTF, displayGamma, displayNits);
                        dlg.exec();
                        applyOOTF = dlg.applyOOTF();
                        displayGamma = dlg.gamma();
                        displayNits = dlg.nominalPeakBrightness();
                    }
                    linearizePolicy = LinearizePolicy::LinearFromHLG;
                    transferCharacteristic = TRC_LINEAR;
                }
                if (nclx->transfer_characteristics == heif_transfer_characteristic_SMPTE_ST_428_1) {
                    dbgFile << "linearizing from SMPTE 428";
                    linearizePolicy = LinearizePolicy::LinearFromSMPTE428;
                    transferCharacteristic = TRC_LINEAR;
                }

                if (nclx->transfer_characteristics == heif_transfer_characteristic_IEC_61966_2_4 ||
                        nclx->transfer_characteristics == heif_transfer_characteristic_ITU_R_BT_1361) {
                    transferCharacteristic = TRC_ITU_R_BT_709_5;
                }

                const QVector<double> colorants = [&]() -> QVector<double> {
                    if (primaries == PRIMARIES_UNSPECIFIED) {
                        return {};
                    } else {
                        return {
                            static_cast<double>(nclx->color_primary_white_x),
                            static_cast<double>(nclx->color_primary_white_y),
                            static_cast<double>(nclx->color_primary_red_x),
                            static_cast<double>(nclx->color_primary_red_y),
                            static_cast<double>(nclx->color_primary_green_x),
                            static_cast<double>(nclx->color_primary_green_y),
                            static_cast<double>(nclx->color_primary_blue_x),
                            static_cast<double>(nclx->color_primary_blue_y)};
                    }
                }();

                profile = KoColorSpaceRegistry::instance()->profileFor(colorants,
                                                                       primaries,
                                                                       transferCharacteristic);

                if (linearizePolicy != LinearizePolicy::KeepTheSame) {
                    colorDepth = Float32BitsColorDepthID;
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
        } else if (heifModel == heif_colorspace_YCbCr) {
            // we do not support importing YCrBr spaces directly
            heifimage = handle.decode_image(heif_colorspace_RGB, heif_chroma_undefined);
            colorModel = RGBAColorModelID.id();
            heifModel = heifimage.get_colorspace();
            heifChroma = heifimage.get_chroma_format();
        }

        // Get the default profile if we haven't found one up till now.
        if (!profile) {
            if (colorModel == RGBAColorModelID.id()) {
                profile = KoColorSpaceRegistry::instance()->profileFor(QVector<double>(),
                                                                       PRIMARIES_ITU_R_BT_709_5,
                                                                       TRC_IEC_61966_2_1);
            } else {
                const QString colorSpaceId = KoColorSpaceRegistry::instance()->colorSpaceId(colorModel, colorDepth.id());
                QString profileName = KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(colorSpaceId);
                profile = KoColorSpaceRegistry::instance()->profileByName(profileName);
            }
        }

        const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorModel, colorDepth.id(), profile);

        int width  = handle.get_width();

        int height = handle.get_height();

        // convert HEIF image to Krita KisDocument

        KisImageSP image = new KisImage(document->createUndoStore(), width, height, colorSpace,
                                        "HEIF image");

        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), OPACITY_OPAQUE_U8);

        if (luma != 8 && luma != 10 && luma != 12) {
            dbgFile << "unknown bitdepth" << luma;
        }

        if (heifChroma == heif_chroma_monochrome) {
            dbgFile << "monochrome heif file, bits:" << luma;
            HeifStrideType strideG = 0;
            HeifStrideType strideA = 0;
            const uint8_t *imgG = heifGetPlaneMethod(heifimage, heif_channel_Y, &strideG);
            const uint8_t *imgA =
                heifGetPlaneMethod(heifimage, heif_channel_Alpha, &strideA);
            const int width = heifimage.get_width(heif_channel_Y);
            const int height = heifimage.get_height(heif_channel_Y);
            KisHLineIteratorSP it =
                layer->paintDevice()->createHLineIteratorNG(0, 0, width);

            Gray::readPlanarLayer(luma,
                                  hasAlpha,
                                  width,
                                  height,
                                  it,
                                  imgG,
                                  imgA,
                                  strideG,
                                  strideA);
        } else if (heifChroma == heif_chroma_444) {
            dbgFile << "planar heif file, bits:" << luma;

            HeifStrideType strideR = 0;
            HeifStrideType strideG = 0;
            HeifStrideType strideB = 0;
            HeifStrideType strideA = 0;
            const uint8_t* imgR = heifGetPlaneMethod(heifimage, heif_channel_R, &strideR);
            const uint8_t* imgG = heifGetPlaneMethod(heifimage, heif_channel_G, &strideG);
            const uint8_t* imgB = heifGetPlaneMethod(heifimage, heif_channel_B, &strideB);
            const uint8_t *imgA =
                heifGetPlaneMethod(heifimage, heif_channel_Alpha, &strideA);
            KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, 0, width);

            Planar::readPlanarLayer(luma,
                                    linearizePolicy,
                                    applyOOTF,
                                    hasAlpha,
                                    width,
                                    height,
                                    imgR,
                                    strideR,
                                    imgG,
                                    strideG,
                                    imgB,
                                    strideB,
                                    imgA,
                                    strideA,
                                    it,
                                    displayGamma,
                                    displayNits,
                                    colorSpace);
        } else if (heifChroma == heif_chroma_interleaved_RGB || heifChroma == heif_chroma_interleaved_RGBA) {
            HeifStrideType stride = 0;
            dbgFile << "interleaved SDR heif file, bits:" << luma;

            const uint8_t *img = heifGetPlaneMethod(heifimage, heif_channel_interleaved, &stride);
            width = heifimage.get_width(heif_channel_interleaved);
            height = heifimage.get_height(heif_channel_interleaved);
            KisHLineIteratorSP it =
                layer->paintDevice()->createHLineIteratorNG(0, 0, width);

            SDR::readInterleavedLayer(linearizePolicy,
                                      applyOOTF,
                                      hasAlpha,
                                      width,
                                      height,
                                      img,
                                      stride,
                                      it,
                                      displayGamma,
                                      displayNits,
                                      colorSpace);

        } else if (heifChroma == heif_chroma_interleaved_RRGGBB_LE || heifChroma == heif_chroma_interleaved_RRGGBBAA_LE || heifChroma == heif_chroma_interleaved_RRGGBB_BE || heifChroma == heif_chroma_interleaved_RRGGBB_BE) {
            HeifStrideType stride = 0;
            dbgFile << "interleaved HDR heif file, bits:" << luma;

            const uint8_t *img =
                heifGetPlaneMethod(heifimage, heif_channel_interleaved, &stride);
            KisHLineIteratorSP it =
                layer->paintDevice()->createHLineIteratorNG(0, 0, width);

            HDR::readInterleavedLayer(luma,
                                      linearizePolicy,
                                      applyOOTF,
                                      hasAlpha,
                                      width,
                                      height,
                                      img,
                                      stride,
                                      it,
                                      displayGamma,
                                      displayNits,
                                      colorSpace);
        }

        image->addNode(layer.data(), image->rootLayer().data());
        image->cropImage(QRect(0, 0, width, height));

        // --- Iterate through all metadata blocks and extract Exif and XMP metadata ---

        std::vector<heif_item_id> metadata_IDs = handle.get_list_of_metadata_block_IDs();

        for (heif_item_id id : metadata_IDs) {

          if (handle.get_metadata_type(id) == "Exif") {
            // Read exif information

            std::vector<uint8_t> exif_data = handle.get_metadata(id);

            if (exif_data.size()>4) {
                size_t skip = ((quint32(exif_data[0]) << 24U)
                               | (quint32(exif_data[1])) << 16U
                               | (quint32(exif_data[2]) << 8U) | exif_data[3])
                    + 4u;

                if (exif_data.size() > skip) {
                    KisMetaData::IOBackend *exifIO =
                        KisMetadataBackendRegistry::instance()->value("exif");

                    // Copy the exif data into the byte array
                    QByteArray ba(
                        reinterpret_cast<char *>(exif_data.data() + skip),
                        static_cast<int>(exif_data.size() - skip));
                    QBuffer buf(&ba);
                    exifIO->loadFrom(layer->metaData(), &buf);
                }
            }
          }

          if (handle.get_metadata_type(id) == "mime" &&
              handle.get_metadata_content_type(id) == "application/rdf+xml") {
            // Read XMP information

            std::vector<uint8_t> xmp_data = handle.get_metadata(id);
            KisMetaData::IOBackend *xmpIO = KisMetadataBackendRegistry::instance()->value("xmp");

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

#include <HeifImport.moc>
