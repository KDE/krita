/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_jpeg_import.h"

#include <QFileInfo>

#include <kpluginfactory.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <KisViewManager.h>
#include <KisImportExportManager.h>

#include "kis_jpeg_converter.h"

K_PLUGIN_FACTORY_WITH_JSON(JPEGImportFactory, "krita_jpeg_import.json", registerPlugin<KisJPEGImport>();)

KisJPEGImport::KisJPEGImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisJPEGImport::~KisJPEGImport()
{
}

KisImportExportErrorCode KisJPEGImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    KisJPEGConverter ib(document, batchMode());
    KisImportExportErrorCode result = ib.buildImage(io);
    if (result.isOk()) {
        document->setCurrentImage(ib.image());
    }
    return result;
}

#include <kis_jpeg_import.moc>

