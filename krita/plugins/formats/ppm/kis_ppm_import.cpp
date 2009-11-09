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
    doc -> prepareForImport();

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
        QFile *fp = new QFile(uriTF.path());
        if (fp->exists()) {
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
    KisPpmFlow(QIODevice* device, int lineWidth) : m_device(device), m_lineWidth(lineWidth) {
    }
    void nextRow() {
        m_array = m_device->read(m_lineWidth);
    }
    bool valid() {
        return m_array.size() == m_lineWidth;
    }
    quint8 nextUint8() {
    }
    quint16 nextUint16() {
    }
    QByteArray array() {
        return m_array;
    }
private:
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

    if (array == "P1") {
        fileType = P1;
    } else if (array == "P2") {
        fileType = P2;
    } else if (array == "P3") {
        fileType = P3;
    } else if (array == "P4") {
        fileType = P4;
    } else if (array == "P5") { // PGM
        fileType = P5;
        channels = 1;
    } else if (array == "P6") { // PPM
        fileType = P6;
        channels = 3;
    }

    if (fileType != P6 && fileType != P5) {
        dbgFile << "Only P6 is implemented for now";
        return KoFilter::CreationError;
    }

    char c; device->getChar(&c);
    if (!isspace(c)) return KoFilter::CreationError; // Invalid file, it should have a seperator now

    // Read width
    int width = readNumber(device);
    int height = readNumber(device);
    int maxval = readNumber(device);

    dbgFile << "Width = " << width << " height = " << height << "maxval = " << maxval;

    // Select the colorspace depending on the maximum value
    int pixelsize = -1;
    const KoColorSpace* colorSpace = 0;
    if (maxval <= 255) {
        if (channels == 1) {
            pixelsize = 1;
            colorSpace = KoColorSpaceRegistry::instance()->colorSpace("GRAYA", 0);
        } else {
            pixelsize = 3;
            colorSpace = KoColorSpaceRegistry::instance()->rgb8();
        }
    } else if (maxval <= 65535) {
        if (channels == 1) {
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

    KisPpmFlow* ppmFlow = new KisPpmFlow(device, pixelsize * width);

    for (int v = 0; v < height; ++v) {
        KisHLineIterator it = layer->paintDevice()->createHLineIterator(0, v, width);
        ppmFlow->nextRow();
        QByteArray arr = ppmFlow->array();
        if (!ppmFlow->valid()) return KoFilter::CreationError;
        if (maxval <= 255) {
            if (channels == 3) {
                quint8* ptr = reinterpret_cast<quint8*>(arr.data());
                while (!it.isDone()) {
                    KoRgbTraits<quint8>::setRed(it.rawData(), ptr[0]);
                    KoRgbTraits<quint8>::setGreen(it.rawData(), ptr[1]);
                    KoRgbTraits<quint8>::setBlue(it.rawData(), ptr[2]);
                    colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                    ptr += 3;
                    ++it;
                }
            } else if (channels == 1) {
                quint8* ptr = reinterpret_cast<quint8*>(arr.data());
                while (!it.isDone()) {
                    *reinterpret_cast<quint8*>(it.rawData()) = ptr[0];
                    colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                    ptr += 1;
                    ++it;
                }
            }
        } else {
            if (channels == 3) {
                quint16* ptr = reinterpret_cast<quint16*>(arr.data());
                while (!it.isDone()) {
                    KoRgbU16Traits::setRed(it.rawData(), qToBigEndian(ptr[0]));
                    KoRgbU16Traits::setGreen(it.rawData(), qToBigEndian(ptr[1]));
                    KoRgbU16Traits::setBlue(it.rawData(), qToBigEndian(ptr[2]));
                    colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                    ptr += 3;
                    ++it;
                }
            } else if (channels == 1) {
                quint16* ptr = reinterpret_cast<quint16*>(arr.data());
                while (!it.isDone()) {
                    *reinterpret_cast<quint16*>(it.rawData()) = ptr[0];
                    colorSpace->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                    ptr += 1;
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
