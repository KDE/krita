/*
 *  SPDX-FileCopyrightText: 2021 the JPEG XL Project Authors
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "JPEGXLImport.h"

#include <jxl/decode_cxx.h>
#include <jxl/resizable_parallel_runner_cxx.h>
#include <kpluginfactory.h>

#include <QBuffer>
#include <cstring>

#include <KisDocument.h>
#include <KisImportExportErrorCode.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include <kis_assert.h>
#include <kis_debug.h>
#include <kis_group_layer.h>
#include <kis_image_animation_interface.h>
#include <kis_iterator_ng.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_paint_layer.h>
#include <kis_raster_keyframe_channel.h>

K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_jxl_import.json", registerPlugin<JPEGXLImport>();)

constexpr static std::array<char, 5> exifTag = {'E', 'x', 'i', 'f', 0x0};
constexpr static std::array<char, 5> xmpTag = {'x', 'm', 'l', ' ', 0x0};

class Q_DECL_HIDDEN JPEGXLImportData
{
public:
    JPEGXLImport *m{nullptr};
    JxlBasicInfo m_info{};
    JxlPixelFormat m_pixelFormat{};
    JxlFrameHeader m_header{};
    KisPaintDeviceSP m_currentFrame{nullptr};
    int m_nextFrameTime{0};
    int m_durationFrameInTicks{0};
    KoID m_colorID;
    KoID m_depthID;
    bool m_forcedConversion;
};

template<class Traits>
void imageOutRgbCallback(void *that, size_t x, size_t y, size_t numPixels, const void *pixels)
{
    auto *data = static_cast<JPEGXLImportData *>(that);
    KIS_ASSERT(data);

    using Pixel = typename Traits::Pixel;
    using channels_type = typename Traits::channels_type;

    auto it = data->m_currentFrame->createHLineIteratorNG(static_cast<int>(x),
                                                          static_cast<int>(y),
                                                          static_cast<int>(data->m_info.xsize));
    const auto *src = static_cast<const channels_type *>(pixels);

    for (size_t i = 0; i < numPixels; i++) {
        auto *dst = reinterpret_cast<Pixel *>(it->rawData());

        std::memcpy(dst, src, (data->m_pixelFormat.num_channels) * sizeof(channels_type));

        std::swap(dst->blue, dst->red);

        src += data->m_pixelFormat.num_channels;

        it->nextPixel();
    }
}

template<typename channels_type>
void imageOutSizedCallback(void *that, size_t x, size_t y, size_t numPixels, const void *pixels)
{
    auto *data = static_cast<JPEGXLImportData *>(that);
    KIS_ASSERT(data);

    auto it = data->m_currentFrame->createHLineIteratorNG(static_cast<int>(x),
                                                          static_cast<int>(y),
                                                          static_cast<int>(data->m_info.xsize));
    const auto *src = static_cast<const channels_type *>(pixels);

    for (size_t i = 0; i < numPixels; i++) {
        auto *dst = reinterpret_cast<channels_type *>(it->rawData());

        std::memcpy(dst, src, (data->m_pixelFormat.num_channels) * sizeof(channels_type));

        src += data->m_pixelFormat.num_channels;

        it->nextPixel();
    }
}

JPEGXLImport::JPEGXLImport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KisImportExportErrorCode
JPEGXLImport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP /*configuration*/)
{
    if (!io->isReadable()) {
        errFile << "Cannot read image contents";
        return ImportExportCodes::NoAccessToRead;
    }

    JPEGXLImportData d{};

    // Multi-threaded parallel runner.
    auto runner = JxlResizableParallelRunnerMake(nullptr);
    auto dec = JxlDecoderMake(nullptr);

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(runner && dec, ImportExportCodes::InternalError);

    if (JXL_DEC_SUCCESS
        != JxlDecoderSubscribeEvents(dec.get(),
                                     JXL_DEC_BASIC_INFO | JXL_DEC_COLOR_ENCODING | JXL_DEC_FULL_IMAGE | JXL_DEC_BOX
                                         | JXL_DEC_FRAME)) {
        errFile << "JxlDecoderSubscribeEvents failed";
        return ImportExportCodes::InternalError;
    }

    if (JXL_DEC_SUCCESS != JxlDecoderSetParallelRunner(dec.get(), JxlResizableParallelRunner, runner.get())) {
        errFile << "JxlDecoderSetParallelRunner failed";
        return ImportExportCodes::InternalError;
    }

    const auto data = io->readAll();

    const auto validation =
        JxlSignatureCheck(reinterpret_cast<const uint8_t *>(data.constData()), static_cast<size_t>(data.size()));

    switch (validation) {
    case JXL_SIG_NOT_ENOUGH_BYTES:
        errFile << "Failed magic byte validation, not enough data";
        return ImportExportCodes::FileFormatIncorrect;
    case JXL_SIG_INVALID:
        errFile << "Failed magic byte validation, incorrect format";
        return ImportExportCodes::FileFormatIncorrect;
    default:
        break;
    }

    if (JXL_DEC_SUCCESS
        != JxlDecoderSetInput(dec.get(),
                              reinterpret_cast<const uint8_t *>(data.constData()),
                              static_cast<size_t>(data.size()))) {
        errFile << "JxlDecoderSetInput failed";
        return ImportExportCodes::InternalError;
    };
    JxlDecoderCloseInput(dec.get());
    if (JXL_DEC_SUCCESS != JxlDecoderSetDecompressBoxes(dec.get(), JXL_TRUE)) {
        errFile << "JxlDecoderSetDecompressBoxes failed";
        return ImportExportCodes::InternalError;
    };

    KisImageSP image{nullptr};
    KisLayerSP layer{nullptr};
    std::array<char, 5> boxType{};
    QByteArray box(16384, 0x0);
    auto boxSize = box.size();

    for (;;) {
        JxlDecoderStatus status = JxlDecoderProcessInput(dec.get());

        if (status == JXL_DEC_ERROR) {
            errFile << "Decoder error";
            return ImportExportCodes::InternalError;
        } else if (status == JXL_DEC_NEED_MORE_INPUT) {
            errFile << "Error, already provided all input";
            return ImportExportCodes::InternalError;
        } else if (status == JXL_DEC_BASIC_INFO) {
            if (JXL_DEC_SUCCESS != JxlDecoderGetBasicInfo(dec.get(), &d.m_info)) {
                errFile << "JxlDecoderGetBasicInfo failed";
                return ImportExportCodes::ErrorWhileReading;
            }
            dbgFile << "Info";
            dbgFile << "Size:" << d.m_info.xsize << "x" << d.m_info.ysize;
            dbgFile << "Depth:" << d.m_info.bits_per_sample << d.m_info.exponent_bits_per_sample;
            dbgFile << "Number of channels:" << d.m_info.num_color_channels;
            dbgFile << "Has alpha" << d.m_info.num_extra_channels << d.m_info.alpha_bits
                    << d.m_info.alpha_exponent_bits;
            dbgFile << "Has animation:" << d.m_info.have_animation << "loops:" << d.m_info.animation.num_loops
                    << "tick:" << d.m_info.animation.tps_numerator << d.m_info.animation.tps_denominator;
            JxlResizableParallelRunnerSetThreads(
                runner.get(),
                JxlResizableParallelRunnerSuggestThreads(d.m_info.xsize, d.m_info.ysize));

            // XXX: libjxl does not yet provide a way to retrieve the real bit depth.
            // See
            // https://github.com/libjxl/libjxl/blame/35ca355660a89819e52013fa201284cb50768f80/lib/jxl/decode.cc#L568
            if (d.m_info.exponent_bits_per_sample != 0) {
                // Let's not rely on float16
                d.m_pixelFormat.data_type = JXL_TYPE_FLOAT;
                d.m_depthID = Float32BitsColorDepthID;
            } else {
                if (d.m_info.bits_per_sample == 8) {
                    d.m_pixelFormat.data_type = JXL_TYPE_UINT8;
                    d.m_depthID = Integer8BitsColorDepthID;
                } else {
                    d.m_pixelFormat.data_type = JXL_TYPE_UINT16;
                    d.m_depthID = Integer16BitsColorDepthID;
                }
            }

            if (d.m_info.num_color_channels == 1) {
                // Grayscale
                d.m_pixelFormat.num_channels = 2;
                d.m_colorID = GrayAColorModelID;
            } else if (d.m_info.num_color_channels == 3) {
                // RGBA
                d.m_pixelFormat.num_channels = 4;
                d.m_colorID = RGBAColorModelID;
            } else if (d.m_info.num_color_channels == 4) {
                // CMYKA
                d.m_pixelFormat.num_channels = 5;
                d.m_colorID = CMYKAColorModelID;
            } else {
                warnFile << "Forcing a RGBA conversion, unknown color space";
                d.m_pixelFormat.num_channels = 4;
                d.m_colorID = RGBAColorModelID;
                d.m_forcedConversion = true;
            }
        } else if (status == JXL_DEC_COLOR_ENCODING) {
            // Get the ICC color profile of the pixel data
            size_t icc_size{};
            QByteArray icc_profile;
            const KoColorSpace *cs{nullptr};
            const auto tgt = d.m_forcedConversion ? JXL_COLOR_PROFILE_TARGET_DATA : JXL_COLOR_PROFILE_TARGET_ORIGINAL;
            if (JXL_DEC_SUCCESS == JxlDecoderGetICCProfileSize(dec.get(), &d.m_pixelFormat, tgt, &icc_size)) {
                dbgFile << "JxlDecoderGetICCProfileSize succeeded, ICC profile available";
                icc_profile.resize(static_cast<int>(icc_size));
                if (JXL_DEC_SUCCESS
                    != JxlDecoderGetColorAsICCProfile(dec.get(),
                                                      &d.m_pixelFormat,
                                                      tgt,
                                                      reinterpret_cast<uint8_t *>(icc_profile.data()),
                                                      static_cast<size_t>(icc_profile.size()))) {
                    document->setErrorMessage(i18nc("JPEG-XL errors", "Unable to read the image profile."));
                    return ImportExportCodes::ErrorWhileReading;
                }

                // With the profile in hand, now we can create the image.
                const auto *profile = KoColorSpaceRegistry::instance()->createColorProfile(d.m_colorID.id(),
                                                                                           d.m_depthID.id(),
                                                                                           icc_profile);
                cs = KoColorSpaceRegistry::instance()->colorSpace(d.m_colorID.id(), d.m_depthID.id(), profile);
            } else {
                // XXX: Need to either create the LCMS profile manually
                // here or inject it into createColorProfile
                document->setErrorMessage(i18nc("JPEG-XL errors", "JPEG-XL encoded profile not implemented"));
                return ImportExportCodes::FormatFeaturesUnsupported;
            }

            image = new KisImage(document->createUndoStore(),
                                 static_cast<int>(d.m_info.xsize),
                                 static_cast<int>(d.m_info.ysize),
                                 cs,
                                 "JPEG-XL image");

            layer = new KisPaintLayer(image, image->nextLayerName(), UCHAR_MAX);
        } else if (status == JXL_DEC_NEED_IMAGE_OUT_BUFFER) {
            d.m = this;
            d.m_currentFrame = new KisPaintDevice(image->colorSpace());
            const auto callback = [&]() {
                if (d.m_colorID == RGBAColorModelID && d.m_depthID == Integer8BitsColorDepthID) {
                    return &::imageOutRgbCallback<KoBgrU8Traits>;
                } else if (d.m_colorID == RGBAColorModelID && d.m_depthID == Integer16BitsColorDepthID) {
                    return &::imageOutRgbCallback<KoBgrU16Traits>;
                } else if (d.m_pixelFormat.data_type == JXL_TYPE_UINT8) {
                    return &::imageOutSizedCallback<uint8_t>;
                } else if (d.m_pixelFormat.data_type == JXL_TYPE_UINT16) {
                    return &::imageOutSizedCallback<uint16_t>;
                } else {
                    return &::imageOutSizedCallback<float>;
                }
            }();

            if (JXL_DEC_SUCCESS != JxlDecoderSetImageOutCallback(dec.get(), &d.m_pixelFormat, callback, &d)) {
                errFile << "JxlDecoderSetImageOutBuffer failed";
                return ImportExportCodes::InternalError;
            }
        } else if (status == JXL_DEC_FRAME) {
            if (d.m_info.have_animation) {
                if (JXL_DEC_SUCCESS != JxlDecoderGetFrameHeader(dec.get(), &d.m_header)) {
                    document->setErrorMessage(i18nc("JPEG-XL errors", "JPEG-XL image is animated, but cannot retrieve animation frame header."));
                    return ImportExportCodes::ErrorWhileReading;
                }
            }
        } else if (status == JXL_DEC_FULL_IMAGE) {
            if (d.m_info.have_animation) {
                dbgFile << "Importing frame @" << d.m_nextFrameTime
                        << d.m_header.duration;

                if (d.m_nextFrameTime == 0) {
                    dbgFile << "Animation detected, ticks per second:"
                            << d.m_info.animation.tps_numerator
                            << d.m_info.animation.tps_denominator;
                    // XXX: How many ticks per second (FPS)?
                    // If > 240, block the derivation-- it's a stock JXL and
                    // Krita only supports up to 240 FPS.
                    // We'll try to derive the framerate from the first frame
                    // instead.
                    int framerate =
                        std::lround(d.m_info.animation.tps_numerator
                                    / static_cast<double>(
                                        d.m_info.animation.tps_denominator));
                    if (framerate > 240) {
                        warnFile << "JXL ticks per second value exceeds 240, "
                                    "approximating FPS from the duration of "
                                    "the first frame";
                        document->setWarningMessage(
                            i18nc("JPEG-XL errors",
                                  "The animation declares a frame rate of more "
                                  "than 240 FPS."));
                        const int approximatedFramerate = std::lround(
                            1000.0 / static_cast<double>(d.m_header.duration));
                        d.m_durationFrameInTicks =
                            static_cast<int>(d.m_header.duration);
                        framerate = std::max(approximatedFramerate, 1);
                    } else {
                        d.m_durationFrameInTicks = 1;
                    }
                    dbgFile << "Framerate:" << framerate;
                    layer->enableAnimation();
                    image->animationInterface()->setFullClipRangeStartTime(0);
                    image->animationInterface()->setFramerate(framerate);
                }

                const int currentFrameTime = std::lround(
                    static_cast<double>(d.m_nextFrameTime)
                    / static_cast<double>(d.m_durationFrameInTicks));

                auto *channel = layer->getKeyframeChannel(KisKeyframeChannel::Raster.id(), true);
                auto *frame = dynamic_cast<KisRasterKeyframeChannel *>(channel);
                image->animationInterface()->setFullClipRangeEndTime(
                    std::lround(static_cast<double>(d.m_nextFrameTime
                                                    + d.m_header.duration)
                                / static_cast<double>(d.m_durationFrameInTicks))
                    - 1);
                frame->importFrame(currentFrameTime, d.m_currentFrame, nullptr);
                d.m_nextFrameTime += static_cast<int>(d.m_header.duration);
            } else {
                layer->paintDevice()->makeCloneFrom(d.m_currentFrame, image->bounds());
            }
        } else if (status == JXL_DEC_SUCCESS || status == JXL_DEC_BOX) {
            if (!boxType.empty()) {
                // Release buffer and get its final size.
                const auto availOut = JxlDecoderReleaseBoxBuffer(dec.get());
                box.resize(box.size() - static_cast<int>(availOut));

                QBuffer buf(&box);
                if (boxType == exifTag) {
                    dbgFile << "Loading EXIF data. Size: " << box.size();

                    const auto *backend = KisMetadataBackendRegistry::instance()->value("exif");

                    backend->loadFrom(layer->metaData(), &buf);
                } else if (boxType == xmpTag) {
                    dbgFile << "Loading XMP or IPTC data. Size: " << box.size();

                    const auto *xmpBackend = KisMetadataBackendRegistry::instance()->value("xmp");
                    const auto *iptcBackend = KisMetadataBackendRegistry::instance()->value("iptc");

                    if (!xmpBackend->loadFrom(layer->metaData(), &buf)) {
                        iptcBackend->loadFrom(layer->metaData(), &buf);
                    }
                }
            }
            if (status == JXL_DEC_SUCCESS) {
                // All decoding successfully finished.
                // It's not required to call JxlDecoderReleaseInput(dec.get()) here since
                // the decoder will be destroyed.
                image->addNode(layer, image->rootLayer().data());
                document->setCurrentImage(image);
                return ImportExportCodes::OK;
            } else {
                if (JxlDecoderGetBoxType(dec.get(), boxType.data(), JXL_TRUE) != JXL_DEC_SUCCESS) {
                    errFile << "JxlDecoderGetBoxType failed";
                    return ImportExportCodes::ErrorWhileReading;
                }
                if (boxType == exifTag || boxType == xmpTag) {
                    if (JxlDecoderSetBoxBuffer(dec.get(),
                                               reinterpret_cast<uint8_t *>(boxType.data()),
                                               static_cast<size_t>(box.size()))
                        != JXL_DEC_SUCCESS) {
                        errFile << "JxlDecoderSetBoxBuffer failed";
                        return ImportExportCodes::InternalError;
                    }
                } else {
                    dbgFile << "Skipping box" << boxType.data();
                }
            }
        } else if (status == JXL_DEC_BOX_NEED_MORE_OUTPUT) {
            box.resize(boxSize * 2);
            if (JxlDecoderSetBoxBuffer(dec.get(),
                                       reinterpret_cast<uint8_t *>(boxType.data() + boxSize),
                                       static_cast<size_t>(box.size() - boxSize))
                != JXL_DEC_SUCCESS) {
                errFile << "JxlDecoderGetBoxType failed";
                return ImportExportCodes::ErrorWhileReading;
            }
        } else {
            errFile << "Unknown decoder status" << status;
            return ImportExportCodes::InternalError;
        }
    }

    return ImportExportCodes::OK;
}

#include <JPEGXLImport.moc>
