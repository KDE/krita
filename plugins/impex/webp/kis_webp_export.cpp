/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2021 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QFileInfo>
#include <kpluginfactory.h>
#include <memory>
#include <webp/encode.h>
#include <webp/types.h>

#include <KisDocument.h>
#include <KisExportCheckRegistry.h>
#include <KisImportExportErrorCode.h>
#include <KisImportExportManager.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <kis_assert.h>
#include <kis_debug.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_random_accessor_ng.h>

#include "dlg_webp_export.h"
#include "kis_webp_export.h"

K_PLUGIN_FACTORY_WITH_JSON(KisWebPExportFactory, "krita_webp_export.json", registerPlugin<KisWebPExport>();)

KisWebPExport::KisWebPExport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KisWebPExport::~KisWebPExport()
{
}

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

    return cfg;
}

KisConfigWidget *KisWebPExport::createConfigurationWidget(QWidget *parent, const QByteArray & /*from*/, const QByteArray & /*to*/) const
{
    return new KisDlgWebPExport(parent);
}

KisImportExportErrorCode KisWebPExport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP cfg)
{
    WebPConfig config;
    if (!WebPConfigInit(&config)) {
        dbgPlugins << "WebP config initialization failed!";
        return ImportExportCodes::InternalError;
    }

    {
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
        config.show_compressed = cfg->getBool("show_compressed", false) ? 1 : 0;
        config.preprocessing = cfg->getInt("preprocessing", 0);
        config.partitions = cfg->getInt("partitions", 0);
        config.partition_limit = cfg->getInt("partition_limit", 0);
        config.emulate_jpeg_size = cfg->getBool("emulate_jpeg_size", false) ? 1 : 0;
        config.thread_level = cfg->getBool("thread_level", false) ? 1 : 0;
        config.low_memory = cfg->getBool("low_memory", false) ? 1 : 0;
        config.near_lossless = cfg->getInt("near_lossless", 100);
        config.exact = cfg->getBool("exact", 0) ? 1 : 0;
        config.use_sharp_yuv = cfg->getBool("use_sharp_yuv", false) ? 1 : 0;
#if WEBP_ENCODER_ABI_VERSION >= 0x020f
        config.qmin = cfg->getInt("qmin", 0);
        config.qmax = cfg->getInt("qmax", 100);
#endif

        if (!WebPValidateConfig(&config)) {
            dbgPlugins << "WebP configuration validation failure";
            return ImportExportCodes::InternalError;
        }
    }

    std::unique_ptr<WebPPicture, decltype(&WebPPictureFree)> picture(new WebPPicture(), &WebPPictureFree);
    if (picture && !WebPPictureInit(picture.get())) {
        dbgPlugins << "WebP picture initialization failure";
        return ImportExportCodes::InternalError;
    }

    const QRect rc = document->savingImage()->bounds();
    picture->width = rc.width();
    picture->height = rc.height();

    const bool enableDithering = cfg->getBool("dithering", true);

    KisPaintDeviceSP dst;
    const KoColorSpace *cs = document->savingImage()->projection()->colorSpace();
    if ((cs->colorModelId() == RGBAColorModelID && cs->colorDepthId() == Integer8BitsColorDepthID) || !enableDithering) {
        dst = document->savingImage()->projection();
    } else {
        // We need to use gradient painter code's:
        //    to convert to RGBA samedepth;
        //    then dither to RGBA8
        //    then convert from ARGB32 to RGBA8888
        const KisPaintDeviceSP src = document->savingImage()->projection();
        const KoID depthId = src->colorSpace()->colorDepthId();
        const KoColorSpace *destCs = KoColorSpaceRegistry::instance()->rgb8();
        const KoColorSpace *mixCs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(), depthId.id(), destCs->profile());

        KisPaintDeviceSP tmp(new KisPaintDevice(*src));
        tmp->convertTo(mixCs);
        dst = new KisPaintDevice(destCs);

        const KisDitherOp *op = mixCs->ditherOp(destCs->colorDepthId().id(), enableDithering ? DITHER_BEST : DITHER_NONE);

        KisRandomConstAccessorSP srcIt(tmp->createRandomConstAccessorNG());
        KisRandomAccessorSP dstIt(dst->createRandomAccessorNG());

        int rows = 1;
        int columns = 1;

        for (int y = rc.y(); y <= rc.bottom(); y += rows) {
            rows = qMin(srcIt->numContiguousRows(y), qMin(dstIt->numContiguousRows(y), rc.bottom() - y + 1));

            for (int x = rc.x(); x <= rc.right(); x += columns) {
                columns = qMin(srcIt->numContiguousColumns(x), qMin(dstIt->numContiguousColumns(x), rc.right() - x + 1));

                srcIt->moveTo(x, y);
                dstIt->moveTo(x, y);

                const qint32 srcRowStride {srcIt->rowStride(x, y)};
                const qint32 dstRowStride {dstIt->rowStride(x, y)};
                const quint8 *srcPtr {srcIt->rawDataConst()};
                quint8 *dstPtr {dstIt->rawData()};

                op->dither(srcPtr, srcRowStride, dstPtr, dstRowStride, x, y, columns, rows);
            }
        }
    }

    const QImage image = dst->convertToQImage(nullptr, 0, 0, rc.width(), rc.height()).convertToFormat(QImage::Format_RGBA8888);
    const uchar *rgba = image.constBits();

    if (!WebPPictureImportRGBA(picture.get(), rgba, rc.width() * 4)) {
        dbgPlugins << "WebP picture conversion failure:" << picture->error_code;
        return ImportExportCodes::InternalError;
    }

    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    picture->writer = WebPMemoryWrite;
    picture->custom_ptr = &writer;

    if (!WebPEncode(&config, picture.get())) {
        dbgPlugins << "WebP encoding failure:" << picture->error_code;
        return ImportExportCodes::ErrorWhileWriting;
    }

    QDataStream s(io);
    s.setByteOrder(QDataStream::LittleEndian);
    s.writeRawData(reinterpret_cast<char *>(writer.mem), static_cast<int>(writer.size));
    return ImportExportCodes::OK;
}

void KisWebPExport::initializeCapabilities()
{
    QList<QPair<KoID, KoID>> supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>() << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "WebP");
}

#include "kis_webp_export.moc"
