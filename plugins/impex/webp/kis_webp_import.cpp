/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>
#include <webp/demux.h>

#include <QBuffer>
#include <QByteArray>

#include <cmath>
#include <cstdint>
#include <memory>

#include <KisDocument.h>
#include <KisImportExportErrorCode.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoCompositeOpRegistry.h>
#include <KoDialog.h>
#include <kis_group_layer.h>
#include <kis_image_animation_interface.h>
#include <kis_keyframe_channel.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_properties_configuration.h>
#include <kis_raster_keyframe_channel.h>

#include "kis_webp_import.h"

K_PLUGIN_FACTORY_WITH_JSON(KisWebPImportFactory, "krita_webp_import.json", registerPlugin<KisWebPImport>();)

KisWebPImport::KisWebPImport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KisWebPImport::~KisWebPImport() = default;

KisImportExportErrorCode KisWebPImport::convert(KisDocument *document,
                                                QIODevice *io,
                                                KisPropertiesConfigurationSP)
{
    const QByteArray buf = io->readAll();

    if (buf.isEmpty()) {
        return ImportExportCodes::ErrorWhileReading;
    }

    const uint8_t *data = reinterpret_cast<const uint8_t *>(buf.constData());
    const size_t data_size = static_cast<size_t>(buf.size());

    const WebPData webpData = {data, data_size};

    WebPDemuxer *demux = WebPDemux(&webpData);
    if (!demux) {
        dbgFile << "WebP demuxer initialization failure";
        return ImportExportCodes::InternalError;
    }

    const uint32_t width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    const uint32_t height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);
    const uint32_t flags = WebPDemuxGetI(demux, WEBP_FF_FORMAT_FLAGS);
    const uint32_t bg = WebPDemuxGetI(demux, WEBP_FF_BACKGROUND_COLOR);

    const KoColorSpace *colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    const KoColorSpace *imageColorSpace = nullptr;

    bool isRgba = true;

    {
        WebPChunkIterator chunk_iter;
        if (flags & ICCP_FLAG) {
            if (WebPDemuxGetChunk(demux, "ICCP", 1, &chunk_iter)) {
                dbgFile << "WebPDemuxGetChunk on ICCP succeeded, ICC profile "
                           "available";

                const QByteArray iccProfile(
                    reinterpret_cast<const char *>(chunk_iter.chunk.bytes),
                    static_cast<int>(chunk_iter.chunk.size));
                const KoColorProfile *profile =
                    KoColorSpaceRegistry::instance()->createColorProfile(
                        RGBAColorModelID.id(),
                        Integer8BitsColorDepthID.id(),
                        iccProfile);
                imageColorSpace = KoColorSpaceRegistry::instance()->colorSpace(
                    RGBAColorModelID.id(),
                    Integer8BitsColorDepthID.id(),
                    profile);

                // Assign as non-RGBA color space to convert it back later
                if (!imageColorSpace) {
                    const QString colId = profile->colorModelID();
                    const KoColorProfile *cProfile =
                        KoColorSpaceRegistry::instance()->createColorProfile(
                            colId,
                            Integer8BitsColorDepthID.id(),
                            iccProfile);
                    imageColorSpace = KoColorSpaceRegistry::instance()->colorSpace(
                        colId,
                        Integer8BitsColorDepthID.id(),
                        cProfile);
                    if (imageColorSpace) {
                        isRgba = false;
                    }
                }
            }
        }
        WebPDemuxReleaseChunkIterator(&chunk_iter);
    }

    if (isRgba && imageColorSpace) {
        colorSpace = imageColorSpace;
    }

    const KoColor bgColor(
        QColor(bg >> 8 & 0xFFu, bg >> 16 & 0xFFu, bg >> 24 & 0xFFu, bg & 0xFFu),
        colorSpace);

    KisImageSP image = new KisImage(document->createUndoStore(),
                                    static_cast<qint32>(width),
                                    static_cast<qint32>(height),
                                    colorSpace,
                                    i18n("WebP Image"));

    KisPaintLayerSP layer(
        new KisPaintLayer(image, image->nextLayerName(), 255));

    {
        WebPChunkIterator chunk_iter;
        if (flags & EXIF_FLAG) {
            if (WebPDemuxGetChunk(demux, "EXIF", 1, &chunk_iter)) {
                dbgFile << "Loading EXIF data. Size: " << chunk_iter.chunk.size;

                QBuffer buf;
                buf.setData(
                    reinterpret_cast<const char *>(chunk_iter.chunk.bytes),
                    static_cast<int>(chunk_iter.chunk.size));

                const KisMetaData::IOBackend *backend =
                    KisMetadataBackendRegistry::instance()->value("exif");

                backend->loadFrom(layer->metaData(), &buf);
            }
        }
        WebPDemuxReleaseChunkIterator(&chunk_iter);
    }

    {
        WebPChunkIterator chunk_iter;
        if (flags & XMP_FLAG) {
            if (WebPDemuxGetChunk(demux, "XMP ", 1, &chunk_iter)) {
                dbgFile << "Loading XMP data. Size: " << chunk_iter.chunk.size;

                QBuffer buf;
                buf.setData(
                    reinterpret_cast<const char *>(chunk_iter.chunk.bytes),
                    static_cast<int>(chunk_iter.chunk.size));

                const KisMetaData::IOBackend *xmpBackend =
                    KisMetadataBackendRegistry::instance()->value("xmp");

                xmpBackend->loadFrom(layer->metaData(), &buf);
            }
        }
        WebPDemuxReleaseChunkIterator(&chunk_iter);
    }

    {
        WebPIterator iter;
        if (WebPDemuxGetFrame(demux, 1, &iter)) {
            int nextTimestamp = 0;
            WebPDecoderConfig config;

            KisPaintDeviceSP compositedFrame(
                new KisPaintDevice(image->colorSpace()));

            do {
                if (!WebPInitDecoderConfig(&config)) {
                    dbgFile << "WebP decode config initialization failure";
                    return ImportExportCodes::InternalError;
                }

                {
                    const VP8StatusCode result =
                        WebPGetFeatures(iter.fragment.bytes,
                                        iter.fragment.size,
                                        &config.input);
                    dbgFile << "WebP import validation status: " << result;
                    switch (result) {
                    case VP8_STATUS_OK:
                        break;
                    case VP8_STATUS_OUT_OF_MEMORY:
                        return ImportExportCodes::InsufficientMemory;
                    case VP8_STATUS_INVALID_PARAM:
                        return ImportExportCodes::InternalError;
                    case VP8_STATUS_BITSTREAM_ERROR:
                        return ImportExportCodes::FileFormatIncorrect;
                    case VP8_STATUS_UNSUPPORTED_FEATURE:
                        return ImportExportCodes::FormatFeaturesUnsupported;
                    case VP8_STATUS_SUSPENDED:
                    case VP8_STATUS_USER_ABORT:
                        return ImportExportCodes::InternalError;
                        return ImportExportCodes::InternalError;
                    case VP8_STATUS_NOT_ENOUGH_DATA:
                        return ImportExportCodes::FileFormatIncorrect;
                    }
                }

                // Doesn't make sense to ask for options for each individual
                // frame. See jxl plugin for a similar approach.
                config.output.colorspace = MODE_BGRA;
                config.options.use_threads = 1;

                {
                    const VP8StatusCode result = WebPDecode(iter.fragment.bytes,
                                                            iter.fragment.size,
                                                            &config);

                    dbgFile << "WebP frame:" << iter.frame_num
                            << ", import status: " << result;
                    switch (result) {
                    case VP8_STATUS_OK:
                        break;
                    case VP8_STATUS_OUT_OF_MEMORY:
                        return ImportExportCodes::InsufficientMemory;
                    case VP8_STATUS_INVALID_PARAM:
                        return ImportExportCodes::InternalError;
                    case VP8_STATUS_BITSTREAM_ERROR:
                        return ImportExportCodes::FileFormatIncorrect;
                    case VP8_STATUS_UNSUPPORTED_FEATURE:
                        return ImportExportCodes::FormatFeaturesUnsupported;
                    case VP8_STATUS_SUSPENDED:
                    case VP8_STATUS_USER_ABORT:
                        return ImportExportCodes::InternalError;
                        return ImportExportCodes::InternalError;
                    case VP8_STATUS_NOT_ENOUGH_DATA:
                        return ImportExportCodes::FileFormatIncorrect;
                    }
                }

                // Check for "we're initializing the first frame".
                // This code had previously "config.input.has_animation",
                // this is incorrect when using the demuxer because
                // each frame is yielded through GetFrame().
                if (iter.num_frames > 0 && iter.frame_num == 1) {
                    dbgFile << "Animation detected, estimated framerate:"
                            << static_cast<double>(1000) / iter.duration;
                    const int framerate = std::lround(
                        1000.0 / static_cast<double>(iter.duration));
                    layer->enableAnimation();
                    image->animationInterface()->setDocumentRangeEndFrame(0);
                    image->animationInterface()->setFramerate(framerate);
                }

                const QRect bounds(
                    QPoint{iter.x_offset, iter.y_offset},
                    QSize{config.output.width, config.output.height});

                {
                    KisPaintDeviceSP currentFrame(
                        new KisPaintDevice(image->colorSpace()));
                    currentFrame->fill(bounds, bgColor);

                    currentFrame->writeBytes(config.output.u.RGBA.rgba,
                                             iter.x_offset,
                                             iter.y_offset,
                                             config.output.width,
                                             config.output.height);

                    KisPainter painter(compositedFrame);
                    painter.setCompositeOpId(iter.blend_method == WEBP_MUX_BLEND
                                                 ? COMPOSITE_OVER
                                                 : COMPOSITE_COPY);

                    painter.bitBlt(
                        {iter.x_offset, iter.y_offset},
                        currentFrame,
                        {QPoint(iter.x_offset, iter.y_offset),
                         QSize(config.output.width, config.output.height)});
                }

                if (iter.num_frames > 1) {
                    const int currentFrameTime =
                        std::lround(static_cast<double>(nextTimestamp)
                                    / static_cast<double>(iter.duration));
                    dbgFile << QString(
                                   "Importing frame %1 @ %2, duration %3 ms, "
                                   "blending %4, disposal %5")
                                   .arg(iter.frame_num)
                                   .arg(currentFrameTime)
                                   .arg(iter.duration)
                                   .arg(iter.blend_method)
                                   .arg(iter.dispose_method)
                                   .toStdString()
                                   .c_str();
                    KisKeyframeChannel *channel = layer->getKeyframeChannel(
                        KisKeyframeChannel::Raster.id(),
                        true);
                    auto *frame =
                        dynamic_cast<KisRasterKeyframeChannel *>(channel);
                    image->animationInterface()->setDocumentRangeEndFrame(
                        std::lround(static_cast<double>(nextTimestamp)
                                    / static_cast<double>(iter.duration)));
                    frame->importFrame(currentFrameTime,
                                       compositedFrame,
                                       nullptr);
                    nextTimestamp += iter.duration;
                } else {
                    layer->paintDevice()->makeCloneFrom(compositedFrame,
                                                        image->bounds());
                }

                if (iter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) {
                    compositedFrame->fill(bounds, bgColor);
                }

                WebPFreeDecBuffer(&config.output);
            } while (WebPDemuxNextFrame(&iter));
        }
        WebPDemuxReleaseIterator(&iter);
    }

    WebPDemuxDelete(demux);

    image->addNode(layer.data(), image->rootLayer().data());

    if (!isRgba) {
        image->convertImageColorSpace(imageColorSpace, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    }

    document->setCurrentImage(image);

    return ImportExportCodes::OK;
}

#include "kis_webp_import.moc"
