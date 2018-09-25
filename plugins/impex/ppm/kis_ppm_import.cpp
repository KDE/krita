/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_ppm_import.h"

#include <ctype.h>

#include <QApplication>
#include <QFile>

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpaceRegistry.h>

#include <kis_debug.h>
#include <KisDocument.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <KoColorSpaceTraits.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include <KoColorSpace.h>
#include <qendian.h>
#include <KoColorModelStandardIds.h>
#include "kis_iterator_ng.h"

K_PLUGIN_FACTORY_WITH_JSON(PPMImportFactory, "krita_ppm_import.json", registerPlugin<KisPPMImport>();)

KisPPMImport::KisPPMImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisPPMImport::~KisPPMImport()
{
}

int readNumber(QIODevice* device)
{
    char c;
    int val = 0;
    while (true) {
        if (!device->getChar(&c)) break; // End of the file
        if (isdigit(c)) {
            val = 10 * val + c - '0';
        } else if (c == '#') {
            device->readLine();
            break;
        } else if (isspace((uchar) c)) {
            break;
        }
    }
    return val;
}

class KisPpmFlow
{
public:
    KisPpmFlow() {
    }
    virtual ~KisPpmFlow() {
    }
    virtual void nextRow() = 0;
    virtual bool valid() = 0;
    virtual bool nextUint1() = 0;
    virtual quint8 nextUint8() = 0;
    virtual quint16 nextUint16() = 0;

};

class KisAsciiPpmFlow : public KisPpmFlow
{
public:
    KisAsciiPpmFlow(QIODevice* device) : m_device(device) {
    }
    ~KisAsciiPpmFlow() override {
    }
    void nextRow() override {
    }
    bool valid() override {
        return !m_device->atEnd();
    }
    bool nextUint1() override {
        return readNumber(m_device) == 1;
    }
    quint8 nextUint8() override {
        return readNumber(m_device);
    }
    quint16 nextUint16() override {
        return readNumber(m_device);
    }
private:
    QIODevice* m_device;
};

class KisBinaryPpmFlow : public KisPpmFlow
{
public:
    KisBinaryPpmFlow(QIODevice* device, int lineWidth) : m_pos(0), m_device(device), m_lineWidth(lineWidth) {
    }
    ~KisBinaryPpmFlow() override {
    }
    void nextRow() override {
        m_array = m_device->read(m_lineWidth);
        m_ptr = m_array.data();
    }
    bool valid() override {
        return m_array.size() == m_lineWidth;
    }
    bool nextUint1() override {
        if (m_pos == 0) {
            m_current = nextUint8();
            m_pos = 8;
        }
        bool v = (m_current & 1) == 1;
        --m_pos;
        m_current = m_current >> 1;
        return v;
    }
    quint8 nextUint8() override {
        quint8 v = *reinterpret_cast<quint8*>(m_ptr);
        m_ptr += 1;
        return v;
    }
    quint16 nextUint16() override {
        quint16 v = *reinterpret_cast<quint16*>(m_ptr);
        m_ptr += 2;
        return qFromBigEndian(v);
    }
private:
    int m_pos;
    quint8 m_current;
    char* m_ptr;
    QIODevice* m_device;
    QByteArray m_array;
    int m_lineWidth;
};



