/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "krz_export.h"

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
#include <kis_config.h>
#include "kra_converter.h"

class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(KrzExportFactory, "krita_krz_export.json", registerPlugin<KrzExport>();)

KrzExport::KrzExport(QObject *parent, const QVariantList &)
    : KisImportExportFilter(parent)
{
}

KrzExport::~KrzExport()
{
}

KisImportExportErrorCode KrzExport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisImageSP image = document->savingImage();
    KIS_ASSERT_RECOVER_RETURN_VALUE(image, ImportExportCodes::InternalError);

    KisConfig cfg(true);
    bool compress = cfg.compressKra();
    cfg.setCompressKra(true);
    KraConverter krzConverter(document, updater());
    KisImportExportErrorCode res = krzConverter.buildFile(io, filename(), false);
    cfg.setCompressKra(compress);
    dbgFile << "KrzExport::convert result =" << res;
    return res;
}

void KrzExport::initializeCapabilities()
{
    // Kra supports everything, by definition
    KisExportCheckFactory *factory = 0;
    Q_FOREACH(const QString &id, KisExportCheckRegistry::instance()->keys()) {
        factory = KisExportCheckRegistry::instance()->get(id);
        addCapability(factory->create(KisExportCheckBase::SUPPORTED));
    }
}

QString KrzExport::verify(const QString &fileName) const
{
    QString error = KisImportExportFilter::verify(fileName);
    if (error.isEmpty()) {
        return KisImportExportFilter::verifyZiPBasedFiles(fileName,
                                                          QStringList()
                                                          << "mimetype"
                                                          << "documentinfo.xml"
                                                          << "maindoc.xml"
                                                          << "preview.png");
    }
    return error;
}


#include <krz_export.moc>

