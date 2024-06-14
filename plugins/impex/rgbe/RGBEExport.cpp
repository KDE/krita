/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "RGBEExport.h"

#include <KisGlobalResourcesInterface.h>

#include <kpluginfactory.h>

#include <QBuffer>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>

#include <KisDocument.h>
#include <KisExportCheckRegistry.h>
#include <KisImportExportErrorCode.h>
#include <KoAlwaysInline.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorTransferFunctions.h>
#include <KoConfig.h>
#include <KoDocumentInfo.h>
#include <kis_assert.h>
#include <kis_debug.h>
#include <kis_iterator_ng.h>
#include <kis_layer.h>
#include <kis_layer_utils.h>
#include <kis_painter.h>
#include <kis_properties_configuration.h>
#include <kis_sequential_iterator.h>

#include "kis_wdg_options_rgbe.h"

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_rgbe_export.json", registerPlugin<RGBEExport>();)

namespace RGBE
{
inline QByteArray floatToRGBE(const int width, const int height, KisPaintDeviceSP &dev)
{
    KisSequentialConstIterator it(dev, {0, 0, width, height});
    QByteArray res;
    res.resize(width * height * 4);

    quint8 rgbe[4] = {0, 0, 0, 0};

    quint8 *ptr = reinterpret_cast<quint8 *>(res.data());
    while (it.nextPixel()) {
        auto *src = reinterpret_cast<const float *>(it.rawDataConst());
        auto *dst = reinterpret_cast<quint8 *>(ptr);
        float vMax = std::max(src[2], std::max(src[0], src[1]));
        int exp;

        if (vMax < 1e-32) {
            rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
        } else {
            vMax = frexp(vMax, &exp) * 256.0f / vMax;
            // Clamp negative values
            rgbe[0] = static_cast<quint8>(std::max(src[0], 0.0f) * vMax);
            rgbe[1] = static_cast<quint8>(std::max(src[1], 0.0f) * vMax);
            rgbe[2] = static_cast<quint8>(std::max(src[2], 0.0f) * vMax);
            rgbe[3] = static_cast<quint8>(exp + 128);
        }

        std::memcpy(dst, rgbe, 4);

        ptr += 4;
    }

    return res;
}

inline void writeBytesRLE(QByteArray &rleBuffer, quint8 *data, int nBytes)
{
    static constexpr int minRunLen = 4;
    int cur = 0;
    int begRun;
    int runCount;
    int oldRunCount;
    int nonRunCount;

    quint8 buf[2];

    while (cur < nBytes) {
        begRun = cur;

        runCount = oldRunCount = 0;
        while ((runCount < minRunLen) && (begRun < nBytes)) {
            begRun += runCount;
            oldRunCount = runCount;
            runCount = 1;
            while ((begRun + runCount < nBytes) && (runCount < 127) && (data[begRun] == data[begRun + runCount])) {
                runCount++;
            }
        }

        if ((oldRunCount > 1) && (oldRunCount == begRun - cur)) {
            buf[0] = 128 + oldRunCount;
            buf[1] = data[cur];
            rleBuffer.append(reinterpret_cast<const char *>(buf), sizeof(buf));
            cur = begRun;
        }

        while (cur < begRun) {
            nonRunCount = begRun - cur;
            if (nonRunCount > 128) {
                nonRunCount = 128;
            }
            buf[0] = nonRunCount;
            rleBuffer.append(buf[0]);
            rleBuffer.append(reinterpret_cast<const char *>(data + cur), sizeof(data[0]) * nonRunCount);
            cur += nonRunCount;
        }

        if (runCount >= minRunLen) {
            buf[0] = 128 + runCount;
            buf[1] = data[begRun];
            rleBuffer.append(reinterpret_cast<const char *>(buf), sizeof(buf));
            cur += runCount;
        }
    }
}
}

RGBEExport::RGBEExport(QObject *parent, const QVariantList &)
    : KisImportExportFilter{parent}
{
}

