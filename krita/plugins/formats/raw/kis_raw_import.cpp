/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_raw_import.h"

#include <KGenericFactory>

#include <KoFilterChain.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>

#include "kis_debug.h"
#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include <kis_iterators_pixel.h>


#include <libkdcraw/kdcraw.h>
#include <libkdcraw/version.h>

using namespace KDcrawIface;

typedef KGenericFactory<KisRawImport> KisRawImportFactory;
K_EXPORT_COMPONENT_FACTORY(libkrita_raw_import, KisRawImportFactory("kofficefilters"))

KisRawImport::KisRawImport(QObject *parent, const QStringList&)
        : KoFilter(parent)
{
    m_dialog = new KDialog();
    m_dialog->enableButtonApply(false);
    QWidget* widget = new QWidget;
    m_rawWidget.setupUi(widget);
    m_dialog->setMainWidget(widget);

    connect(m_rawWidget.pushButtonUpdate, SIGNAL(clicked()), this, SLOT(slotUpdatePreview()));
}

KisRawImport::~KisRawImport()
{
    delete m_dialog;
}

inline quint16 correctIndian(quint16 v)
{
#if KDCRAW_VERSION < 0x000400
    return ((v & 0x00FF) << 8) | ((v & 0xFF00 >> 8));
#else
    return v;
#endif
}

KoFilter::ConversionStatus KisRawImport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << from << " " << to;
    if (/*from != "image/x-raw" || */to != "application/x-krita") { // too many from to check, and I don't think it can happen an unsupported from
        return KoFilter::NotImplemented;
    }

    dbgFile << "Krita importing from Raw";

    KisDoc2 * doc = dynamic_cast<KisDoc2*>(m_chain -> outputDocument());
    if (!doc) {
        return KoFilter::CreationError;
    }

    doc -> prepareForImport();

    QString filename = m_chain -> inputFile();

    if (filename.isEmpty()) {
        return KoFilter::FileNotFound;
    }


    // Show dialog
    m_dialog->setCursor(Qt::ArrowCursor);
    QApplication::setOverrideCursor(Qt::ArrowCursor);

    m_rawWidget.rawSettings->setDefaultSettings();

    slotUpdatePreview();

    if (m_dialog->exec() == QDialog::Accepted) {

        QApplication::setOverrideCursor(Qt::WaitCursor);
        // Do the decoding
        // TODO: it would probably be better done in a thread, while an other thread simulate that the application is still living (or even better if libkdcraw was giving us some progress report
        QByteArray imageData;
        RawDecodingSettings settings = rawDecodingSettings();
        settings.sixteenBitsImage =  true;
        int width, height, rgbmax;
        KDcraw dcraw;
        if (!dcraw.decodeRAWImage(m_chain->inputFile(), settings, imageData, width, height, rgbmax)) return KoFilter::CreationError;

        QApplication::restoreOverrideCursor();

        // Init the image
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA16", "");
        KisImageWSP image = new KisImage(doc->undoAdapter(), width, height, cs, filename);
        if (image.isNull()) return KoFilter::CreationError;
        image->lock();
        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), quint8_MAX);
        KisTransaction("", layer -> paintDevice());

        image->addNode(layer, image->rootLayer());
        if (layer.isNull()) return KoFilter::CreationError;

        KisPaintDeviceSP device = layer->paintDevice();
        if (device.isNull()) return KoFilter::CreationError;

        // Copy the data
        KisHLineIterator it = device->createHLineIterator(0, 0, width);
        for (int y = 0; y < height; ++y) {
            while (!it.isDone()) {
                KoRgbU16Traits::Pixel* pixel = reinterpret_cast<KoRgbU16Traits::Pixel*>(it.rawData());
                quint16* ptr = ((quint16*)imageData.data()) + (y * width + it.x()) * 3;
#if KDCRAW_VERSION < 0x000400
                pixel->red = correctIndian(ptr[2]);
                pixel->green = correctIndian(ptr[1]);
                pixel->blue = correctIndian(ptr[0]);
#else
                pixel->red = correctIndian(ptr[0]);
                pixel->green = correctIndian(ptr[1]);
                pixel->blue = correctIndian(ptr[2]);
#endif
                cs->setAlpha(it.rawData(), OPACITY_OPAQUE, 1);
                ++it;
            }
            it.nextRow();
        }

        layer->setDirty();
        QApplication::restoreOverrideCursor();
        image->unlock();
        doc->setCurrentImage(image);
        return KoFilter::OK;
    }

    QApplication::restoreOverrideCursor();
    return KoFilter::UserCancelled;
}

void KisRawImport::slotUpdatePreview()
{
    QByteArray imageData;
    RawDecodingSettings settings = rawDecodingSettings();
    int width, height, rgbmax;
    KDcraw dcraw;
    dcraw.decodeHalfRAWImage(m_chain->inputFile(), settings, imageData, width, height, rgbmax);
    QImage image(width, height, QImage::Format_RGB32);
    for (int y = 0; y < height; ++y) {
        QRgb *pixel= reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            quint16* ptr = ((quint16*)imageData.data()) + (y * width + x) * 3;
#if KDCRAW_VERSION < 0x000400
            pixel[x] = qRgb(ptr[0] & 0xFF, ptr[1] & 0xFF, ptr[2]  & 0xFF);
#else
            pixel[x] = qRgb(ptr[0] / 0xFF, ptr[1] / 0xFF, ptr[2] / 0xFF);
#endif
        }
    }
    m_rawWidget.preview->setPixmap(QPixmap::fromImage(image));
}

RawDecodingSettings KisRawImport::rawDecodingSettings()
{
    RawDecodingSettings settings;
    settings.sixteenBitsImage = true;
    settings.brightness = m_rawWidget.rawSettings->brightness();
    settings.RAWQuality = m_rawWidget.rawSettings->quality();
    settings.outputColorSpace = m_rawWidget.rawSettings->outputColorSpace();
    settings.RGBInterpolate4Colors = m_rawWidget.rawSettings->useFourColor();
    settings.DontStretchPixels = m_rawWidget.rawSettings->useDontStretchPixels();
    settings.unclipColors = m_rawWidget.rawSettings->unclipColor();
    settings.whiteBalance = m_rawWidget.rawSettings->whiteBalance();
    settings.customWhiteBalance = m_rawWidget.rawSettings->customWhiteBalance();
    settings.customWhiteBalanceGreen = m_rawWidget.rawSettings->customWhiteBalanceGreen();

    settings.enableBlackPoint = m_rawWidget.rawSettings->useBlackPoint();
    settings.blackPoint = m_rawWidget.rawSettings->blackPoint();

    settings.enableNoiseReduction = m_rawWidget.rawSettings->useNoiseReduction();
    settings.NRThreshold = m_rawWidget.rawSettings->NRThreshold();

    settings.enableCACorrection = m_rawWidget.rawSettings->useCACorrection();
    settings.caMultiplier[0] = m_rawWidget.rawSettings->caRedMultiplier();
    settings.caMultiplier[1] = m_rawWidget.rawSettings->caBlueMultiplier();


    return settings;
}

#include "kis_raw_import.moc"
