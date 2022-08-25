/*
 *  SPDX-FileCopyrightText: 2021 the JPEG XL Project Authors
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "JPEGXLExport.h"

#include <jxl/encode_cxx.h>
#include <jxl/resizable_parallel_runner_cxx.h>
#include <kpluginfactory.h>

#include <QBuffer>
#include <cstdint>

#include <KisDocument.h>
#include <KisExportCheckRegistry.h>
#include <KisImportExportErrorCode.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoConfig.h>
#include <kis_assert.h>
#include <kis_debug.h>
#include <kis_exif_info_visitor.h>
#include <kis_image_animation_interface.h>
#include <kis_layer.h>
#include <kis_layer_utils.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_raster_keyframe_channel.h>
#include <kis_time_span.h>

#include "kis_wdg_options_jpegxl.h"

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_jxl_export.json", registerPlugin<JPEGXLExport>();)

template<class Traits>
inline void swap(char *dstPtr, size_t numPixels)
{
    using Pixel = typename Traits::Pixel;

    auto *pixelPtr = reinterpret_cast<Pixel *>(dstPtr);

    for (size_t i = 0; i < numPixels; i++) {
        std::swap(pixelPtr->blue, pixelPtr->red);
        pixelPtr += 1;
    }
}

inline void swapRgb(const KoColorSpace *cs, QByteArray &pixels)
{
    KIS_ASSERT(cs->colorModelId() == RGBAColorModelID);
    KIS_ASSERT(cs->colorDepthId() == Integer8BitsColorDepthID || cs->colorDepthId() == Integer16BitsColorDepthID);

    const auto numPixels = static_cast<size_t>(pixels.size()) / cs->pixelSize();

    auto *currentPixel = pixels.data();

    if (cs->colorDepthId() == Integer8BitsColorDepthID) {
        swap<KoBgrU8Traits>(currentPixel, numPixels);
    } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
        swap<KoBgrU16Traits>(currentPixel, numPixels);
    }
}

JPEGXLExport::JPEGXLExport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KisImportExportErrorCode JPEGXLExport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP cfg)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(io->isWritable(), ImportExportCodes::NoAccessToWrite);

    KisImageSP image = document->savingImage();
    const auto bounds = image->bounds();
    const auto *const cs = image->colorSpace();

    auto enc = JxlEncoderMake(nullptr);
    auto runner = JxlResizableParallelRunnerMake(nullptr);
    if (JXL_ENC_SUCCESS != JxlEncoderSetParallelRunner(enc.get(), JxlResizableParallelRunner, runner.get())) {
        errFile << "JxlEncoderSetParallelRunner failed";
        return ImportExportCodes::InternalError;
    }

    JxlResizableParallelRunnerSetThreads(runner.get(),
                                         JxlResizableParallelRunnerSuggestThreads(static_cast<uint64_t>(bounds.width()), static_cast<uint64_t>(bounds.height())));

    const auto pixelFormat = [&]() {
        JxlPixelFormat pixelFormat{};
        if (cs->colorDepthId() == Integer8BitsColorDepthID) {
            pixelFormat.data_type = JXL_TYPE_UINT8;
        } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
            pixelFormat.data_type = JXL_TYPE_UINT16;
#ifdef HAVE_OPENEXR
        } else if (cs->colorDepthId() == Float16BitsColorDepthID) {
            pixelFormat.data_type = JXL_TYPE_FLOAT16;
#endif
        } else if (cs->colorDepthId() == Float32BitsColorDepthID) {
            pixelFormat.data_type = JXL_TYPE_FLOAT;
        }
        if (cs->colorModelId() == RGBAColorModelID) {
            pixelFormat.num_channels = 4;
        } else if (cs->colorModelId() == GrayAColorModelID) {
            pixelFormat.num_channels = 2;
        }
        return pixelFormat;
    }();

    if (JXL_ENC_SUCCESS != JxlEncoderUseBoxes(enc.get())) {
        errFile << "JxlEncoderUseBoxes failed";
        return ImportExportCodes::InternalError;
    }

    const auto basicInfo = [&]() {
        auto info{std::make_unique<JxlBasicInfo>()};
        JxlEncoderInitBasicInfo(info.get());
        info->xsize = static_cast<uint32_t>(bounds.width());
        info->ysize = static_cast<uint32_t>(bounds.height());
        {
            if (cs->colorDepthId() == Integer8BitsColorDepthID) {
                info->bits_per_sample = 8;
                info->exponent_bits_per_sample = 0;
                info->alpha_bits = 8;
                info->alpha_exponent_bits = 0;
            } else if (cs->colorDepthId() == Integer16BitsColorDepthID) {
                info->bits_per_sample = 16;
                info->exponent_bits_per_sample = 0;
                info->alpha_bits = 16;
                info->alpha_exponent_bits = 0;
#ifdef HAVE_OPENEXR
            } else if (cs->colorDepthId() == Float16BitsColorDepthID) {
                info->bits_per_sample = 16;
                info->exponent_bits_per_sample = 5;
                info->alpha_bits = 16;
                info->alpha_exponent_bits = 5;
#endif
            } else if (cs->colorDepthId() == Float32BitsColorDepthID) {
                info->bits_per_sample = 32;
                info->exponent_bits_per_sample = 8;
                info->alpha_bits = 32;
                info->alpha_exponent_bits = 8;
            }
        }
        if (cs->colorModelId() == RGBAColorModelID) {
            info->num_color_channels = 3;
            info->num_extra_channels = 1;
        } else if (cs->colorModelId() == GrayAColorModelID) {
            info->num_color_channels = 1;
            info->num_extra_channels = 1;
        }
        info->uses_original_profile = JXL_TRUE;
        if (image->animationInterface()->hasAnimation() && cfg->getBool("haveAnimation", true)) {
            info->have_animation = JXL_TRUE;
            info->animation.have_timecodes = JXL_FALSE;
            info->animation.num_loops = 0;
            // Unlike WebP, JXL does allow for setting proper framerates.
            info->animation.tps_numerator =
                static_cast<uint32_t>(image->animationInterface()->framerate());
            info->animation.tps_denominator = 1;
        }
        return info;
    }();

    if (JXL_ENC_SUCCESS != JxlEncoderSetBasicInfo(enc.get(), basicInfo.get())) {
        errFile << "JxlEncoderSetBasicInfo failed";
        return ImportExportCodes::InternalError;
    }

    {
        const auto profile = image->profile()->rawData();

        if (JXL_ENC_SUCCESS
            != JxlEncoderSetICCProfile(enc.get(),
                                       reinterpret_cast<const uint8_t *>(profile.constData()),
                                       static_cast<size_t>(profile.size()))) {
            errFile << "JxlEncoderSetColorEncoding failed";
            return ImportExportCodes::InternalError;
        }
    }

    if (cfg->getBool("storeMetadata", false)) {
        auto metaDataStore = [&]() -> std::unique_ptr<KisMetaData::Store> {
            KisExifInfoVisitor exivInfoVisitor;
            exivInfoVisitor.visit(image->rootLayer().data());
            if (exivInfoVisitor.metaDataCount() == 1) {
                return std::make_unique<KisMetaData::Store>(*exivInfoVisitor.exifInfo());
            } else {
                return {};
            }
        }();

        if (metaDataStore && !metaDataStore->isEmpty()) {
            KisMetaData::FilterRegistryModel model;
            model.setEnabledFilters(cfg->getString("filters").split(","));
            metaDataStore->applyFilters(model.enabledFilters());
        }

        if (metaDataStore && cfg->getBool("exif", true)) {
            const KisMetaData::IOBackend *io = KisMetadataBackendRegistry::instance()->value("exif");

            QBuffer ioDevice;

            // Inject the data as any other IOBackend
            io->saveTo(metaDataStore.get(), &ioDevice);

            if (JXL_ENC_SUCCESS
                != JxlEncoderAddBox(enc.get(),
                                    "Exif",
                                    reinterpret_cast<const uint8_t *>(ioDevice.data().constData()),
                                    static_cast<size_t>(ioDevice.size()),
                                    JXL_FALSE)) {
                errFile << "JxlEncoderAddBox for EXIF failed";
                return ImportExportCodes::InternalError;
            }
        }

        if (metaDataStore && cfg->getBool("xmp", true)) {
            const KisMetaData::IOBackend *io = KisMetadataBackendRegistry::instance()->value("xmp");

            QBuffer ioDevice;

            // Inject the data as any other IOBackend
            io->saveTo(metaDataStore.get(), &ioDevice);

            if (JXL_ENC_SUCCESS
                != JxlEncoderAddBox(enc.get(),
                                    "xml ",
                                    reinterpret_cast<const uint8_t *>(ioDevice.data().constData()),
                                    static_cast<size_t>(ioDevice.size()),
                                    JXL_FALSE)) {
                errFile << "JxlEncoderAddBox for XMP failed";
                return ImportExportCodes::InternalError;
            }
        }

        if (metaDataStore && cfg->getBool("iptc", true)) {
            const KisMetaData::IOBackend *io = KisMetadataBackendRegistry::instance()->value("iptc");

            QBuffer ioDevice;

            // Inject the data as any other IOBackend
            io->saveTo(metaDataStore.get(), &ioDevice);

            if (JXL_ENC_SUCCESS
                != JxlEncoderAddBox(enc.get(),
                                    "xml ",
                                    reinterpret_cast<const uint8_t *>(ioDevice.data().constData()),
                                    static_cast<size_t>(ioDevice.size()),
                                    JXL_FALSE)) {
                errFile << "JxlEncoderAddBox for IPTC failed";
                return ImportExportCodes::InternalError;
            }
        }
    }

    auto *frameSettings = JxlEncoderFrameSettingsCreate(enc.get(), nullptr);
    {
        const auto setFrameLossless = [&](bool v) {
            if (JxlEncoderSetFrameLossless(frameSettings, v ? JXL_TRUE : JXL_FALSE) != JXL_ENC_SUCCESS) {
                errFile << "JxlEncoderSetFrameLossless failed";
                return false;
            }
            return true;
        };

        const auto setSetting = [&](JxlEncoderFrameSettingId id, int v) {
            // https://github.com/libjxl/libjxl/issues/1210
            if (id == JXL_ENC_FRAME_SETTING_RESAMPLING && v == -1)
                return true;
            if (JxlEncoderFrameSettingsSetOption(frameSettings, id, v) != JXL_ENC_SUCCESS) {
                errFile << "JxlEncoderFrameSettingsSetOption failed";
                return false;
            }
            return true;
        };

        if (!setFrameLossless(cfg->getBool("lossless"))
            || !setSetting(JXL_ENC_FRAME_SETTING_EFFORT, cfg->getInt("effort", 7))
            || !setSetting(JXL_ENC_FRAME_SETTING_DECODING_SPEED, cfg->getInt("decodingSpeed", 0))
            || !setSetting(JXL_ENC_FRAME_SETTING_RESAMPLING, cfg->getInt("resampling", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_EXTRA_CHANNEL_RESAMPLING, cfg->getInt("extraChannelResampling", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_DOTS, cfg->getInt("dots", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_PATCHES, cfg->getInt("patches", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_EPF, cfg->getInt("epf", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_GABORISH, cfg->getInt("gaborish", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_MODULAR, cfg->getInt("modular", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_KEEP_INVISIBLE, cfg->getInt("keepInvisible", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_GROUP_ORDER, cfg->getInt("groupOrder", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_RESPONSIVE, cfg->getInt("responsive", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_PROGRESSIVE_AC, cfg->getInt("progressiveAC", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_QPROGRESSIVE_AC, cfg->getInt("qProgressiveAC", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_PROGRESSIVE_DC, cfg->getInt("progressiveDC", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_PALETTE_COLORS, cfg->getInt("paletteColors", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_LOSSY_PALETTE, cfg->getInt("lossyPalette", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_MODULAR_GROUP_SIZE, cfg->getInt("modularGroupSize", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_MODULAR_PREDICTOR, cfg->getInt("modularPredictor", -1))
            || !setSetting(JXL_ENC_FRAME_SETTING_JPEG_RECON_CFL, cfg->getInt("jpegReconCFL", -1))) {
            return ImportExportCodes::InternalError;
        }
    }

    {
        const auto setSettingFloat = [&](JxlEncoderFrameSettingId id, float v) {
            if (JxlEncoderFrameSettingsSetFloatOption(frameSettings, id, v) != JXL_ENC_SUCCESS) {
                errFile << "JxlEncoderFrameSettingsSetFloatOption failed";
                return false;
            }
            return true;
        };

        if (!setSettingFloat(JXL_ENC_FRAME_SETTING_PHOTON_NOISE, cfg->getFloat("photonNoise", 0))
            || !setSettingFloat(JXL_ENC_FRAME_SETTING_CHANNEL_COLORS_GLOBAL_PERCENT,
                                cfg->getFloat("channelColorsGlobalPercent", -1))
            || !setSettingFloat(JXL_ENC_FRAME_SETTING_CHANNEL_COLORS_GROUP_PERCENT,
                                cfg->getFloat("channelColorsGroupPercent", -1))
            || !setSettingFloat(JXL_ENC_FRAME_SETTING_MODULAR_MA_TREE_LEARNING_PERCENT,
                                cfg->getFloat("modularMATreeLearningPercent", -1))) {
            return ImportExportCodes::InternalError;
        }
    }

    {
        if (image->animationInterface()->hasAnimation()
            && cfg->getBool("haveAnimation", true)) {
            // Flatten the image, projections don't have keyframes.
            KisLayerUtils::flattenImage(image, nullptr);
            image->waitForDone();

            const KisNodeSP projection = image->rootLayer()->firstChild();
            KIS_ASSERT(projection->isAnimated());
            KIS_ASSERT(projection->hasEditablePaintDevice());

            const auto *frames = projection->paintDevice()->keyframeChannel();
            const auto times = [&]() {
                auto t = frames->allKeyframeTimes().toList();
                std::sort(t.begin(), t.end());
                return t;
            }();

            auto frameHeader = []() {
                auto header = std::make_unique<JxlFrameHeader>();
                JxlEncoderInitFrameHeader(header.get());
                return header;
            }();

            for (const auto i : times) {
                frameHeader->duration = [&]() {
                    const auto nextKeyframe = frames->nextKeyframeTime(i);
                    if (nextKeyframe == -1) {
                        return static_cast<uint32_t>(
                            image->animationInterface()->fullClipRange().end()
                            - i + 1);
                    } else {
                        return static_cast<uint32_t>(frames->nextKeyframeTime(i) - i);
                    }
                }();
                frameHeader->is_last = 0;

                if (JxlEncoderSetFrameHeader(frameSettings, frameHeader.get()) != JXL_ENC_SUCCESS) {
                    errFile << "JxlEncoderSetFrameHeader failed";
                    return ImportExportCodes::InternalError;
                }

                QByteArray pixels{[&]() {
                    const auto frameData = frames->keyframeAt<KisRasterKeyframe>(i);
                    KisPaintDeviceSP dev =
                        new KisPaintDevice(*image->projection(), KritaUtils::DeviceCopyMode::CopySnapshot);
                    frameData->writeFrameToDevice(dev);
                    QByteArray p(static_cast<int>(cs->pixelSize()) * bounds.width() * bounds.height(), 0x0);
                    dev->readBytes(reinterpret_cast<quint8 *>(p.data()), image->bounds());
                    return p;
                }()};

                // BGRA -> RGBA
                if (cs->colorModelId() == RGBAColorModelID
                    && (cs->colorDepthId() == Integer8BitsColorDepthID
                        || cs->colorDepthId() == Integer16BitsColorDepthID)) {
                    swapRgb(cs, pixels);
                }

                if (JxlEncoderAddImageFrame(frameSettings,
                                            &pixelFormat,
                                            pixels.data(),
                                            static_cast<size_t>(pixels.size()))
                    != JXL_ENC_SUCCESS) {
                    errFile << "JxlEncoderAddImageFrame @" << i << "failed";
                    return ImportExportCodes::InternalError;
                }
            }
        } else {
            // Insert the projection itself only
            QByteArray pixels{[&]() {
                const auto bounds = image->bounds();
                QByteArray p(static_cast<int>(cs->pixelSize()) * bounds.width() * bounds.height(), 0x0);
                image->projection()->readBytes(reinterpret_cast<quint8 *>(pixels.data()), image->bounds());
                return p;
            }()};

            // BGRA -> RGBA
            if (cs->colorModelId() == RGBAColorModelID
                && (cs->colorDepthId() == Integer8BitsColorDepthID
                    || cs->colorDepthId() == Integer16BitsColorDepthID)) {
                swapRgb(cs, pixels);
            }

            if (JxlEncoderAddImageFrame(frameSettings, &pixelFormat, pixels.data(), static_cast<size_t>(pixels.size()))
                != JXL_ENC_SUCCESS) {
                errFile << "JxlEncoderAddImageFrame failed";
                return ImportExportCodes::InternalError;
            }
        }
        JxlEncoderCloseInput(enc.get());

        QByteArray compressed(16384, 0x0);
        auto *nextOut = reinterpret_cast<uint8_t *>(compressed.data());
        auto availOut = static_cast<size_t>(compressed.size());
        auto result = JXL_ENC_NEED_MORE_OUTPUT;
        while (result == JXL_ENC_NEED_MORE_OUTPUT) {
            result = JxlEncoderProcessOutput(enc.get(), &nextOut, &availOut);
            if (result != JXL_ENC_ERROR) {
                io->write(compressed.data(), compressed.size() - static_cast<int>(availOut));
            }
            if (result == JXL_ENC_NEED_MORE_OUTPUT) {
                compressed.resize(compressed.size() * 2);
                nextOut = reinterpret_cast<uint8_t *>(compressed.data());
                availOut = static_cast<size_t>(compressed.size());
            }
        }
        if (JXL_ENC_SUCCESS != result) {
            errFile << "JxlEncoderProcessOutput failed";
            return ImportExportCodes::ErrorWhileWriting;
        }
    }

    return ImportExportCodes::OK;
}

void JPEGXLExport::initializeCapabilities()
{
    // This checks before saving for what the file format supports: anything that is supported needs to be mentioned
    // here

    QList<QPair<KoID, KoID>> supportedColorModels;
    addCapability(KisExportCheckRegistry::instance()
                      ->get("AnimationCheck")
                      ->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("ExifCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::PARTIALLY));
    addCapability(KisExportCheckRegistry::instance()->get("TiffExifCheck")->create(KisExportCheckBase::PARTIALLY));
    supportedColorModels << QPair<KoID, KoID>() << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID)
                         << QPair<KoID, KoID>(GrayAColorModelID, Integer8BitsColorDepthID)
                         << QPair<KoID, KoID>(RGBAColorModelID, Integer16BitsColorDepthID)
                         << QPair<KoID, KoID>(GrayAColorModelID, Integer16BitsColorDepthID)
#ifdef HAVE_OPENEXR
                         << QPair<KoID, KoID>(RGBAColorModelID, Float16BitsColorDepthID)
                         << QPair<KoID, KoID>(GrayAColorModelID, Float16BitsColorDepthID)
#endif
                         << QPair<KoID, KoID>(RGBAColorModelID, Float32BitsColorDepthID)
                         << QPair<KoID, KoID>(GrayAColorModelID, Float32BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "JPEG-XL");
}

KisConfigWidget *
JPEGXLExport::createConfigurationWidget(QWidget *parent, const QByteArray & /*from*/, const QByteArray & /*to*/) const
{
    return new KisWdgOptionsJPEGXL(parent);
}

