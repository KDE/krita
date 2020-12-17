/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tga_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QApplication>
#include <KoColorModelStandardIds.h>
#include <KisExportCheckRegistry.h>
#include <KisImportExportManager.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>

#include "tga.h"

K_PLUGIN_FACTORY_WITH_JSON(KisTGAExportFactory, "krita_tga_export.json", registerPlugin<KisTGAExport>();)

KisTGAExport::KisTGAExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisTGAExport::~KisTGAExport()
{
}

KisImportExportErrorCode KisTGAExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    Q_UNUSED(configuration);
    QRect rc = document->savingImage()->bounds();
    QImage image = document->savingImage()->projection()->convertToQImage(0, 0, 0, rc.width(), rc.height(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());

    QDataStream s(io);
    s.setByteOrder(QDataStream::LittleEndian);

    const QImage& img = image;
    const bool hasAlpha = (img.format() == QImage::Format_ARGB32);
    for (int i = 0; i < 12; i++)
        s << targaMagic[i];

    // write header
    s << quint16(img.width());   // width
    s << quint16(img.height());   // height
    s << quint8(hasAlpha ? 32 : 24);   // depth (24 bit RGB + 8 bit alpha)
    s << quint8(hasAlpha ? 0x24 : 0x20);   // top left image (0x20) + 8 bit alpha (0x4)

    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            const QRgb color = img.pixel(x, y);
            s << quint8(qBlue(color));
            s << quint8(qGreen(color));
            s << quint8(qRed(color));
            if (hasAlpha)
                s << quint8(qAlpha(color));
        }
    }

    return ImportExportCodes::OK;
}

void KisTGAExport::initializeCapabilities()
{

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "TGA");
}



#include "kis_tga_export.moc"

