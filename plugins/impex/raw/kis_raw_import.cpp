/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_raw_import.h"

#include <kpluginfactory.h>
#include <KoDialog.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KisImportExportErrorCode.h>
#include <KoColorSpaceTraits.h>

#include "kis_debug.h"
#include "KisDocument.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_transaction.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_iterator_ng.h"

#include <kdcraw.h>
#include <libkdcraw_version.h>

using namespace KDcrawIface;

K_PLUGIN_FACTORY_WITH_JSON(KisRawImportFactory, "krita_raw_import.json",
                           registerPlugin<KisRawImport>();)

KisRawImport::KisRawImport(QObject *parent, const QVariantList &)
        : KisImportExportFilter(parent)
{
    m_dialog = new KoDialog();
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

KisImportExportErrorCode KisRawImport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP /*configuration*/)
{
    // Show dialog
    m_dialog->setCursor(Qt::ArrowCursor);
    QApplication::setOverrideCursor(Qt::ArrowCursor);

#if KDCRAW_VERSION < 0x010200
    m_rawWidget.rawSettings->setDefaultSettings();
#else
    m_rawWidget.rawSettings->resetToDefault();
#endif

    slotUpdatePreview();

    int r = QDialog::Accepted;
    if (!batchMode()) {
        r = m_dialog->exec();
    }

    if (r == QDialog::Accepted) {

        QApplication::setOverrideCursor(Qt::WaitCursor);
        // Do the decoding
        // TODO: it would probably be better done in a thread, while an other thread simulate that the application is still living (or even better if libkdcraw was giving us some progress report
        QByteArray imageData;
        RawDecodingSettings settings = rawDecodingSettings();
        settings.sixteenBitsImage =  true;
        int width, height, rgbmax;
        KDcraw dcraw;
        if (!dcraw.decodeRAWImage(filename(), settings, imageData, width, height, rgbmax))
            return ImportExportCodes::FileFormatIncorrect;

        QApplication::restoreOverrideCursor();

        const KoColorProfile *profile = 0;

        switch (settings.outputColorSpace) {
        case RawDecodingSettings::RAWCOLOR:
        case RawDecodingSettings::SRGB:
            profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
            break;
        case RawDecodingSettings::ADOBERGB:
            profile = KoColorSpaceRegistry::instance()->profileByName("ClayRGB-elle-V2-g22.icc");
            break;
        case RawDecodingSettings::WIDEGAMMUT:
            profile = KoColorSpaceRegistry::instance()->profileByName("WideRGB-elle-V2-g22.icc");
            break;
        case RawDecodingSettings::PROPHOTO:
            profile = KoColorSpaceRegistry::instance()->profileByName("LargeRGB-elle-V2-g22.icc");
            break;
        case RawDecodingSettings::CUSTOMOUTPUTCS:
            QFileInfo info(settings.outputProfile);

            if (!info.exists()) {
                qWarning() << "WARNING: couldn't find custom profile" << settings.outputProfile;
                profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
            } else {
                QFile profileFile(settings.outputProfile);

                if (profileFile.open(QFile::ReadOnly)) {
                    profile = KoColorSpaceRegistry::instance()->createColorProfile(RGBAColorModelID.id(), Integer16BitsColorDepthID.id(), profileFile.readAll());
                } else {
                    qWarning() << "WARNING: couldn't open custom profile file" << settings.outputProfile;
                }
            }

            if (!profile) {
                qWarning() << "WARNING: reset profile to sRGB";
                profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
            }

            break;
        }

        // Init the image
        const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb16(profile);
        KisImageSP image = new KisImage(document->createUndoStore(), width, height, cs, filename());
        KIS_ASSERT_RECOVER_RETURN_VALUE(!image.isNull(), ImportExportCodes::InternalError);

        KisPaintLayerSP layer = new KisPaintLayer(image, image->nextLayerName(), quint8_MAX);

        image->addNode(layer, image->rootLayer());
        KIS_ASSERT_RECOVER_RETURN_VALUE(!layer.isNull(), ImportExportCodes::InternalError);

        KisPaintDeviceSP device = layer->paintDevice();
        KIS_ASSERT_RECOVER_RETURN_VALUE(!device.isNull(), ImportExportCodes::InternalError);

        // Copy the data
        KisHLineIteratorSP it = device->createHLineIteratorNG(0, 0, width);
        for (int y = 0; y < height; ++y) {
            do {
                KoBgrU16Traits::Pixel* pixel = reinterpret_cast<KoBgrU16Traits::Pixel*>(it->rawData());
                quint16* ptr = (reinterpret_cast<quint16*>(imageData.data())) + (y * width + it->x()) * 3;
#if KDCRAW_VERSION < 0x000400
                pixel->red = correctIndian(ptr[2]);
                pixel->green = correctIndian(ptr[1]);
                pixel->blue = correctIndian(ptr[0]);
#else
                pixel->red = correctIndian(ptr[0]);
                pixel->green = correctIndian(ptr[1]);
                pixel->blue = correctIndian(ptr[2]);
#endif
                pixel->alpha = 0xFFFF;
            } while (it->nextPixel());
            it->nextRow();
        }

        QApplication::restoreOverrideCursor();
        document->setCurrentImage(image);
        return ImportExportCodes::OK;
    }

    QApplication::restoreOverrideCursor();
    return ImportExportCodes::Cancelled;
}

void KisRawImport::slotUpdatePreview()
{
    QByteArray imageData;
    RawDecodingSettings settings = rawDecodingSettings();
    settings.sixteenBitsImage =  false;
    int width, height, rgbmax;
    KDcraw dcraw;
    if (dcraw.decodeHalfRAWImage(filename(), settings, imageData, width, height, rgbmax)) {
        QImage image(width, height, QImage::Format_RGB32);
        for (int y = 0; y < height; ++y) {
            QRgb *pixel= reinterpret_cast<QRgb *>(image.scanLine(y));
            for (int x = 0; x < width; ++x) {
                quint8* ptr = ((quint8*)imageData.data()) + (y * width + x) * 3;
                pixel[x] = qRgb(ptr[0], ptr[1], ptr[2]);
            }
        }
        m_rawWidget.preview->setPixmap(QPixmap::fromImage(image));
    }
}

RawDecodingSettings KisRawImport::rawDecodingSettings()
{
#if KDCRAW_VERSION < 0x010200
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
#else
    return m_rawWidget.rawSettings->settings();
#endif
}

#include "kis_raw_import.moc"
