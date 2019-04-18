/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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

ImportExport::ErrorCode KisGIFExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP configuration)
{
    Q_UNUSED(configuration);
    QRect rc = document->savingImage()->bounds();
    QImage image = document->savingImage()->projection()->convertToQImage(0, 0, 0, rc.width(), rc.height(), KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());

    QGIFLibHandler handler;
    handler.setDevice(io);
    bool result = handler.write(image);
    if (!result) {
       KIS_ASSERT_RECOVER_RETURN_VALUE(true, ImportExport::ErrorCodeID::InternalError);
       return ImportExport::ErrorCodeID::InternalError;
    }
    return ImportExport::ErrorCodeID::OK;
}

void KisGIFExport::initializeCapabilities()
{

    QList<QPair<KoID, KoID> > supportedColorModels;
    supportedColorModels << QPair<KoID, KoID>()
            << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID);
    addSupportedColorModels(supportedColorModels, "GIF");
}



#include "kis_gif_export.moc"