KisImportExportFilter::ConversionStatus KisPPMImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    QByteArray array = io->read(2);

    if (array.size() < 2) return KisImportExportFilter::CreationError;

    // Read the type of the ppm file
    enum { Puk, P1, P2, P3, P4, P5, P6 } fileType = Puk; // Puk => unknown

    int channels = -1;
    bool isAscii = false;

    if (array == "P1") {
        fileType = P1;
        isAscii = true;
        channels = 0;
    } else if (array == "P2") {
        fileType = P2;
        channels = 1;
        isAscii = true;
    } else if (array == "P3") {
        fileType = P3;
        channels = 3;
        isAscii = true;
    } else if (array == "P4") {
        fileType = P4;
        channels = 0;
    } else if (array == "P5") { // PGM
        fileType = P5;
        channels = 1;
    } else if (array == "P6") { // PPM
        fileType = P6;
        channels = 3;
    }

    Q_ASSERT(channels != -1);
    char c; io->getChar(&c);
    if (!isspace(c)) return KisImportExportFilter::CreationError; // Invalid file, it should have a separator now

    while (io->peek(1) == "#") {
        io->readLine();
    }

    // Read width
    int width = readNumber(io);
    int height = readNumber(io);
    int maxval = 1;

    if (fileType != P1 && fileType != P4) {
        maxval = readNumber(io);
    }

    dbgFile << "Width = " << width << " height = " << height << "maxval = " << maxval;

    // Select the colorspace depending on the maximum value
    int pixelsize = -1;
    const KoColorSpace* colorSpace = 0;

    const KoColorProfile *profile = 0;
    QString colorSpaceId;
    QString bitDepthId;


    if (maxval <= 255) {
        bitDepthId = Integer8BitsColorDepthID.id();

        if (channels == 1 || channels == 0) {
            pixelsize = 1;
            colorSpaceId = GrayAColorModelID.id();
        } else {
            pixelsize = 3;
            colorSpaceId = RGBAColorModelID.id();
        }
    } else if (maxval <= 65535) {
        bitDepthId = Integer16BitsColorDepthID.id();

        if (channels == 1 || channels == 0) {
            pixelsize = 2;
            colorSpaceId = GrayAColorModelID.id();
        } else {
            pixelsize = 6;
            colorSpaceId = RGBAColorModelID.id();
        }
    } else {
        dbgFile << "Unknown colorspace";
        return KisImportExportFilter::CreationError;
    }

    if (colorSpaceId == RGBAColorModelID.id()) {
        profile = KoColorSpaceRegistry::instance()->profileByName("sRGB-elle-V2-srgbtrc.icc");
    } else if (colorSpaceId == GrayAColorModelID.id()) {
        profile = KoColorSpaceRegistry::instance()->profileByName("Gray-D50-elle-V2-srgbtrc.icc");
    }

    colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorSpaceId, bitDepthId, profile);

    KisImageSP image = new KisImage(document->createUndoStore(), width, height, colorSpace, "built image");
    KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), 255);

    QScopedPointer<KisPpmFlow> ppmFlow;
    if (isAscii) {
        ppmFlow.reset(new KisAsciiPpmFlow(io));
    } else {
        ppmFlow.reset(new KisBinaryPpmFlow(io, pixelsize * width));
    }

    for (int v = 0; v < height; ++v) {
        KisHLineIteratorSP it = layer->paintDevice()->createHLineIteratorNG(0, v, width);
        ppmFlow->nextRow();
        if (!ppmFlow->valid()) return KisImportExportFilter::CreationError;
        if (maxval <= 255) {
            if (channels == 3) {
                do {
                    KoBgrTraits<quint8>::setRed(it->rawData(), ppmFlow->nextUint8());
                    KoBgrTraits<quint8>::setGreen(it->rawData(), ppmFlow->nextUint8());
                    KoBgrTraits<quint8>::setBlue(it->rawData(), ppmFlow->nextUint8());
                    colorSpace->setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
                } while (it->nextPixel());
            } else if (channels == 1) {
                do {
                    *reinterpret_cast<quint8*>(it->rawData()) = ppmFlow->nextUint8();
                    colorSpace->setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
                } while (it->nextPixel());
            } else if (channels == 0) {
                do {
                    if (ppmFlow->nextUint1()) {
                        *reinterpret_cast<quint8*>(it->rawData()) = 255;
                    } else {
                        *reinterpret_cast<quint8*>(it->rawData()) = 0;
                    }
                    colorSpace->setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
                } while (it->nextPixel());
            }
        } else {
            if (channels == 3) {
                do {
                    KoBgrU16Traits::setRed(it->rawData(), ppmFlow->nextUint16());
                    KoBgrU16Traits::setGreen(it->rawData(), ppmFlow->nextUint16());
                    KoBgrU16Traits::setBlue(it->rawData(), ppmFlow->nextUint16());
                    colorSpace->setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
                } while (it->nextPixel());
            } else if (channels == 1) {
                do {
                    *reinterpret_cast<quint16*>(it->rawData()) = ppmFlow->nextUint16();
                    colorSpace->setOpacity(it->rawData(), OPACITY_OPAQUE_U8, 1);
                } while (it->nextPixel());
            }
        }
    }

    image->addNode(layer.data(), image->rootLayer().data());

    document->setCurrentImage(image);
    return KisImportExportFilter::OK;
}

#include "kis_ppm_import.moc"
