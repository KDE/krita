/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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

#include <KGenericFactory>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <KoColorSpaceRegistry.h>
#include <KoFilterChain.h>

#include <kis_debug.h>
#include <kis_doc2.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_iterator.h>
#include <KoColorSpaceTraits.h>
#include <kis_paint_device.h>
#include <KoColorSpace.h>
#include <qendian.h>

typedef KGenericFactory<KisPPMImport> PPMImportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritappmimport, PPMImportFactory("kofficefilters"))

KisPPMImport::KisPPMImport(QObject* parent, const QStringList&) : KoFilter(parent)
{
}

KisPPMImport::~KisPPMImport()
{
}

KoFilter::ConversionStatus KisPPMImport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "Importing using PPMImport!";

    if (to != "application/x-krita")
        return KoFilter::BadMimeType;

    KisDoc2 * doc = dynamic_cast<KisDoc2*>(m_chain -> outputDocument());

    if (!doc)
        return KoFilter::CreationError;

    QString filename = m_chain -> inputFile();

    if (filename.isEmpty()) {
        return KoFilter::FileNotFound;
    }

    KUrl url;
    url.setPath(filename);


    dbgFile << "Import: " << url;
    if (url.isEmpty())
        return KoFilter::FileNotFound;

    if (!KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, qApp -> activeWindow())) {
        dbgFile << "Inexistant file";
        return KoFilter::FileNotFound;
    }

    // We're not set up to handle asynchronous loading at the moment.
    QString tmpFile;
    KoFilter::ConversionStatus result;
    if (KIO::NetAccess::download(url, tmpFile, QApplication::activeWindow())) {
        KUrl uriTF;
        uriTF.setPath(tmpFile);

        // open the file
        QFile *fp = new QFile(uriTF.toLocalFile());
        if (fp->exists()) {
            doc->prepareForImport();
            result = loadFromDevice(fp, doc);
        } else {
            result = KoFilter::CreationError;
        }

        KIO::NetAccess::removeTempFile(tmpFile);
        return result;
    }
    dbgFile << "Download failed";
    return KoFilter::CreationError;
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
    virtual ~KisAsciiPpmFlow() {
    }
    virtual void nextRow() {
    }
    virtual bool valid() {
        return !m_device->atEnd();
    }
    virtual bool nextUint1() {
        return readNumber(m_device) == 1;
    }
    virtual quint8 nextUint8() {
        return readNumber(m_device);
    }
    virtual quint16 nextUint16() {
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
    virtual ~KisBinaryPpmFlow() {
    }
    virtual void nextRow() {
        m_array = m_device->read(m_lineWidth);
        m_ptr = m_array.data();
    }
    virtual bool valid() {
        return m_array.size() == m_lineWidth;
    }
    virtual bool nextUint1() {
        if (m_pos == 0) {
            m_current = nextUint8();
            m_pos = 8;
        }
        bool v = (m_current & 1) == 1;
        --m_pos;
        m_current = m_current >> 1;
        return v;
    }
    virtual quint8 nextUint8() {
        quint8 v = *reinterpret_cast<quint8*>(m_ptr);
        m_ptr += 1;
        return v;
    }
    virtual quint16 nextUint16() {
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

KoFilter::ConversionStatus KisPPMImport::loadFromDevice(QIODevice* device, KisDoc2* doc)
{
    dbgFile << "Start decoding file";
    device->open(QIODevice::ReadOnly);
    if (!device->isOpen()) {
        return KoFilter::CreationError;
    }

    QByteArray array = device->read(2);

    if (array.size() < 2) return KoFilter::CreationError;

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
    char c; device->getChar(&c);
    if (!isspace(c)) return KoFilter::CreationError; // Invalid file, it should have a seperator now

    // Read width
    int width = readNumber(device);
    int height = readNumber(device);
    int maxval = 1;

    if (fileType != P1 && fileType != P4) {
        maxval = readNumber(device);
    }

    dbgFile << "Width = " << width << " height = " << height << "maxval = " << maxval;

    // Select the colorspace depending on the maximum value
    int pixelsize = -1;
    const KoColorSpace* colorSpace = 0;
    if (maxval <= 255) {
        if (channels == 1 || channels == 0) {
            pixelsize = 1;
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0);
        } else {
            pixelsize = 3;
            colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        }
    } else if (maxval <= 65535) {
        if (channels == 1 || channels == 0) {
            pixelsize = 2;
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA16", 0);
        } else {
            pixelsize = 6;
            colorSpace = KoColorSpaceRegistry::instance()->rgb16();
        }
    } else {
        dbgFile << "Unknown colorspace";
        return KoFilter::CreationError;
    }

    KisImageSP image = new KisImage(doc->undoAdapter(), width, height, colorSpace, "built image");
    image->lock();

    KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), 255);

    KisPpmFlow* ppmFlow = 0;
    if (isAscii) {
        ppmFlow = new KisAsciiPpmFlow(device);
    } else {
        ppmFlow = new KisBinaryPpmFlow(device, pixelsize * width);
    }

    for (int v = 0; v < height; ++v) {
        KisHLineIterator it = layer->paintDevice()->createHLineIterator(0, v, width);
        ppmFlow->nextRow();
        if (!ppmFlow->valid()) return KoFilter::CreationError;
        if (maxval <= 255) {
            if (channels == 3) {
                while (!it.isDone()) {
                    KoRgbTraits<quint8>::setRed(it.rawData(), ppmFlow->nextUint8());
                    KoRgbTraits<quint8>::setGreen(it.rawData(), ppmFlow->nextUint8());
                    KoRgbTraits<quint8>::setBlue(it.rawData(), ppmFlow->nextUint8());
                    colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                    ++it;
                }
            } else if (channels == 1) {
                while (!it.isDone()) {
                    *reinterpret_cast<quint8*>(it.rawData()) = ppmFlow->nextUint8();
                    colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                    ++it;
                }
            } else if (channels == 0) {
                while (!it.isDone()) {
                    if (ppmFlow->nextUint1()) {
                        *reinterpret_cast<quint8*>(it.rawData()) = 255;
                    } else {
                        *reinterpret_cast<quint8*>(it.rawData()) = 0;
                    }
                    colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                    ++it;
                }
            }
        } else {
            if (channels == 3) {
                while (!it.isDone()) {
                    KoRgbU16Traits::setRed(it.rawData(), ppmFlow->nextUint16());
                    KoRgbU16Traits::setGreen(it.rawData(), ppmFlow->nextUint16());
                    KoRgbU16Traits::setBlue(it.rawData(), ppmFlow->nextUint16());
                    colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                    ++it;
                }
            } else if (channels == 1) {
                while (!it.isDone()) {
                    *reinterpret_cast<quint16*>(it.rawData()) = ppmFlow->nextUint16();
                    colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                    ++it;
                }
            }
        }
    }

    image->addNode(layer.data(), image->rootLayer().data());

    image->unlock();
    doc->setCurrentImage(image);
    return KoFilter::OK;
}
