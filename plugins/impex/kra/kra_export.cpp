/*
 * Copyright (C) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kra_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kpluginfactory.h>
#include <QFileInfo>
#include <QApplication>

#include <KisImportExportManager.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpace.h>

#include <KisExportCheckRegistry.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_node.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_shape_layer.h>
#include <KoProperties.h>

#include "kra_converter.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_kra_export.json", registerPlugin<KraExport>();)

KraExport::KraExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KraExport::~KraExport()
{
}

KisImportExportErrorCode KraExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisImageSP image = document->savingImage();
    KIS_ASSERT_RECOVER_RETURN_VALUE(image, ImportExportCodes::InternalError);

    KraConverter kraConverter(document);
    KisImportExportErrorCode res = kraConverter.buildFile(io, filename());
    dbgFile << "KraExport::convert result =" << res;
    return res;
}

void KraExport::initializeCapabilities()
{
    // Kra supports everything, by definition
    KisExportCheckFactory *factory = 0;
    Q_FOREACH(const QString &id, KisExportCheckRegistry::instance()->keys()) {
        factory = KisExportCheckRegistry::instance()->get(id);
        addCapability(factory->create(KisExportCheckBase::SUPPORTED));
    }
}

QString KraExport::verify(const QString &fileName) const
{
    QString error = KisImportExportFilter::verify(fileName);
    if (error.isEmpty()) {
        return KisImportExportFilter::verifyZiPBasedFiles(fileName,
                                                          QStringList()
                                                          << "mimetype"
                                                          << "documentinfo.xml"
                                                          << "maindoc.xml"
                                                          << "mergedimage.png"
                                                          << "preview.png");
    }
    return error;
}


#include <kra_export.moc>