KisPropertiesConfigurationSP JPEGXLExport::defaultConfiguration(const QByteArray &, const QByteArray &) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();

    // WARNING: libjxl only allows setting encoding properties,
    // so I hardcoded values from https://libjxl.readthedocs.io/en/latest/api_encoder.html
    // https://readthedocs.org/projects/libjxl/builds/16271112/

    // Options for the following were not added because they rely
    // on the image's specific color space, or can introduce more
    // trouble than help:
    //   JXL_ENC_FRAME_SETTING_GROUP_ORDER_CENTER_X
    //   JXL_ENC_FRAME_SETTING_GROUP_ORDER_CENTER_Y
    //   JXL_ENC_FRAME_SETTING_MODULAR_COLOR_SPACE
    //   JXL_ENC_FRAME_SETTING_MODULAR_NB_PREV_CHANNELS
    // These are directly incompatible with the export logic:
    //   JXL_ENC_FRAME_SETTING_ALREADY_DOWNSAMPLED
    //   JXL_ENC_FRAME_SETTING_COLOR_TRANSFORM

    cfg->setProperty("haveAnimation", true);
    cfg->setProperty("lossless", true);
    cfg->setProperty("effort", 7);
    cfg->setProperty("decodingSpeed", 0);
    cfg->setProperty("resampling", -1);
    cfg->setProperty("extraChannelResampling", -1);
    cfg->setProperty("photonNoise", 0);
    cfg->setProperty("dots", -1);
    cfg->setProperty("patches", -1);
    cfg->setProperty("epf", -1);
    cfg->setProperty("gaborish", -1);
    cfg->setProperty("modular", -1);
    cfg->setProperty("keepInvisible", -1);
    cfg->setProperty("groupOrder", -1);
    cfg->setProperty("responsive", -1);
    cfg->setProperty("progressiveAC", -1);
    cfg->setProperty("qProgressiveAC", -1);
    cfg->setProperty("progressiveDC", -1);
    cfg->setProperty("channelColorsGlobalPercent", -1);
    cfg->setProperty("channelColorsGroupPercent", -1);
    cfg->setProperty("paletteColors", -1);
    cfg->setProperty("lossyPalette", -1);
    cfg->setProperty("modularGroupSize", -1);
    cfg->setProperty("modularPredictor", -1);
    cfg->setProperty("modularMATreeLearningPercent", -1);
    cfg->setProperty("jpegReconCFL", -1);

    cfg->setProperty("exif", true);
    cfg->setProperty("xmp", true);
    cfg->setProperty("iptc", true);
    cfg->setProperty("storeMetadata", false);
    cfg->setProperty("filters", "");
    return cfg;
}

#include <JPEGXLExport.moc>
