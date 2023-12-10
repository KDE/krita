/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>
#include <webp/encode.h>
#include <webp/mux.h>
#include <webp/mux_types.h>

#include <QBuffer>

#include <cmath>
#include <memory>

#include <KisDocument.h>
#include <KisExportCheckRegistry.h>
#include <KisImportExportErrorCode.h>
#include <KisImportExportManager.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <kis_debug.h>
#include <kis_exif_info_visitor.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_layer_utils.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_random_accessor_ng.h>
#include <kis_raster_keyframe_channel.h>
#include <kis_time_span.h>

#include "kis_wdg_options_webp.h"
#include "kis_webp_export.h"

K_PLUGIN_FACTORY_WITH_JSON(KisWebPExportFactory, "krita_webp_export.json", registerPlugin<KisWebPExport>();)

KisWebPExport::KisWebPExport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KisWebPExport::~KisWebPExport() = default;

KisPropertiesConfigurationSP KisWebPExport::defaultConfiguration(const QByteArray & /*from*/, const QByteArray & /*to*/) const
{
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    WebPConfig preset {};

    if (!WebPConfigInit(&preset)) {
        return cfg;
    }

    if (!WebPConfigLosslessPreset(&preset, 6)) {
        return cfg;
    }

    preset.thread_level = 1;

    if (!WebPValidateConfig(&preset)) {
        return cfg;
    }

    cfg->setProperty("haveAnimation", true);

    cfg->setProperty("preset", 0);
    cfg->setProperty("lossless", preset.lossless == 1);
    cfg->setProperty("quality", preset.quality);
    cfg->setProperty("method", preset.method);
    cfg->setProperty("dithering", true);

    cfg->setProperty("target_size", preset.target_size);
    cfg->setProperty("target_PSNR", preset.target_PSNR);
    cfg->setProperty("segments", preset.segments);
    cfg->setProperty("sns_strength", preset.sns_strength);
    cfg->setProperty("filter_strength", preset.filter_strength);
    cfg->setProperty("filter_sharpness", preset.filter_sharpness);
    cfg->setProperty("filter_type", preset.filter_type);
    cfg->setProperty("autofilter", preset.autofilter == 1);
    cfg->setProperty("alpha_compression", preset.alpha_compression);
    cfg->setProperty("alpha_filtering", preset.alpha_filtering);
    cfg->setProperty("alpha_quality", preset.alpha_quality);
    cfg->setProperty("pass", preset.pass);
    cfg->setProperty("show_compressed", preset.show_compressed == 1);
    cfg->setProperty("preprocessing", preset.preprocessing);
    cfg->setProperty("partitions", preset.partitions);
    cfg->setProperty("partition_limit", preset.partition_limit);
    cfg->setProperty("emulate_jpeg_size", preset.emulate_jpeg_size == 1);
    cfg->setProperty("thread_level", preset.thread_level > 0);
    cfg->setProperty("low_memory", preset.low_memory == 1);
    cfg->setProperty("near_lossless", preset.near_lossless);
    cfg->setProperty("exact", preset.exact == 1);
    cfg->setProperty("use_sharp_yuv", preset.use_sharp_yuv == 1);
#if WEBP_ENCODER_ABI_VERSION >= 0x020f
    cfg->setProperty("qmin", preset.qmin);
    cfg->setProperty("qmax", preset.qmax);
#endif

    cfg->setProperty("exif", true);
    cfg->setProperty("xmp", true);
    cfg->setProperty("iptc", true);
    cfg->setProperty("storeMetaData", false);
    cfg->setProperty("filters", "");

    return cfg;
}

KisConfigWidget *KisWebPExport::createConfigurationWidget(QWidget *parent, const QByteArray & /*from*/, const QByteArray & /*to*/) const
{
    return new KisWdgOptionsWebP(parent);
}

// RAII a WebPPicture.
struct WebPPictureSP {
    WebPPictureSP()
        : picture()
    {
    }

    WebPPicture *get()
    {
        return &picture;
    }

    ~WebPPictureSP()
    {
        WebPPictureFree(&picture);
    }

    WebPPicture picture{};
};

