/*
 *  SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_import.h"

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisDocument.h>
#include <kis_image.h>

#include "psd_loader.h"

K_PLUGIN_FACTORY_WITH_JSON(ImportFactory, "krita_psd_import.json", registerPlugin<psdImport>();)

psdImport::psdImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

psdImport::~psdImport()
{
}

KisImportExportErrorCode psdImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    PSDLoader ib(document);
    KisImportExportErrorCode result = ib.buildImage(io);
    if (result.isOk()) {
        document->setCurrentImage(ib.image());
    }
    return result;
}

#include <psd_import.moc>

