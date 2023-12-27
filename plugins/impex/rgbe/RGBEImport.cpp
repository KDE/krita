/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Based on KImageFormats Radiance HDR loader
 *
 * SPDX-FileCopyrightText: 2005 Christoph Hormann <chris_hormann@gmx.de>
 * SPDX-FileCopyrightText: 2005 Ignacio Castaño <castanyo@yahoo.es>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <kpluginfactory.h>

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
#include <kis_iterator_ng.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_properties_configuration.h>
#include <kis_sequential_iterator.h>

#include "RGBEImport.h"

K_PLUGIN_FACTORY_WITH_JSON(KisRGBEImportFactory, "krita_rgbe_import.json", registerPlugin<RGBEImport>();)

class Q_DECL_HIDDEN RGBEImportData
{
public:
    KisPaintDeviceSP m_currentFrame{nullptr};
    KoID m_colorID;
    KoID m_depthID;
    float m_gamma = 1.0;
    float m_exposure = 1.0;
    const KoColorSpace *cs = nullptr;
};

namespace RGBEIMPORT
{
#define MAXLINE 1024
#define MINELEN 8 // minimum scanline length for encoding
#define MAXELEN 0x7fff // maximum scanline length for encoding

// read an old style line from the hdr image file
// if 'first' is true the first byte is already read
static bool ReadOldLine(quint8 *image, int width, QDataStream &s)
{
    int rshift = 0;
    int i;

    while (width > 0) {
        s >> image[0];
        s >> image[1];
        s >> image[2];
        s >> image[3];

        if (s.atEnd()) {
            return false;
        }

        if ((image[0] == 1) && (image[1] == 1) && (image[2] == 1)) {
            for (i = image[3] << rshift; i > 0; i--) {
                memcpy(image, image-4, 4);
                image += 4;
                width--;
            }
            rshift += 8;
        } else {
            image += 4;
            width--;
            rshift = 0;
        }
    }
    return true;
}

static void RGBEToPaintDevice(quint8 *image, int width, KisSequentialIterator &it)
{
    for (int j = 0; j < width; j++) {
        it.nextPixel();
        auto *dst = reinterpret_cast<float *>(it.rawData());

        if (image[3]) {
            const float v = std::ldexp(1.0f, int(image[3]) - (128 + 8));
            const float pixelData[4] = {float(image[0]) * v,
                                        float(image[1]) * v,
                                        float(image[2]) * v,
                                        1.0f};
            memcpy(dst, pixelData, 4 * sizeof(float));
        } else {
            // Zero exponent handle
            const float pixelData[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            memcpy(dst, pixelData, 4 * sizeof(float));
        }

        image += 4;
    }
}

// Load the HDR image.
static bool LoadHDR(QDataStream &s, const int width, const int height, KisSequentialIterator &it)
{
    quint8 val;
    quint8 code;

    QByteArray lineArray;
    lineArray.resize(4 * width);
    quint8 *image = (quint8 *)lineArray.data();

    for (int cline = 0; cline < height; cline++) {
        // determine scanline type
        if ((width < MINELEN) || (MAXELEN < width)) {
            ReadOldLine(image, width, s);
            RGBEToPaintDevice(image, width, it);
            continue;
        }

        s >> val;

        if (s.atEnd()) {
            return true;
        }

        if (val != 2) {
            s.device()->ungetChar(val);
            ReadOldLine(image, width, s);
            RGBEToPaintDevice(image, width, it);
            continue;
        }

        s >> image[1];
        s >> image[2];
        s >> image[3];

        if (s.atEnd()) {
            return true;
        }

        if ((image[1] != 2) || (image[2] & 128)) {
            image[0] = 2;
            ReadOldLine(image + 4, width - 1, s);
            RGBEToPaintDevice(image, width, it);
            continue;
        }

        if ((image[2] << 8 | image[3]) != width) {
            dbgFile << "Line of pixels had width" << (image[2] << 8 | image[3]) << "instead of" << width;
            return false;
        }

        // read each component
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < width;) {
                s >> code;
                if (s.atEnd()) {
                    dbgFile << "Truncated HDR file";
                    return false;
                }
                if (code > 128) {
                    // run
                    code &= 127;
                    s >> val;
                    while (code != 0) {
                        image[i + j * 4] = val;
                        j++;
                        code--;
                    }
                } else {
                    // non-run
                    while (code != 0) {
                        s >> image[i + j * 4];
                        j++;
                        code--;
                    }
                }
            }
        }

        RGBEToPaintDevice(image, width, it);
    }

    return true;
}

} // namespace RGBEIMPORT

