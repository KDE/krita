/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_gif_export.h"

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

#include "qgiflibhandler.h"

K_PLUGIN_FACTORY_WITH_JSON(KisGIFExportFactory, "krita_gif_export.json", registerPlugin<KisGIFExport>();)

KisGIFExport::KisGIFExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisGIFExport::~KisGIFExport()
{
}

KisImportExportErrorCode KisGIFExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    Q_UNUSED(configuration);
    QRect rc = document->savingImage()->bounds();
    QImage image = document->savingImage()->projection()->convertToQImage(0, 0, 0, rc.width(), rc.height(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());

    QGIFLibHandler handler;
    handler.setDevice(io);
    bool result = handler.write(image);
    if (!result) {
       KIS_ASSERT_RECOVER_RETURN_VALUE(true, ImportExportCodes::InternalError);
       return ImportExportCodes::InternalError;
    }
    return ImportExportCodes::OK;
}

void KisGIFExport::initializeCapabilities()
{

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "GIF");
}



#include "kis_gif_export.moc"

