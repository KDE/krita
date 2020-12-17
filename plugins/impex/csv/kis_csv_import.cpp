/*
 *  SPDX-FileCopyrightText: 2016 Laszlo Fazekas <mneko@freemail.hu>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_csv_import.h"

#include <kpluginfactory.h>

#include <QDebug>
#include <QFileInfo>

#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <kis_image.h>

#include "csv_loader.h"

K_PLUGIN_FACTORY_WITH_JSON(CSVImportFactory, "krita_csv_import.json", registerPlugin<KisCSVImport>();)

KisCSVImport::KisCSVImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisCSVImport::~KisCSVImport()
{
}

KisImportExportErrorCode KisCSVImport::convert(KisDocument *document, QIODevice *io,  KisPropertiesConfigurationSP /*configuration*/)
{
    CSVLoader ib(document, batchMode());
    KisImportExportErrorCode result = ib.buildAnimation(io, filename());
    if (result.isOk()) {
        document->setCurrentImage(ib.image());
    }
    return result;
}

#include <kis_csv_import.moc>