KisImportExportErrorCode RGBEExport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP cfg)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(io->isWritable(), ImportExportCodes::NoAccessToWrite);

    KisImageSP image = document->savingImage();
    const QRect bounds = image->bounds();

    const KoColorSpace *cs = image->colorSpace();
    const KoColorProfile *targetProfile = KoColorSpaceRegistry::instance()->p709G10Profile();
    const KoColorSpace *targetCs = KoColorSpaceRegistry::instance()->colorSpace(RGBAColorModelID.id(),
                                                                                Float32BitsColorDepthID.id(),
                                                                                targetProfile);

    if (image->root()->childCount() > 1) {
        KisLayerUtils::flattenImage(image, nullptr);
        image->waitForDone();
    }

    const bool isLinearSrgb = [&]() {
        const bool hasPrimaries = cs->profile()->hasColorants();
        const TransferCharacteristics gamma = cs->profile()->getTransferCharacteristics();
        if (hasPrimaries) {
            const ColorPrimaries primaries = cs->profile()->getColorPrimaries();
            if (gamma == TRC_LINEAR && primaries == PRIMARIES_ITU_R_BT_709_5) {
                return true;
            }
        }
        return false;
    }();

    // Color profile will be lost on RGBE export; so convert it to F32, linear sRGB
    if (cs->colorModelId() != RGBAColorModelID || cs->colorDepthId() != Float32BitsColorDepthID || !isLinearSrgb) {
        dbgFile << "Image is not in linear sRGB, converting...";
        image->convertImageColorSpace(targetCs,
                                      KoColorConversionTransformation::internalRenderingIntent(),
                                      KoColorConversionTransformation::internalConversionFlags());
        image->waitForDone();
    }

    // Fill transparent pixels with full opacity
    KoColor bgColor(Qt::white, targetCs);
    bgColor.fromKoColor(cfg->getColor("transparencyFillcolor"));

    KisPaintDeviceSP dev = new KisPaintDevice(targetCs);
    KisPainter gc(dev);

    dev->fill(QRect(0, 0, image->width(), image->height()), bgColor);
    gc.bitBlt(QPoint(0, 0), image->projection(), QRect(0, 0, image->width(), image->height()));
    gc.end();

    // Get pixel data and convert it to RGBE format
    const QByteArray pixels = RGBE::floatToRGBE(bounds.width(), bounds.height(), dev);

    QByteArray fileBuffer;
    {
        // Write header
        QByteArray header;
        header.append("#?RADIANCE\n");
        header.append("# Created with Krita RGBE Export\n");
        header.append("FORMAT=32-bit_rle_rgbe\n\n");
        header.append(QStringLiteral("-Y %1 +X %2\n").arg(image->height()).arg(image->width()).toUtf8());

        fileBuffer.append(header);
    }

    {
        // Write pixel data
        const int scanWidth = image->width();
        const int scanHeight = image->height();

        if ((scanWidth < 8) || (scanWidth > 0x7fff)) {
            // Invalid width, save without RLE
            fileBuffer.append(pixels);
            io->write(fileBuffer);
        } else {
            // Save with RLE
            QByteArray rleBuffer;
            QByteArray outputBuffer;

            int numScanline = scanHeight;
            quint8 rgbe[4];

            rleBuffer.resize(sizeof(quint8) * 4 * scanWidth);
            auto *src = reinterpret_cast<const quint8 *>(pixels.data());
            auto *rle = reinterpret_cast<quint8 *>(rleBuffer.data());

            while (numScanline-- > 0) {
                rgbe[0] = 2;
                rgbe[1] = 2;
                rgbe[2] = scanWidth >> 8;
                rgbe[3] = scanWidth & 0xFF;
                outputBuffer.append(reinterpret_cast<const char *>(rgbe), sizeof(rgbe));

                for (int i = 0; i < scanWidth; i++) {
                    rle[i] = src[0];
                    rle[i + scanWidth] = src[1];
                    rle[i + 2 * scanWidth] = src[2];
                    rle[i + 3 * scanWidth] = src[3];
                    src += 4;
                }

                for (int i = 0; i < 4; i++) {
                    RGBE::writeBytesRLE(outputBuffer, &rle[i * scanWidth], scanWidth);
                    fileBuffer.append(outputBuffer);
                    outputBuffer.clear();
                }
            }

            io->write(fileBuffer);
        }
    }

    return ImportExportCodes::OK;
}

void RGBEExport::initializeCapabilities()
{
    QList<QPair<KoID, KoID>> supportedColorModels;
    addCapability(KisExportCheckRegistry::instance()->get("AnimationCheck")->create(KisExportCheckBase::PARTIALLY));
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::PARTIALLY));
    addCapability(KisExportCheckRegistry::instance()->get("ExifCheck")->create(KisExportCheckBase::PARTIALLY));
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::PARTIALLY));
    addCapability(KisExportCheckRegistry::instance()->get("TiffExifCheck")->create(KisExportCheckBase::PARTIALLY));
    supportedColorModels << QPair<KoID, KoID>() << QPair<KoID, KoID>(RGBAColorModelID, Float32BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "RGBE");
}

KisConfigWidget *
RGBEExport::createConfigurationWidget(QWidget *parent, const QByteArray & /*from*/, const QByteArray & /*to*/) const
{
    return new KisWdgOptionsRGBE(parent);
}

KisPropertiesConfigurationSP RGBEExport::defaultConfiguration(const QByteArray &, const QByteArray &) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();

    KoColor background(KoColorSpaceRegistry::instance()->rgb8());
    background.fromQColor(Qt::white);
    QVariant v;
    v.setValue(background);

    cfg->setProperty("transparencyFillcolor", v);

    return cfg;
}

#include <RGBEExport.moc>