KisImportExportErrorCode KisWebPExport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP cfg)
{
    using WebPMuxSP = std::unique_ptr<WebPMux, decltype(&WebPMuxDelete)>;
    using WebPAnimEncoderSP =
        std::unique_ptr<WebPAnimEncoder, decltype(&WebPAnimEncoderDelete)>;

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(io->isWritable(),
                                         ImportExportCodes::NoAccessToWrite);

    KisImageSP image = document->savingImage();
    const QRect bounds = document->savingImage()->bounds();
    const KoColorSpace *cs =
        document->savingImage()->projection()->colorSpace();

    // Then comes the animation chunk.
    WebPData imageChunk = {nullptr, 0};

    {
        WebPAnimEncoderOptions encodingOptions;
        if (!WebPAnimEncoderOptionsInit(&encodingOptions)) {
            errFile << "WebP animation configuration initialization failure";
            return ImportExportCodes::InternalError;
        }

        if (cfg->getBool("lossless", true)) {
            encodingOptions.allow_mixed = false;
        } else {
            encodingOptions.allow_mixed = true;
        }
        encodingOptions.verbose = true;
        // XXX: should we implement background selection as in JPEG?
        encodingOptions.anim_params.loop_count = 0;

        WebPAnimEncoderSP enc(WebPAnimEncoderNew(bounds.width(),
                                                 bounds.height(),
                                                 &encodingOptions),
                              &WebPAnimEncoderDelete);

        WebPConfig config;
        {
            if (!WebPConfigInit(&config)) {
                errFile << "WebP config initialization failed!";
                return ImportExportCodes::InternalError;
            }

            config.lossless = cfg->getBool("lossless", true) ? 1 : 0;
            config.quality = cfg->getFloat("quality", 75.0);
            config.method = cfg->getInt("method", 4);

            config.target_size = cfg->getInt("target_size", 0);
            config.target_PSNR = cfg->getFloat("target_PSNR", 0.0f);
            config.segments = cfg->getInt("segments", 4);
            config.sns_strength = cfg->getInt("sns_strength", 50);
            config.filter_strength = cfg->getInt("filter_strength", 60);
            config.filter_sharpness = cfg->getInt("filter_sharpness", 0);
            config.filter_type = cfg->getInt("filter_type", 1);
            config.autofilter = cfg->getBool("autofilter", false) ? 1 : 0;
            config.alpha_compression = cfg->getInt("alpha_compression", 1);
            config.alpha_filtering = cfg->getInt("alpha_filtering", 1);
            config.alpha_quality = cfg->getInt("alpha_quality", 100);
            config.pass = cfg->getInt("pass", 1);
            config.show_compressed =
                cfg->getBool("show_compressed", false) ? 1 : 0;
            config.preprocessing = cfg->getInt("preprocessing", 0);
            config.partitions = cfg->getInt("partitions", 0);
            config.partition_limit = cfg->getInt("partition_limit", 0);
            config.emulate_jpeg_size =
                cfg->getBool("emulate_jpeg_size", false) ? 1 : 0;
            config.thread_level = cfg->getBool("thread_level", false) ? 1 : 0;
            config.low_memory = cfg->getBool("low_memory", false) ? 1 : 0;
            config.near_lossless = cfg->getInt("near_lossless", 100);
            config.exact = cfg->getBool("exact", false) ? 1 : 0;
            config.use_sharp_yuv = cfg->getBool("use_sharp_yuv", false) ? 1 : 0;
#if WEBP_ENCODER_ABI_VERSION >= 0x020f
            config.qmin = cfg->getInt("qmin", 0);
            config.qmax = cfg->getInt("qmax", 100);
#endif

            if (!WebPValidateConfig(&config)) {
                errFile << "WebP configuration validation failure";
                return ImportExportCodes::InternalError;
            }
        }

        const bool enableDithering = cfg->getBool("dithering", true);

        const bool isAnimated = [&]() {
            if (image->animationInterface()->hasAnimation() && cfg->getBool("haveAnimation", true)) {
                KisLayerUtils::flattenImage(image, nullptr);
                image->waitForDone();

                const KisNodeSP projection = image->rootLayer()->firstChild();
                return projection->isAnimated() && projection->hasEditablePaintDevice();
            }
            return false;
        }();

        if (isAnimated) {
            // Flatten the image, projections don't have keyframes.
            KisLayerUtils::flattenImage(image, nullptr);
            image->waitForDone();

            const KisNodeSP projection = image->rootLayer()->firstChild();
            if (!projection->isAnimated()) return ImportExportCodes::InternalError;

            const KisRasterKeyframeChannel *frames =
                projection->paintDevice()->keyframeChannel();
            const auto times = [&]() -> QList<int> {
                    QList<int> t;
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
                    QSet<int> s = frames->allKeyframeTimes();
                    t = QList<int>(s.begin(), s.end());
#else
                    t = frames->allKeyframeTimes().toList();
#endif
                std::sort(t.begin(), t.end());
                return t;
            }();

            // If this is not an integral number, it must be diagnosed on
            // export and reported to the user.
            // THE FRAME DURATION WILL BE ROUNDED.
            const int duration =
                std::lround(1000.0
                            / static_cast<double>(
                                image->animationInterface()->framerate()));

            for (const int i : times) {
                const int timestamp_ms = i * duration;

                WebPPictureSP currentFrame;
                if (!WebPPictureInit(currentFrame.get())) {
                    errFile << "WebP picture initialization failure";
                    return ImportExportCodes::InternalError;
                }

                currentFrame.get()->width = bounds.width();
                currentFrame.get()->height = bounds.height();

                // Use ARGB in lossless mode
                if (config.lossless == 1) {
                    currentFrame.get()->use_argb = 1;
                }

                const QImage pixels = [&]() {
                    const KisRasterKeyframeSP frameData =
                        frames->keyframeAt<KisRasterKeyframe>(i);
                    KisPaintDeviceSP dev = new KisPaintDevice(
                        *image->projection(),
                        KritaUtils::DeviceCopyMode::CopySnapshot);
                    frameData->writeFrameToDevice(dev);

                    KisPaintDeviceSP dst;
                    if ((cs->colorModelId() == RGBAColorModelID && cs->colorDepthId() == Integer8BitsColorDepthID)
                        || !enableDithering) {
                        dst = dev;
                    } else {
                        // We need to use gradient painter code's:
                        //    to convert to RGBA samedepth;
                        //    then dither to RGBA8
                        //    then convert from ARGB32 to RGBA8888
                        const KisPaintDeviceSP src = dev;
                        const KoID depthId = src->colorSpace()->colorDepthId();
                        const KoColorSpace *destCs = KoColorSpaceRegistry::instance()->rgb8();
                        const KoColorSpace *mixCs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                                 depthId.id(),
                                                                                                 destCs->profile());

                        KisPaintDeviceSP tmp = new KisPaintDevice(*src);
                        tmp->convertTo(mixCs);
                        dst = new KisPaintDevice(destCs);

                        const KisDitherOp *op =
                            mixCs->ditherOp(destCs->colorDepthId().id(), enableDithering ? DITHER_BEST : DITHER_NONE);

                        KisRandomConstAccessorSP srcIt = tmp->createRandomConstAccessorNG();
                        KisRandomAccessorSP dstIt = dst->createRandomAccessorNG();

                        int rows = 1;
                        int columns = 1;

                        for (int y = bounds.y(); y <= bounds.bottom(); y += rows) {
                            rows = qMin(srcIt->numContiguousRows(y),
                                        qMin(dstIt->numContiguousRows(y), bounds.bottom() - y + 1));

                            for (int x = bounds.x(); x <= bounds.right(); x += columns) {
                                columns = qMin(srcIt->numContiguousColumns(x),
                                               qMin(dstIt->numContiguousColumns(x), bounds.right() - x + 1));

                                srcIt->moveTo(x, y);
                                dstIt->moveTo(x, y);

                                const qint32 srcRowStride = srcIt->rowStride(x, y);
                                const qint32 dstRowStride = dstIt->rowStride(x, y);
                                const quint8 *srcPtr = srcIt->rawDataConst();
                                quint8 *dstPtr = dstIt->rawData();

                                op->dither(srcPtr, srcRowStride, dstPtr, dstRowStride, x, y, columns, rows);
                            }
                        }
                    }

                    // Convert to sRGB for non-RGBA color model
                    const KoColorProfile *imageProfile = (dst->colorSpace()->colorModelId() == RGBAColorModelID)
                        ? dev->colorSpace()->profile()
                        : nullptr;

                    const QImage imageOut = dst->convertToQImage(imageProfile, 0, 0, bounds.width(), bounds.height())
                                                .convertToFormat(QImage::Format_RGBA8888);

                    return imageOut;
                }();

                if (!WebPPictureImportRGBA(currentFrame.get(),
                                           pixels.constBits(),
                                           bounds.width() * 4)) {
                    errFile << "WebP picture conversion failure:"
                            << currentFrame.get()->error_code;
                    return ImportExportCodes::InternalError;
                }

                WebPMemoryWriter writer;
                WebPMemoryWriterInit(&writer);
                currentFrame.get()->writer = WebPMemoryWrite;
                currentFrame.get()->custom_ptr = &writer;

                if (!WebPEncode(&config, currentFrame.get())) {
                    errFile << "WebP encoding failure:"
                            << currentFrame.get()->error_code;
                    return ImportExportCodes::ErrorWhileWriting;
                }

                if (!WebPAnimEncoderAdd(enc.get(),
                                        currentFrame.get(),
                                        timestamp_ms,
                                        &config)) {
                    errFile << "WebPAnimEncoderAdd failed";
                    return ImportExportCodes::InternalError;
                }

                dbgFile << "Added frame @" << i << timestamp_ms << "ms";
            }

            const int timestamp_ms =
                (image->animationInterface()->documentPlaybackRange().end() + 1)
                * (1000 / image->animationInterface()->framerate());

            // Insert the finish beacon.
            WebPAnimEncoderAdd(enc.get(), nullptr, timestamp_ms, nullptr);

            dbgFile << "Animation finished @" << timestamp_ms << "ms";
        } else {
            WebPPictureSP currentFrame;
            if (!WebPPictureInit(currentFrame.get())) {
                errFile << "WebP picture initialization failure";
                return ImportExportCodes::InternalError;
            }

            currentFrame.get()->width = bounds.width();
            currentFrame.get()->height = bounds.height();

            // Use ARGB in lossless mode
            if (config.lossless == 1) {
                currentFrame.get()->use_argb = 1;
            }

            // Insert the projection itself only
            const QImage pixels = [&]() {
                KisPaintDeviceSP dst;
                if ((cs->colorModelId() == RGBAColorModelID && cs->colorDepthId() == Integer8BitsColorDepthID)
                    || !enableDithering) {
                    dst = document->savingImage()->projection();
                } else {
                    // We need to use gradient painter code's:
                    //    to convert to RGBA samedepth;
                    //    then dither to RGBA8
                    //    then convert from ARGB32 to RGBA8888
                    const KisPaintDeviceSP src = document->savingImage()->projection();
                    const KoID depthId = src->colorSpace()->colorDepthId();
                    const KoColorSpace *destCs = KoColorSpaceRegistry::instance()->rgb8();
                    const KoColorSpace *mixCs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                             depthId.id(),
                                                                                             destCs->profile());

                    KisPaintDeviceSP tmp = new KisPaintDevice(*src);
                    tmp->convertTo(mixCs);
                    dst = new KisPaintDevice(destCs);

                    const KisDitherOp *op =
                        mixCs->ditherOp(destCs->colorDepthId().id(), enableDithering ? DITHER_BEST : DITHER_NONE);

                    KisRandomConstAccessorSP srcIt = tmp->createRandomConstAccessorNG();
                    KisRandomAccessorSP dstIt = dst->createRandomAccessorNG();

                    int rows = 1;
                    int columns = 1;

                    for (int y = bounds.y(); y <= bounds.bottom(); y += rows) {
                        rows = qMin(srcIt->numContiguousRows(y),
                                    qMin(dstIt->numContiguousRows(y), bounds.bottom() - y + 1));

                        for (int x = bounds.x(); x <= bounds.right(); x += columns) {
                            columns = qMin(srcIt->numContiguousColumns(x),
                                           qMin(dstIt->numContiguousColumns(x), bounds.right() - x + 1));

                            srcIt->moveTo(x, y);
                            dstIt->moveTo(x, y);

                            const qint32 srcRowStride = srcIt->rowStride(x, y);
                            const qint32 dstRowStride = dstIt->rowStride(x, y);
                            const quint8 *srcPtr = srcIt->rawDataConst();
                            quint8 *dstPtr = dstIt->rawData();

                            op->dither(srcPtr, srcRowStride, dstPtr, dstRowStride, x, y, columns, rows);
                        }
                    }
                }

                // Convert to sRGB for non-RGBA color model
                const KoColorProfile *imageProfile = (dst->colorSpace()->colorModelId() == RGBAColorModelID)
                    ? document->savingImage()->projection()->colorSpace()->profile()
                    : nullptr;

                const QImage imageOut = dst->convertToQImage(imageProfile, 0, 0, bounds.width(), bounds.height())
                                            .convertToFormat(QImage::Format_RGBA8888);

                return imageOut;
            }();

            if (!WebPPictureImportRGBA(currentFrame.get(),
                                       pixels.constBits(),
                                       bounds.width() * 4)) {
                errFile << "WebP picture conversion failure:"
                        << currentFrame.get()->error_code;
                return ImportExportCodes::InternalError;
            }

            WebPMemoryWriter writer;
            WebPMemoryWriterInit(&writer);
            currentFrame.get()->writer = WebPMemoryWrite;
            currentFrame.get()->custom_ptr = &writer;

            if (!WebPEncode(&config, currentFrame.get())) {
                errFile << "WebP encoding failure:"
                        << currentFrame.get()->error_code;
                return ImportExportCodes::ErrorWhileWriting;
            }

            if (!WebPAnimEncoderAdd(enc.get(),
                                    currentFrame.get(),
                                    0,
                                    &config)) {
                errFile << "WebPAnimEncoderAdd failed";
                return ImportExportCodes::InternalError;
            }
        }

        WebPAnimEncoderAssemble(enc.get(), &imageChunk);
    };

    // Don't copy this data, it's the biggest chunk.
    WebPMuxSP mux(WebPMuxCreate(&imageChunk, 0), &WebPMuxDelete);

    if (!mux) {
        errFile << "WebP mux initialization failure";
        return ImportExportCodes::InternalError;
    }

    // According to the standard, the ICC profile must be written first.
    {
        const QByteArray profile = image->profile()->rawData();

        WebPData iccChunk = {reinterpret_cast<const uint8_t *>(profile.data()),
                             static_cast<size_t>(profile.size())};

        // This data will die at the end of the scope.
        if (WEBP_MUX_OK != WebPMuxSetChunk(mux.get(), "ICCP", &iccChunk, 1)) {
            errFile << "WebPMuxSetChunk for the ICC profile failed";
            return ImportExportCodes::InternalError;
        }
    }

    if (cfg->getBool("storeMetaData", false)) {
        auto metaDataStore = [&]() -> std::unique_ptr<KisMetaData::Store> {
            KisExifInfoVisitor exivInfoVisitor;
            exivInfoVisitor.visit(image->rootLayer().data());
            if (exivInfoVisitor.metaDataCount() == 1) {
                return std::make_unique<KisMetaData::Store>(
                    *exivInfoVisitor.exifInfo());
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
            const KisMetaData::IOBackend *io =
                KisMetadataBackendRegistry::instance()->value("exif");

            QBuffer ioDevice;

            // Inject the data as any other IOBackend
            io->saveTo(metaDataStore.get(), &ioDevice);

            WebPData xmp = {
                reinterpret_cast<const uint8_t *>(ioDevice.data().constData()),
                static_cast<size_t>(ioDevice.data().size())};

            // This data will die at the end of the scope.
            if (WEBP_MUX_OK != WebPMuxSetChunk(mux.get(), "EXIF", &xmp, 1)) {
                errFile << "WebPMuxSetChunk for EXIF failed";
                return ImportExportCodes::InternalError;
            }
        }

        if (metaDataStore && cfg->getBool("xmp", true)) {
            const KisMetaData::IOBackend *io =
                KisMetadataBackendRegistry::instance()->value("xmp");

            QBuffer ioDevice;

            // Inject the data as any other IOBackend
            io->saveTo(metaDataStore.get(), &ioDevice);

            WebPData xmp = {
                reinterpret_cast<const uint8_t *>(ioDevice.data().constData()),
                static_cast<size_t>(ioDevice.data().size())};

            // This data will die at the end of the scope.
            if (WEBP_MUX_OK != WebPMuxSetChunk(mux.get(), "XMP ", &xmp, 1)) {
                errFile << "WebPMuxSetChunk for XMP failed";
                return ImportExportCodes::InternalError;
            }
        }
    }

    WebPData output;
    WebPMuxAssemble(mux.get(), &output);
    QDataStream s(io);
    s.setByteOrder(QDataStream::LittleEndian);
    s.writeRawData(reinterpret_cast<const char *>(output.bytes),
                   static_cast<int>(output.size));
    WebPDataClear(&output);

    return ImportExportCodes::OK;
}

void KisWebPExport::initializeCapabilities()
{
    addCapability(KisExportCheckRegistry::instance()
                      ->get("AnimationCheck")
                      ->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()
                      ->get("IntegralFrameDurationCheck")
                      ->create(KisExportCheckBase::PARTIALLY));
    addCapability(KisExportCheckRegistry::instance()
                      ->get("sRGBProfileCheck")
                      ->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()
                      ->get("ExifCheck")
                      ->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()
                      ->get("MultiLayerCheck")
                      ->create(KisExportCheckBase::PARTIALLY));
    addCapability(KisExportCheckRegistry::instance()
                      ->get("TiffExifCheck")
                      ->create(KisExportCheckBase::PARTIALLY));
    // XXX: add check for IPTC metadata and mark as UNSUPPORTED by the standard.
    QList<QPair<KoID, KoID>> supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>() << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "WebP");
}

#include "kis_webp_export.moc"