RGBEImport::RGBEImport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KisImportExportErrorCode
RGBEImport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP /*configuration*/)
{
    if (!io->isReadable()) {
        errFile << "Cannot read image contents";
        return ImportExportCodes::NoAccessToRead;
    }

    if (!(io->peek(11) == "#?RADIANCE\n" || io->peek(7) == "#?RGBE\n")) {
        errFile << "Invalid RGBE header!";
        return ImportExportCodes::ErrorWhileReading;
    }

    RGBEImportData d{};

    int len;
    QByteArray line(MAXLINE + 1, Qt::Uninitialized);
    QByteArray rawFormat;
    QByteArray rawGamma;
    QByteArray rawExposure;
    QByteArray rawHeaderInfo;

    // Parse header
    do {
        len = io->readLine(line.data(), MAXLINE);
        if (line.startsWith("# ")) {
            rawHeaderInfo = line.mid(2, len - 2 - 1 /*\n*/);
        } else if (line.startsWith("GAMMA=")) {
            rawGamma = line.mid(6, len - 6 - 1 /*\n*/);
        } else if (line.startsWith("EXPOSURE=")) {
            rawExposure = line.mid(9, len - 9 - 1 /*\n*/);
        } else if (line.startsWith("FORMAT=")) {
            rawFormat = line.mid(7, len - 7 - 1 /*\n*/);
        }
    } while ((len > 0) && (line[0] != '\n'));

    if (rawFormat != "32-bit_rle_rgbe") {
        errFile << "Invalid RGBE format!";
        return ImportExportCodes::ErrorWhileReading;
    }

    const QString headerInfo = [&]() {
        if (!rawHeaderInfo.isEmpty()) {
            return QString(rawHeaderInfo).trimmed();
        }
        return QString();
    }();

    // Unused fields, I don't know what to do with gamma and exposure fields yet.
    if (!rawGamma.isEmpty()) {
        bool gammaOk = false;
        const float gammaTemp = QString(rawGamma).toFloat(&gammaOk);
        if (gammaOk) {
            d.m_gamma = gammaTemp;
        }
    }
    if (!rawExposure.isEmpty()) {
        bool exposureOk = false;
        const float expTemp = QString(rawExposure).toFloat(&exposureOk);
        if (exposureOk) {
            d.m_exposure = expTemp;
        }
    }

    len = io->readLine(line.data(), MAXLINE);
    line.resize(len);

    /*
       TODO: handle flipping and rotation, as per the spec below
       The single resolution line consists of 4 values, a X and Y label each followed by a numerical
       integer value. The X and Y are immediately preceded by a sign which can be used to indicate
       flipping, the order of the X and Y indicate rotation. The standard coordinate system for
       Radiance images would have the following resolution string -Y N +X N. This indicates that the
       vertical axis runs down the file and the X axis is to the right (imagining the image as a
       rectangular block of data). A -X would indicate a horizontal flip of the image. A +Y would
       indicate a vertical flip. If the X value appears before the Y value then that indicates that
       the image is stored in column order rather than row order, that is, it is rotated by 90 degrees.
       The reader can convince themselves that the 8 combinations cover all the possible image orientations
       and rotations.
    */
    QRegularExpression resolutionRegExp(QStringLiteral("([+\\-][XY]) ([0-9]+) ([+\\-][XY]) ([0-9]+)\n"));
    QRegularExpressionMatch match = resolutionRegExp.match(QString::fromLatin1(line));
    if (!match.hasMatch()) {
        errFile << "Invalid HDR file, the first line after the header didn't have the expected format:" << line;
        return ImportExportCodes::InternalError;
    }

    if ((match.captured(1).at(1) != u'Y') || (match.captured(3).at(1) != u'X')) {
        errFile << "Unsupported image orientation in HDR file.";
        return ImportExportCodes::InternalError;
    }

    const int width = match.captured(4).toInt();
    const int height = match.captured(2).toInt();

    dbgFile << "RGBE image information:";
    dbgFile << "Program info:" << headerInfo;
    if (!rawGamma.isEmpty()) {
        dbgFile << "Gamma:" << d.m_gamma;
    } else {
        dbgFile << "No gamma metadata provided";
    }
    if (!rawExposure.isEmpty()) {
        dbgFile << "Exposure:" << d.m_exposure;
    } else {
        dbgFile << "No exposure metadata provided";
    }
    dbgFile << "Dimension:" << width << "x" << height;

    KisImageSP image;
    KisLayerSP layer;

    const KoColorProfile *profile = nullptr;

    d.m_colorID = RGBAColorModelID;
    d.m_depthID = Float32BitsColorDepthID;

    profile = KoColorSpaceRegistry::instance()->p709G10Profile();
    d.cs = KoColorSpaceRegistry::instance()->colorSpace(d.m_colorID.id(), d.m_depthID.id(), profile);

    image = new KisImage(document->createUndoStore(), width, height, d.cs, "RGBE image");
    layer = new KisPaintLayer(image, image->nextLayerName(), OPACITY_OPAQUE_U8);
    d.m_currentFrame = new KisPaintDevice(image->colorSpace());

    QDataStream stream(io);
    KisSequentialIterator it(d.m_currentFrame, {0, 0, width, height});

    if (!RGBEIMPORT::LoadHDR(stream, width, height, it)) {
        errFile << "Error loading HDR file.";
        return ImportExportCodes::InternalError;
    }

    layer->paintDevice()->makeCloneFrom(d.m_currentFrame, image->bounds());
    image->addNode(layer, image->rootLayer().data());

    document->setCurrentImage(image);

    return ImportExportCodes::OK;
}

#include <RGBEImport.moc>
